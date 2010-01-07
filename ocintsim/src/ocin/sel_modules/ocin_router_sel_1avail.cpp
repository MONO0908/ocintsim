/* Selector that picks the first available valid output port.
 * A port is available if it has at least 1 unallocated VC.
 * Effectively, this implements a selection function that 
 * favors DOR routing.
 */

#include "ocin_router_sel_1avail.h"

void ocin_router_sel_1avail::init(ocin_router *parent, const string &_name) {
	
	// call base init() w/ fully specified name
	stringstream *ss = new stringstream();
	*ss << parent->node_info->name.c_str() << "." << _name;
	ocin_router_sel::init(parent, ss->str());
	delete ss;
	
	// What's the order of the topology?
	_num_dimensions = parent_router->node_info->coord.size();
}


/* select()
 *  - Pick first avail output port w/ at least 1 unallocated VC
 */
int ocin_router_sel_1avail::select(ocin_msg *msg) {
	map <int, set <int> >::iterator routes_itr; 
	routes_itr = msg->routes.begin();
	int p_out = (*routes_itr).first;
	
	if (msg->routes.size() > 1) {
		for (; routes_itr != msg->routes.end(); routes_itr++) {
			p_out = (*routes_itr).first;
			
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
			
			// If an avail VC has been found, terminate the search
			if (match)
				break;
		
		} // for each valid output port
	} // if (routes > 1) 
	
	return p_out;  
}

