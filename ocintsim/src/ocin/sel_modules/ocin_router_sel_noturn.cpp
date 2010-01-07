/* Selector implementing the "no turn" oblivious selection function.
 * As the name implies, it tries to keep the packet in the same
 * dimension, turning only if no VC is available in the dimension
 * being traversed.
 */

#include "ocin_router_sel_noturn.h"

void ocin_router_sel_noturn::init(ocin_router *parent, const string &_name) {
	
	// call base init() w/ fully specified name
	stringstream *ss = new stringstream();
	*ss << parent->node_info->name.c_str() << "." << _name;
	ocin_router_sel::init(parent, ss->str());
	delete ss;
	
	// What's the order of the topology?
	_num_dimensions = parent_router->node_info->coord.size();
}


/* select()
 *  - Oblivious selector implementing the "no turn" policy.
 *  - Tries to keep the packet from turning as long as the 
 *    buffer space in the current dimension is avail.
 */
int ocin_router_sel_noturn::select(ocin_msg *msg) {
	map <int, set <int> >::iterator routes_itr; 
	
	if (msg->routes.size() > 1) {
		int prev_pout = msg->p_out;
		// is the port we routed to on the prev node
		// available at this node?
		routes_itr = msg->routes.find(prev_pout);
		if (routes_itr != msg->routes.end()) {
			if (is_avail(prev_pout, msg))
				return prev_pout;
		}
	}

	// If can't route in the same dim as before for whatever reason
	// (dim is not valid or no free buffers), route in the first
	// available dimension.
	return first_avail(msg);
	
}



/* first_avail()
 *  - Pick first avail output port w/ at least 1 unallocated VC
 */
int ocin_router_sel_noturn::first_avail(ocin_msg *msg) {
	map <int, set <int> >::iterator routes_itr; 
	routes_itr = msg->routes.begin();
	int p_out = (*routes_itr).first;
	
	if (msg->routes.size() > 1) {
		for (; routes_itr != msg->routes.end(); routes_itr++) {
			p_out = (*routes_itr).first;
			
			// If an avail VC has been found, terminate the search
			if (is_avail(p_out, msg))
				break;
		
		} // for each valid output port
	} // if (routes > 1) 
	
	return p_out;  
}


/* is_avail()
 *  - checks whether a given output port is available.
 *  - A port is available if it has an unallocated VC
 *    that a given msg can route to.
 */
bool ocin_router_sel_noturn::is_avail(int p_out, ocin_msg *msg) {
	// ptr to downstream credit base @ selected p_out
	vector <ocin_vc_status> &pout_vcs = parent_router->downstream_vc_stats[p_out];
	// ptr to the valid set of VCs @ selected p_out
	set <int> &vc_set = msg->routes[p_out];
	
	bool match = false;
	
	// Iterate over all the VCs @ the selected p_out
	// For each VC, check if it's available & can be used by this msg 
	for (int dest_vc=0; dest_vc < pout_vcs.size(); dest_vc++) { 
		if (pout_vcs[dest_vc].is_avail() &&
			(vc_set.count(dest_vc) > 0))
		{
				// Terminate the search once a valid & avail VC is found
  				match = true;
  				break;
		}
	}
	
	return match;	
}
