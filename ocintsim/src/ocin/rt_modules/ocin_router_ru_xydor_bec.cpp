#include "ocin_router_ru_xydor_bec.h"


void ocin_router_ru_xydor_bec::init(ocin_router *parent, const string &_name) {

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
	
	// pre-initialize the route look-up tables
	int ports_per_dir = parent->node_info->phys_ports.size();
	// X-dir
	for (int i=0; i<param_num_x_tiles; i++) { // for each dest
	  int offset = i - _coord[0];  // hops to dest
	  int abs_offset = abs(offset);
	  int dir = (offset < 0) ? WEST : EAST;
	  map <int, set <int> > routes;   // key = p_out, value = set of valid VC's
	  
	  // Only keep the longest routes
	  bool found_max = false;
	  int max_span = -1;
	  
	  // iterate over available link lengths
	  // Only keep the longest links.
	  // Key assumption: link lengths are non-decreasing
	  for (int j=ports_per_dir-1; j>=0; j--) {
	  	// Valid minimal output if link length <= absolute offset
	  	int port_span = parent->node_info->phys_ports[j];  
	    if (port_span <= abs_offset) {
	    	if (found_max && (port_span < max_span)) {
	    		// this is not the longest link to dest
	    		continue;
	    	}
	    	if (!found_max) {
	    		max_span = port_span;
	    		found_max = true;
	    	}
	    	
	    	int pport = get_bec_port(j, dir, ports_per_dir);
	    	// generate the set of valid VCs at this output port 
	    	set <int> vc_out_set;
	    	for (int v=0; v<parent->vc_fifos[pport].size(); v++) {
        		vc_out_set.insert(v);
      		}
      		routes[pport] = vc_out_set;
      		
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

	  // Only keep the longest routes
	  bool found_max = false;
	  int max_span = -1;
	  
	  // iterate over available link lengths
	  // Only keep the longest links.
	  // Key assumption: link lengths are non-decreasing
	  for (int j=ports_per_dir-1; j>=0; j--) {
	  	// Valid minimal output if link length <= absolute offset
	  	int port_span = parent->node_info->phys_ports[j];  
	    if (port_span <= abs_offset) {
	    	if (found_max && (port_span < max_span)) {
	    		// this is not the longest link to dest
	    		continue;
	    	}
	    	if (!found_max) {
	    		max_span = port_span;
	    		found_max = true;
	    	}
	    	int pport = get_bec_port(j, dir, ports_per_dir);
	    	// generate the set of valid VCs at this output port 
	    	set <int> vc_out_set;
	    	for (int v=0; v<parent->vc_fifos[pport].size(); v++) {
        		vc_out_set.insert(v);
      		}
      		routes[pport] = vc_out_set;

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
}


/* route()
 *  - Perform XY.. dimension-ordered routing by sequentially traversing
 *    all dimensions from lowest to highest.
 *  - Within each dimension, keep traveling until the destination coordinate
 *    in this dimension is equal to the node's intra-dimensional coordinate.
 */
void ocin_router_ru_xydor_bec::route (ocin_flit *flit) {
	ocin_msg *msg = flit->msg;
	int num_dimensions = _coord.size();
	bool done = false;      // fixed init problem - PG 3/28/07
	int rt_dir; 
	int p_out;
	set <int> vc_out_set;
	map <int, set <int> > routes;   // key = p_out, value = set of valid VC's @ this p_out
	
	/* Figure out the dimension for next hop */
	int curr_dim = msg->rt_dimension;
	while (msg->dest_addr[curr_dim] == _coord[curr_dim]) {
		curr_dim++;
		// have we traversed all dimensions?
		if (curr_dim == num_dimensions) {
		    done = true;
		    break;
		}
	}
	
	// Have we reached the destination router?
	if (done) {

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
	else {
		// read the route map for current dimension
		if (curr_dim == 0) {       // X dim
			int x_coord = msg->dest_addr[curr_dim]; 	
			routes = _x_routes[x_coord];
		}  
		else if (curr_dim == 1) {  // Y dim
			int y_coord = msg->dest_addr[curr_dim]; 	
			routes = _y_routes[y_coord];
		}
		else {   // Invalid dimensio
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
          tmp  << "Curr D: " << msg->rt_dimension
               << ", new D: " << curr_dim;
          ocin_name_debug(name,tmp.str());
        }

  //cout << msg2str(msg, false /*print flits*/);
#endif
}
