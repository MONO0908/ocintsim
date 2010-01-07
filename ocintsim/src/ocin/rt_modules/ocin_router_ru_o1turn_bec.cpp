#include "ocin_router_ru_o1turn_bec.h"

/* O1Turn routing for BEC-enabled networks
 * Compatible w/ mesh, cmesh, flattened bfly and bec's.
 * 
 * At initialization time, the set of valid output ports is produced, 
 * partitioned by X-first and Y-first routing.
 * The VCs at each physical output are split in half, so that 1/2
 * goes to the x-first and the other half to y-first partitions.
 * Thus, four route sets are prepared: 
 *   X-first X & Y routes and Y-first X & Y routes.
 * 
 * Routing is done in similarly to conventional DOR, except that
 * the dimension is a function of whether the msg is routing X- or Y-first.
 * At injection time (msg->src = coord), both route sets are generated
 * (X-first X and Y-first Y) for eligible message, and the Selection
 * function picks one or the other and tags the msg as being routed
 * xFirst or yFirst.
 */

void ocin_router_ru_o1turn_bec::init(ocin_router *parent, const string &_name) {

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
	
	/* pre-initialize the route look-up tables.
	 * This is very inefficient, since I first populate all possible routes from
	 * a given source to all destinations (same as in XYDOR_BEC), then prune
	 * them to (a) longest hop only and (b) segregate into VC sets for O1Turn */
	
	int ports_per_dir = parent->node_info->phys_ports.size();
	// X-dir
	for (int i=0; i<param_num_x_tiles; i++) { // for each dest
	  int offset = i - _coord[0];  // hops to dest
	  int abs_offset = abs(offset);
	  int dir = (offset < 0) ? WEST : EAST;
	  map <int, set <int> > routes;   // key = p_out, value = set of valid VC's
	  // iterate over available link lengths
	  for (int j=0; j<ports_per_dir; j++) {
	  	// Valid minimal output if link length <= absolute offset 
	    if (parent->node_info->phys_ports[j] <= abs_offset) {
	    	int pport = get_bec_port(j, dir, ports_per_dir);
	    	// generate the set of valid VCs at this output port 
	    	set <int> vc_out_set;
	    	for (int v=0; v<parent->vc_fifos[pport].size(); v++) {
        		vc_out_set.insert(v);
      		}
      		routes[pport] = vc_out_set;
      		
// DELETE
//cout << fcns(name) << "Adding pport " << pport << " as valid route to dest " << i << " (X dir)" << endl;
#ifdef DEBUG
    {
      stringstream tmp;
      tmp << "Adding pport " << pport << " as valid route to dest " << i << " (X dir)"<< endl;
      ocin_name_debug(name,tmp.str());
    }
#endif
	    }
	  }
	  
	  // Insert the route map for this destination
	  _x_routes.push_back(routes);

// DELETE	  
//cout << fcns(name) << "valid X ports to dest " << i << endl << map_set2str(&routes);
	}
	
    // Y-dir
	for (int i=0; i<param_num_x_tiles; i++) { // for each dest
	  int offset = i - _coord[1];  // hops to dest
	  int abs_offset = abs(offset);
	  int dir = (offset < 0) ? NORTH : SOUTH;
	  map <int, set <int> > routes;   // key = p_out, value = set of valid VC's
	  // iterate over available link lengths
	  for (int j=0; j<ports_per_dir; j++) {
	  	// Valid minimal output if link length <= absolute offset 
	    if (parent->node_info->phys_ports[j] <= abs_offset) {
	    	int pport = get_bec_port(j, dir, ports_per_dir);
	    	// generate the set of valid VCs at this output port 
	    	set <int> vc_out_set;
	    	for (int v=0; v<parent->vc_fifos[pport].size(); v++) {
        		vc_out_set.insert(v);
      		}
      		routes[pport] = vc_out_set;

// DELETE      		
//cout << fcns(name) << "Adding pport " << pport << " as valid route to dest " << i << " (Y dir)" << endl;
#ifdef DEBUG
    {
      stringstream tmp;
      tmp << "Adding pport " << pport << " as valid route to dest " << i  << " (Y dir)"<< endl;
      ocin_name_debug(name,tmp.str());
    }
#endif
	    }
	  }
	  
	  // Insert the route map for this destination
	  _y_routes.push_back(routes);

// DELETE	  
//cout << fcns(name) << "valid Y ports to dest " << i << endl << map_set2str(&routes);
	}
	
	/* Populate the O1TURN routing maps.
	 * Keep the longest hop in each dimension & segrate VCs basec on 
	 * X-first or Y-first */
	map <int, set <int> >::reverse_iterator routes_ritr;  // Reverse iterator 
	set <int>::iterator set_itr;
	
	// X dir
	for (int i=0; i<_x_routes.size(); i++) {
		// Segregate VCs into 2 sets based on X-first or Y-first
		map <int, set<int> > xFirst_r, yFirst_r;
		
		map <int, set <int> > routes = _x_routes[i];
		if (routes.size() != 0) {
			routes_ritr = routes.rbegin();
			int pout = (*routes_ritr).first;
			set <int> vcs = (*routes_ritr).second;
			
			// Split the VCs into 2 equal sets
			int vcs_per_set = vcs.size() / 2;
			if (vcs_per_set < 1) {
				stringstream t;
	            t << "O1Turn requires at least 1 VC per set (at least 2 VCs/port)!" ;
	            ocin_name_fatal(name, t.str() );
	            exit(0);
			}
			
			// iterate over the VCs, placing the first half into xFirst
			// and the second half into Yfirst routes.
			int vc_cnt = 0;
			for (set_itr=vcs.begin(); set_itr!=vcs.end(); set_itr++, vc_cnt++) {
				if (vc_cnt < vcs_per_set) {
					xFirst_r[pout].insert(*set_itr);
				}
				else {
					yFirst_r[pout].insert(*set_itr);
				}
			}
		}
		_xFirst_x_routes.push_back(xFirst_r);
		_yFirst_x_routes.push_back(yFirst_r);
//cout << name << ". X dir (xFirst): " << map_set2str(&xFirst_r) << endl;
//cout << name << ". X dir (yFirst): " << map_set2str(&yFirst_r) << endl;
	}
	
	// Y dir
	for (int i=0; i<_y_routes.size(); i++) {
		// Segregate VCs into 2 sets based on X-first or Y-first
		map <int, set<int> > xFirst_r, yFirst_r;
		
		map <int, set <int> > routes = _y_routes[i];
		if (routes.size() != 0) {		
			routes_ritr = routes.rbegin();
			int pout = (*routes_ritr).first;
			set <int> vcs = (*routes_ritr).second;
			
			// Split the VCs into 2 equal sets
			int vcs_per_set = vcs.size() / 2;
			if (vcs_per_set < 1) {
				stringstream t;
	            t << "O1Turn requires at least 1 VC per set (at least 2 VCs/port)!" ;
	            ocin_name_fatal(name, t.str() );
	            exit(0);
			}
			
			// iterate over the VCs, placing the first half into xFirst
			// and the second half into Yfirst routes.
			int vc_cnt = 0;
			for (set_itr=vcs.begin(); set_itr!=vcs.end(); set_itr++, vc_cnt++) {
				if (vc_cnt < vcs_per_set) {
					xFirst_r[pout].insert(*set_itr);
				}
				else {
					yFirst_r[pout].insert(*set_itr);
				}
			}
		}
		_xFirst_y_routes.push_back(xFirst_r);
		_yFirst_y_routes.push_back(yFirst_r);
	}	
	
}


