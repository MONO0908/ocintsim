/* Last match selector (to be used w/ adaptive routing function).
 * Similar to First Match (sel_1match), but returns the *last* output
 * port produced by the routing function.  The result is that a packet
 * will traverse dimension in reverse order (eg: Y-X)
 */

#include "ocin_router_sel_bec.h"

void ocin_router_sel_bec::init(ocin_router *parent, const string &_name) {
	
	// call base init() w/ fully specified name
	stringstream *ss = new stringstream();
	*ss << parent->node_info->name.c_str() << "." << _name;
	ocin_router_sel::init(parent, ss->str());
	delete ss;
	
	// What's the order of the topology?
	_num_dimensions = parent_router->node_info->coord.size();
}


/* select()
 *  Currently, 2 selection functions are implemented
 *  1) Pick the output w/ most free buffs (only eligible VCs are considered)
 *  2) Round-robin among outputs/quadrant
 * At the moment, (1) runs and returns, so (2) doesn't actually run.
 */
int ocin_router_sel_bec::select(ocin_msg *msg) {

	map <int, set <int> >::iterator routes_itr; 
	routes_itr = msg->routes.begin();

	if (msg->routes.size() > 1) {
		/* Adaptive, based on buffer availability */
		
		// Search through all valid ports and sum up the available credits per port 
//		map <int, int> pval;  // port num --> free buffs
		int max_p = -1;  // winner port
		int max_c = -1;  // max credits (@ winner port)
		for (; routes_itr != msg->routes.end(); routes_itr++) {
			int pout = routes_itr->first;
		
			// ptr to downstream credit base @ selected p_out
			vector <ocin_vc_status> &pout_vcs = parent_router->downstream_vc_stats[pout];
			// ptr to the valid set of VCs @ selected p_out
			set <int> vc_set = routes_itr->second; 
			
		    int credits = 0; 
			// Iterate over all the VCs @ the selected p_out
			// For each VC, check if it's available & can be used by this msg 
			for (int dest_vc=0; dest_vc < pout_vcs.size(); dest_vc++) { 
				if (pout_vcs[dest_vc].is_avail() &&
					(vc_set.count(dest_vc) > 0))
				{
					credits += pout_vcs[dest_vc].credits;
				}
			}
//cout << "pout: " << pout << ", credits: " << credits << endl;			
			// does this port have the most free credits?
			if (credits > max_c) {
				max_p = pout;
				max_c = credits;
			}
		}
		
//cout << "  => Port: " << max_p << " (credits: " << max_c << ")" << endl;

		return max_p;
	}
	else {
		return routes_itr->first;
	}
}

