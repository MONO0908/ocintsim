/***********************************
 * Various link cost functions used by any Selector class.
 ***********************************/

#include "ocin_router_cost.h"

void ocin_router_cost::init(ocin_router *parent, const string &_name) {
	parent_router = parent;
	
	stringstream ss;
	ss << parent->node_info->name.c_str() << "." << _name;
	name = ss.str();
	
	// determine the cost function
	string cost_str = parent_router->node_info->rt_cost_fn;
	// relies on local info
	if (cost_str == "local") {
		cost_fn = LOCAL;
	}
        // removed, not used
	// // quadrant-based
	// // TODO: DELETE (not used!!!)
	// else if (cost_str == "quadrant") {
	// 	cost_fn = QUADRANT;
	// }
	// // idealized 0-delay cost estimation functions
	// else if (cost_str == "buff_nohist") {
	// 	cost_fn = BUFF_NOHIST;
	// }
	// else if (cost_str == "buff_hist") {
	// 	cost_fn = BUFF_HIST;
	// }
	// else if (cost_str == "free_vc_nohist") {
	// 	cost_fn = FREE_VC_NOHIST;
	// }
	// else if (cost_str == "omni_1D") {
	// 	cost_fn = OMNI_1D;
	// }
	// // idealized 0-delay omniscient search function
	// else if (cost_str == "omni_all") {
	// 	cost_fn = OMNI_ALL;
	// }
	// // invalid cost function specified
	else {
	    stringstream tmp;
	    tmp  << "Invalid Cost function: " << cost_str << "\n";     
	    ocin_name_fatal(name,tmp.str());
    	exit(0); 
	}

#ifdef DEBUG	
        {
          stringstream tmp;
          tmp   << "Initialized.  Cost function = " << cost_str;
          ocin_name_debug(name,tmp.str());
        }
#endif

}

/* cost()
 *  - estimate cost based on the selected cost function
 */
int ocin_router_cost::cost(int p_out, ocin_msg *msg) {
  switch (cost_fn) {
  case LOCAL:
    {
      if (parent_router->cost_regs.size() > 1) {
        int q = msg->quadrant;
        return parent_router->cost_regs[q][p_out].get_cost();
      }
      else {
        return parent_router->cost_regs[0][p_out].get_cost();
      }
    }
    // case BUFF_NOHIST:
    //   return cost_buff_nohist(p_out, msg);
    // case BUFF_HIST:
    //   return cost_buff_hist(p_out, msg, parent_router);
    // case FREE_VC_NOHIST:
    //   return cost_free_vc_nohist(p_out, msg);
    // case OMNI_1D:
    //  {
    //     int hops = 0;  // hop count is used to weigh links
    //     return cost_omni_1D(p_out, msg, parent_router, hops);
    //  }
    // case OMNI_ALL:
    //  {
    //  	// record available routes, as they will be modified by cost_buff_omni
    //  	map <int, set <int> > routes = msg->routes;
    //  	// only search the specified p_out
    //  	msg->routes.clear();
    //  	msg->routes[p_out] = routes[p_out];
    //  	int max_cost = 0;
    
    //  	int cost = cost_omni_all(p_out, msg, parent_router, max_cost);
    
    //  	// restore the routes
    //  	msg->routes = routes;
    
    //  	// return the cost
    //  	return cost;
    //  }
  default:
    {
      stringstream tmp;
      tmp  << "Invalid or removed enum value for the Cost function:" << cost_fn << "\n";     
      ocin_name_fatal(name,tmp.str());
    }
    exit(0); 
  } // switch
}


// ******** Removed free_vc_nohist and buff_nohist, the are only
// ******** partially functional and have been replaced with other
// ******** code...

// /* cost_free_vc_nohist()
//  *  - cost estimate based on *current* VC availability
//  *  - pick an output port *only* if there's at least 1 VC 
//  *    that this packet is allowed to route to.
//  */
// int ocin_router_cost::cost_free_vc_nohist(int p_out, ocin_msg *msg) {
// 	vector <ocin_vc_status> &pout_vcs = _downstream_vc_stats[p_out];
	
// 	int free_vcs = 0;
//     bool match = false;  // make sure there's at least 1 free VC
//                          // in the set of all free VCs 
//                          // that can be used to route this pkt
// 	set <int> &vc_set = msg->routes[p_out];
// 	for (int dest_vc=0; dest_vc < pout_vcs.size(); dest_vc++) { 
// 		//if (is_free_vc(pout_vcs[dest_vc].free_vc_list, dest_vc)) {
// 		if (pout_vcs[dest_vc].is_avail()) {
// 			if (vc_set.count(dest_vc) > 0)
// 		  		match = true;
// 		  	free_vcs++;	
// 		}
// 	}
	
// 	// If no VC from the set of valid VCs is free,
// 	// return 0 regardless of the total # of free VCs.
// 	// This is necessary to avoid deadlock.
//     int final_cost = (match) ? free_vcs : 0;
// 	return final_cost;

// }


// /* cost_buff_nohist()
//  *  - cost estimate based on *current* buffer utilization
//  */
// int ocin_router_cost::cost_buff_nohist(int p_out, ocin_msg *msg) {
// 	vector <ocin_vc_status> &pout_vcs = _downstream_vc_stats[p_out];
//     int credits = 0;
	
// 	// iterate over all VCs @ p_out
// 	// sum up buffer utilization & capacity. 
// 	for (int i=0; i < pout_vcs.size(); i++) {
// 		credits += pout_vcs[i].credits;
// 	}
// 	return credits;
// }


