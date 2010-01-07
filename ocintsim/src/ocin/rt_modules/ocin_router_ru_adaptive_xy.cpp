/* Simple minimal fully-adaptive routing algorithm.
 * Deadlock-free rt subfunction: XY DOR
 * Min VCs for adaptivity: 2 (1 for DOR, 1+ adaptive)
 * VC0: DOR
 * VC1+: adaptive
 * Routing is unrestricted on VC1+, but must be DOR on VC0.
 * Thus, VC0 can be used at any point to route in the X dir,
 * but can only be used in the dest column to route in Y dir.
 */

#include "ocin_router_ru_adaptive_xy.h"


void ocin_router_ru_adaptive_xy::init(ocin_router *parent, const string &_name) {
	// make a "local" copy of this node's coordinates to avoid an expensive
	// lookup for each packet.
	_coord = parent->node_info->coord;
	
	// call base init() w/ fully specified name
	stringstream *ss = new stringstream();
	*ss << parent->node_info->name.c_str() << "." << _name;
	ocin_router_ru::init(parent, ss->str());
	delete ss;
	
	// initialize the port count
	port_count = parent->node_info->port_count;
	
}


/* route()
 *  - Route adaptively in any n-cube using XY DOR as the 
 *    deadlock-free routing subfunction.
 *  - First, identify all the valid output dimensions
 *    (any dimension for which the current coordinate 
 *     is not equal to the dest coordinate)
 *  - For each dimension, determine the profitable direction
 *  - Finally, for each possible output port (p_out = <dim,dir>)
 *    determine the set of valid VCs.
 */
void ocin_router_ru_adaptive_xy::route (ocin_flit *flit) {
	ocin_msg *msg = flit->msg;
	int num_dimensions = _coord.size();
	int rt_dir; 
	int p_out;
	set <int> vc_out_set;
	//map <int, set <int> > routes;   // key = p_out, value = set of valid VC's @ this p_out
	
	// clear the set of routes
	msg->routes.clear();
	
	// 1. Go through all possible dimensions and determine 
	//    which have yet to be traversed.
	for (int curr_dim=0; curr_dim < num_dimensions; curr_dim++) {
		// not finished in this dim?
		if (msg->dest_addr[curr_dim] != _coord[curr_dim]) {
			// compute the direction in this dim
			rt_dir = (msg->dest_addr[curr_dim] > _coord[curr_dim]) ? INC : DEC ;
			// map <dim, dir> to an output port
			p_out = get_port(curr_dim, rt_dir);
			// Add the output port to the set of valid routes
			msg->routes[p_out].clear(); 
			
		}
	} 
	
	
	// 2. If the routes list is empty, we've reached the destination
	if (msg->routes.size() == 0) {
		rt_dir = 0;
		p_out = get_port(num_dimensions, rt_dir);
		
		// Add the local port as a valid route
		msg->routes[p_out].clear();
		
#ifdef DEBUG
                {
                  stringstream tmp;
                  tmp  << "Dest router reached!";
                  ocin_name_debug(name,tmp.str());
                }
#endif
	}
	
	
	// 3. produce a set of valid VCs at each output port
	map <int, set <int> >::iterator routes_itr;
	int src_vc = flit->vc_idx;
	bool dor;         // used in production of valid output VCs
	                  // that conform to the base routing function
	                  // (ie, VC0 -> VC0 can be acquired only in DOR
	int last_dim = msg->rt_dimension;
	// First valid dimension is always DOR conformant
	int dor_dim = get_dim( (*msg->routes.begin()).first );
	dor = (last_dim <= dor_dim);
	
	// Iterate over all output ports               
	for (routes_itr = msg->routes.begin(); routes_itr != msg->routes.end(); routes_itr++) {
		int p_out = (*routes_itr).first;
		int this_dim = get_dim(p_out);
        
	    if (p_out <= port_count  /*OUs + eject port*/) {
	    	vector <ocin_vc_status> *vc_info = &(parent_router->downstream_vc_stats[p_out]);
	    	// Iterate over all VCs at this output port
	      	for (int dest_vc=0; dest_vc < vc_info->size(); dest_vc++) {
	      		// do not use VC0 unless this dimension is DOR conformant
				if ((dest_vc==0) && (this_dim != dor_dim)) {
					continue;
				}
	        	(msg->routes[p_out]).insert(dest_vc);
	      	}
	    } else {
	      {
	        stringstream tmp;
	        tmp  << "Invalid out port requested!! \n"      
	             << "\t Last valid port index (ejector) = " << port_count
	             << "\t Requested port = " << p_out << endl;
	        ocin_name_fatal(name,tmp.str());
	      }
	      exit(0);
	    }
	}

	/* 4. Assign values to msg header
	 * P_out, dir, dim & valid VC set will be chosen 
	 * by the Selection function in VC alloc
	 */
	//msg->rt_dimension = curr_dim;
	//msg->rt_dir = rt_dir;
	//msg->vc_set = vc_out_set;
	//msg->p_out = (*routes.begin()).first;
	
	//msg->routes = routes;
	

        //#ifdef DEBUG
        {
          int p_out = (*msg->routes.begin()).first;
          stringstream tmp;
          tmp  << "PID: " <<msg->pid 
               << ", Curr D: " << msg->rt_dimension
               << ", new D: " << get_dim(p_out)
               << ". Curr dir: " << msg->rt_dir
               << ", new dir: " << get_dir(p_out)
               << ". P_out: " << p_out
               << ". Routes size: " << msg->routes.size() << endl;
          tmp << "\t Src: " << vector2str(&msg->src_addr)
               <<  ", Dest: " << vector2str(&msg->dest_addr)
               <<  ", This: " << vector2str(&_coord)
               <<  ", src VC: " << src_vc << endl;
          for (routes_itr = msg->routes.begin(); routes_itr != msg->routes.end(); routes_itr++) {
          	int pout = (*routes_itr).first;
          	set <int> *vcs = &msg->routes[pout];
          	tmp << "\t pout " << pout << ": " << set2str(vcs) << endl;
          }
          ocin_name_debug(name,tmp.str());
        }

  //cout << msg2str(msg, false /*print flits*/);
        //#endif
}
