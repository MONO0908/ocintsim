#include "ocin_router_ru_xydor.h"


void ocin_router_ru_xydor::init(ocin_router *parent, const string &_name) {
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
 *  - Perform XY.. dimension-ordered routing by sequentially traversing
 *    all dimensions from lowest to highest.
 *  - Within each dimension, keep traveling until the destination coordinate
 *    in this dimension is equal to the node's intra-dimensional coordinate.
 */
void ocin_router_ru_xydor::route (ocin_flit *flit) {
	ocin_msg *msg = flit->msg;
	int num_dimensions = _coord.size();
	bool done = false;      // fixed init problem - PG 3/28/07
	int rt_dir; 
	int p_out;
	set <int> vc_out_set;
	map <int, set <int> > routes;   // key = p_out, value = set of valid VC's @ this p_out
	
	// 1. figure out the dimension for next hop
	int curr_dim = msg->rt_dimension;
	while (msg->dest_addr[curr_dim] == _coord[curr_dim]) {
		curr_dim++;
		// have we traversed all dimensions?
		if (curr_dim == num_dimensions) {
		    done = true;
		    break;
		}
	}
	
	// within the dimension, are we traveling in increasing or decreasing direction?
	if (done) {
		// dest: local port
		rt_dir = 0;

#ifdef DEBUG
                {
                  stringstream tmp;
                  tmp  << "Dest router reached!";
                  ocin_name_debug(name,tmp.str());
                }
#endif
	}
	else {
		rt_dir = (msg->dest_addr[curr_dim] > _coord[curr_dim]) ? INC : DEC ; 
	}
	
	// 2. map the dimension to an output port
	// Note: the mapping assumes that local port is the LAST port.
	p_out = get_port(curr_dim, rt_dir);
	
	
	// 3. produce a set of valid VCs at the output port
	vc_out_set.clear();
	// TODO: currently, all VCs @ output port are game.
	//       In the future, may need to support multiple classes or 
	//       intra-class routing restrictions
    if (p_out <= port_count  /*OUs + eject port*/) {
    	vector <ocin_vc_status> *vc_info = &(parent_router->downstream_vc_stats[p_out]);
      	for (int i=0; i<vc_info->size(); i++) {
        	vc_out_set.insert(i);
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
    
    routes[p_out] = vc_out_set;

	// 4. Assign values to msg header
	// P_out, dir, dim & valid VC set will be chosen 
	// by the Selection function in VC alloc
//msg->rt_dimension = curr_dim;
//msg->rt_dir = rt_dir;
//msg->vc_set = vc_out_set;
//msg->p_out = p_out;
	msg->routes = routes;

#ifdef DEBUG
        {
          stringstream tmp;
          tmp  << "Curr D: " << msg->rt_dimension
               << ", new D: " << curr_dim
               << ". Curr dir: " << msg->rt_dir
               << ", new dir: " << rt_dir 
               << ". P_out: " << p_out;
          ocin_name_debug(name,tmp.str());
        }

  //cout << msg2str(msg, false /*print flits*/);
#endif
}
