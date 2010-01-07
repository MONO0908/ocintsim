/* Last match selector (to be used w/ adaptive routing function).
 * Similar to First Match (sel_1match), but returns the *last* output
 * port produced by the routing function.  The result is that a packet
 * will traverse dimension in reverse order (eg: Y-X)
 */

#include "ocin_router_sel_o1turn.h"

void ocin_router_sel_o1turn::init(ocin_router *parent, const string &_name) {
	
	// call base init() w/ fully specified name
	stringstream *ss = new stringstream();
	*ss << parent->node_info->name.c_str() << "." << _name;
	ocin_router_sel::init(parent, ss->str());
	delete ss;
	
	// What's the order of the topology?
	_num_dimensions = parent_router->node_info->coord.size();
	
	// initialize the _next_dimension vector
	_next_dimension.assign(_num_dimensions, 0);
}


/* select()
 *  Currently, 2 selection functions are implemented
 *  1) Pick the output w/ most free buffs (only eligible VCs are considered)
 *  2) Round-robin among outputs/quadrant
 * At the moment, (1) runs and returns, so (2) doesn't actually run.
 */
int ocin_router_sel_o1turn::select(ocin_msg *msg) {
	// Message is only eligible for adaptive decision at the source,
	// where X- or Y-first decision is made and only if the packet
	// is eligible for adaptivity (has hops in both X & Y dims)
	if ((msg->src_addr == parent_router->node_info->coord) &&
		(msg->routes.size() > 1)) 
	{
		int quadrant = msg->quadrant;
		int dim = _next_dimension[quadrant];

///////////////////////////////////////////////////////////////////////////////////////

		/* Adaptive, based on buffer availability */
		
		// Search through all valid ports and sum up the available credits per port
		map <int, set <int> >::iterator routes_itr;  // Forward iterator 
		map <int, int> pval;
		for (routes_itr = msg->routes.begin(); routes_itr != msg->routes.end(); routes_itr++) {
			int pout = routes_itr->first;
		
			// ptr to downstream credit base @ selected p_out
			vector <ocin_vc_status> &pout_vcs = parent_router->downstream_vc_stats[pout];
			// ptr to the valid set of VCs @ selected p_out
			set <int> vc_set = routes_itr->second; 
			
		    pval[pout] = 0;
			// Iterate over all the VCs @ the selected p_out
			// For each VC, check if it's available & can be used by this msg 
			for (int dest_vc=0; dest_vc < pout_vcs.size(); dest_vc++) { 
				if (pout_vcs[dest_vc].is_avail() &&
					(vc_set.count(dest_vc) > 0))
				{
					pval[pout] += pout_vcs[dest_vc].credits;
				}
			}
		}
		
		// find the port that maximizes free credits
		map <int, int>::iterator pval_it;
		int max_p = -1;
		int max_v = -1;
		for (pval_it = pval.begin(); pval_it != pval.end(); pval_it++) {
			int p = pval_it->first;
			int v = pval_it->second;
			if (v > max_v) {
				max_p = p;
				max_v = v;
			}
		}
		// X- or Y-first routing?
		msg->yFirst = (get_bec_dim(max_p, parent_router->_ports) == X_DIM) ? false : true;
		//cout << "  => Max = " << max_v << " (port " << max_p << "). yFirst: " << msg->yFirst << endl;
		
		return max_p;
	
///////////////////////////////////////////////////////////////////////////////////////
		
		/* Round-robin among eligible ports in a quadrant */
		
		if (dim == X_DIM) {
			_next_dimension[quadrant] = Y_DIM;
			
			map <int, set <int> >::iterator routes_itr;  // Forward iterator 
			routes_itr = msg->routes.begin();
			msg->yFirst = false;
			return (*routes_itr).first;
		}
		else {   // route Y-first
			_next_dimension[quadrant] = X_DIM;
			
			map <int, set <int> >::reverse_iterator routes_ritr;  // Reverse iterator 
			routes_ritr = msg->routes.rbegin();
			msg->yFirst = true;
			return (*routes_ritr).first;
		}
	}
	else {
		map <int, set <int> >::reverse_iterator routes_ritr;  // Reverse iterator 
		routes_ritr = msg->routes.rbegin();
		
		// The key of the first element in the map is the first valid out port
		return (*routes_ritr).first;
	}  
}