/* route()
 *  - Perform XY.. dimension-ordered routing by sequentially traversing
 *    all dimensions from lowest to highest.
 *  - Within each dimension, keep traveling until the destination coordinate
 *    in this dimension is equal to the node's intra-dimensional coordinate.
 */
void ocin_router_ru_o1turn_bec::route (ocin_flit *flit) {
	ocin_msg *msg = flit->msg;
	int num_dimensions = _coord.size();
	bool done = false;      // fixed init problem - PG 3/28/07
	int rt_dir; 
	int p_out;
	set <int> vc_out_set;
	map <int, set <int> > routes;   // key = p_out, value = set of valid VC's @ this p_out
	
	// Have we reached the destination router?
	if (msg->dest_addr == _coord) {

#ifdef DEBUG
                {
                  stringstream tmp;
                  tmp  << "Dest router reached!";
                  ocin_name_debug(name,tmp.str());
                }
#endif
		/* Generate set of valid ejection VCs */
		int terminal_port = msg->t_out;  // terminal port #
		// ejection port  = (index of first local port) + (local port #)
		int ej_port = port_count + terminal_port; 
		vector <ocin_vc_status> *vc_info = &(parent_router->downstream_vc_stats[ej_port]);
      	for (int i=0; i<vc_info->size(); i++) {
        	vc_out_set.insert(i);
      	}
    	routes[ej_port] = vc_out_set; 
	}
	// Is this the source router?
	else if (msg->src_addr == _coord) {
		// find the starting dimension for the pkt and generate all 
		// valid routes (X-first and Y-first)
		if (msg->dest_addr[X_DIM] != _coord[X_DIM]) {
			if (msg->dest_addr[Y_DIM] != _coord[Y_DIM]) {
				// Both X-first and Y-first are valid
				int x_coord = msg->dest_addr[X_DIM];
				routes = _xFirst_x_routes[x_coord];
				int y_coord = msg->dest_addr[Y_DIM];
				routes.insert(_yFirst_y_routes[y_coord].begin(), _yFirst_y_routes[y_coord].end()); 
//cout << name << "->" << msg->dst << ": " << map_set2str(&routes) << endl;
			}
			else {
				// Only X available for routing
				int x_coord = msg->dest_addr[X_DIM];
				routes = _xFirst_x_routes[x_coord];
			}
		}
		else {
			// Only Y remaining for routing
			int y_coord = msg->dest_addr[Y_DIM];
			routes = _xFirst_y_routes[y_coord];
		}
	}
	// Intermediate network node
	else {		
		/* Figure out the dimension for next hop */
		int curr_dim = msg->rt_dimension;		
		
		while (msg->dest_addr[curr_dim] == _coord[curr_dim]) {
			msg->yFirst ? --curr_dim : ++curr_dim;
		}
		
		// Read the route map for current dimension.
		// path depends on whether the msg is X-first or Y-first routed.
		if (curr_dim == 0) {       // X dim
			int x_coord = msg->dest_addr[curr_dim];
			routes = msg->yFirst ? _yFirst_x_routes[x_coord] :
			                       _xFirst_x_routes[x_coord];
		}  
		else if (curr_dim == 1) {  // Y dim
			int y_coord = msg->dest_addr[curr_dim]; 	
			routes = msg->yFirst ? _yFirst_y_routes[y_coord] :
			                       _xFirst_y_routes[y_coord];
		}
		else {   // Invalid dimension
			stringstream tmp;
        	tmp  << "Invalid dimension: " << curr_dim;
        	ocin_name_fatal(name,tmp.str());
        	exit(0);
		}
		 
	}

// DELETE
//cout << fcns(name) << "valid routes to " << msg->dst << endl << map_set2str(&routes);
	msg->routes = routes;

#ifdef DEBUG
        {
          stringstream tmp;
          tmp  << "Valid routes for msg #" << msg->pid
               << ": " << map_set2str(&msg->routes); 
          ocin_name_debug(name,tmp.str());
        }

  //cout << msg2str(msg, false /*print flits*/);
#endif
}