// **** Removed cost_buff_hist, omni_1D and omni_all, they are broken and not worth fixing.
// /* cost_buff_hist()
//  *  - cost estimate based on historical buffer utilization
//  */
// int ocin_router_cost::cost_buff_hist(int p_out, ocin_msg *msg, ocin_router *parent_rtr) {
// 	int buffers_free = 0;
// 	vector <ocin_router_vc_fifo> &vc_fifos = 
// 			parent_rtr->node_info->next_hop_input_ports[p_out]->_input_fifos;
	
// 	//int pin_dir = 1 - p_out % 2;  // reverse the dir of p_out 
// 	//int pin = ((p_out / 2) * 2) + pin_dir;
		
// 	// iterate over all VCs @ p_out
// 	// sum up buffer utilization & capacity. 
// 	for (int i=0; i < vc_fifos.size(); i++) {
// 		buffers_free += vc_fifos[i].mon->get_chpt_buffers_free();	
// //		buffers_free += (vc_fifos[i].is_avail() ? 1 : 0);
// 	}

// 	return buffers_free;
// }

/* cost_omni_1D()
 *  - computes link cost by considering the cost
 *    along the entire direction of travel.
 *  - weighs links by their distance from the current node (0.5, 0.25 ..)
 *  - does *not* take into account the max number of hops that a packet
 *    may travel in a given direction.
 */
// int ocin_router_cost::cost_omni_1D(int p_out, ocin_msg *msg, ocin_router *parent_rtr, int &hops) {
// 	ocin_router_iu *next_hop_iu = 
// 		parent_rtr->node_info->next_hop_input_ports[p_out];

// 	// base case for recursion
// 	if (next_hop_iu == NULL)
// 		return 0;
	
	
// 	int my_hop = ++hops;  // increment the hop count
// 	// compute the "bare" cost of this link 
// 	//int this_cost = cost_buff_hist(p_out, msg, parent_rtr);
// 	int this_cost = parent_rtr->cost_regs[0][p_out].get_cost();
// 	// compute the cost of all subsequent links in this direction
// 	int rem_cost = cost_omni_1D(p_out, msg, next_hop_iu->parent_router, hops);
	
// 	// do not scale link cost for the last link in the dimension
// 	if (my_hop == hops) {
// 		return this_cost;
// 	}
// 	else {
// 		// link cost is the average of the cost of this link
// 		// and all subsequent links.
// 		int avg_cost = ((this_cost + rem_cost) / 2);
// 		return avg_cost;
// 	}
// }


// /* cost_omni_all()
//  *  - evaluates all possible paths from a given node to dest
//  *    via a specified output port, and returns the highest
//  *    aggregate cost.
//  */
// int ocin_router_cost::cost_omni_all(int p_out, ocin_msg *msg, ocin_router *parent_rtr, int &max_cost) {
// 	// Perfom routing on the message
// 	parent_rtr->routing_units[0].route(msg->flits.front());
	
// 	// Have we reached the destination?
// 	int local_port = parent_rtr->node_info->port_count; // local port index
// 	if (msg->routes.find(local_port) != msg->routes.end()) {

// // DELETE
// //cout << fcns(parent_rtr->node_info->name) << "Dest reached" << endl;

// 		// Destination reached
// 		return -1;
// 	}
// 	else {
// 		int new_max_cost = 0;
		
// 		// iterate over all valid output ports
// 		map <int, set <int> > routes = msg->routes; // make a local copy, as routes
// 		                                            // are modified @ each hop
// 		map <int, set <int> >::iterator routes_itr;
// 		for (routes_itr = routes.begin(); routes_itr != routes.end(); routes_itr++) {
// 			int this_pout = (*routes_itr).first;

// 			// grab a pointer to next hop's router thru this out port
// 			ocin_router *next_rtr = parent_rtr->node_info->next_hop_input_ports[this_pout]->parent_router;

// 			// compute the cost of using this output port
// 			int this_cost = parent_rtr->cost_regs[0][p_out].get_cost();

// // DELETE
// //cout << fcns(parent_rtr->node_info->name) << "Evaluating p_out " << this_pout 
// //                                          << ". Cost = " << this_cost << endl;

// 			// compute the cost of all subsequent links in this direction
// 			int rem_cost = cost_omni_all(this_pout, msg, next_rtr, max_cost);
			
// 			// aggregate values (unless destination reached)
// 			int this_pout_cost;
// 			if (rem_cost >= 0) {
// 				//this_pout_cost = (this_cost + rem_cost) / 2;
// 				this_pout_cost = (this_cost + rem_cost);
// 			}
// 			else {
// 				this_pout_cost = this_cost;
// 			}
			
// 			// new max found?
// 			new_max_cost = (this_pout_cost > new_max_cost) ?  this_pout_cost : new_max_cost;
			
// 			// restore routes
// 			msg->routes = routes;
// 		}
		
// 		return new_max_cost;
// 	}
 
// }


//////////// HELPERS //////////////////

/* is_free_vc()
 *  - Check the free_vc_list at an output port to see whether
 *    a given VC is available. O(v)
 * Note:
 *    Use downstream_vc_stats[pout].is_avail() instead,
 *    which approximates it with >99% accuracy in O(1) time.
 */
bool ocin_router_cost::is_free_vc(deque <int> *free_vc_list, int vc_idx) {
	deque <int>::iterator itr;
	for (itr=free_vc_list->begin(); itr != free_vc_list->end(); itr++) {
		if (*itr == vc_idx)
			return true;
	}
	return false;
}

