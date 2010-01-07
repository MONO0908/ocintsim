/* Selector that uses only info local to current node
 * to make its decision. 
 * Note that buffer utilization @ downstream neighbors is 
 * local information (it's used by both VC & X-bar allocators).
 * 
 * Note: the selector tries to MAXIMIZE the cost. 
 * (eg: highest # of free VCs; highest # of credits)
 */

#include "ocin_router_sel_local.h"

void ocin_router_sel_local::init(ocin_router *parent, const string &_name) {
	
  // call base init() w/ fully specified name
  stringstream *ss = new stringstream();
  *ss << parent->node_info->name.c_str() << "." << _name;
  ocin_router_sel::init(parent, ss->str());
  delete ss;
	
  // What's the order of the topology?
  _num_dimensions = parent_router->node_info->coord.size();
}


/* select()
 *  - Returns the output port corresponding to the lowest link cost
 *  - Link cost is computed using the user-specified rt_cost_fn param
 *  - Cost computation is skipped if only 1 output port is available.
 */
int ocin_router_sel_local::select(ocin_msg *msg) {
  map <int, set <int> >::iterator routes_itr;
  int min_cost = MAX_INT;
  //int max_cost = -100000; 
  // Keep track of costs for each link
  vector <int> cost_vector;
  // If multiple links have same cost, will resolve randomly
  vector <int> min_links;
  int pout;  // final choice of output port based on link cost
	
#ifdef PARANOID
  if (msg->routes.size() > _num_dimensions) {
    stringstream tmp;
    tmp << "More than 2 routes found for a 2D mesh!! \n";
    ocin_name_fatal(name,tmp.str());
    exit(0);
  } 
#endif

  // For each possible output port, compute the cost.
  routes_itr = msg->routes.begin();
  // Don't bother if only 1 out port is available
  if (msg->routes.size() > 1) {
    for (; routes_itr != msg->routes.end(); routes_itr++) {
      int p_out = (*routes_itr).first;
      // skip any port w/ no valid VCs
      if (((*routes_itr).second).empty()) {
        continue;
      }
			
      int cost = _cost_fn->cost(p_out, msg); 
      cost_vector.push_back(cost);

      // keep track of the highest score thus far		 
      if (cost < min_cost) {
        min_cost = cost;
      }

      // DELETE
      //cout << "Cost = " << cost << " (p_out " << p_out << ")" << endl;


#ifdef DEBUG	
      {
        stringstream tmp;
        tmp   << "Cost = " << cost << " (p_out " << p_out << ")";
        ocin_name_debug(name,tmp.str());
      }
#endif
	
    }
		
#ifdef PARANOID
    // Make sure there's at least 1 valid output port.
	
    // Don't have to keep this turned on at all times, 
    // since occurence of this will inevitably lead to deadlock,
    // which should be caught  :)
    if (min_cost == -1) {
      stringstream tmp;
      tmp << "\nNo output port w/ a valid set of VCs!";      
      ocin_name_fatal(name,tmp.str());
      exit(0);
    }
#endif

    // Do a second pass to figure out which port(s) is(are) most profitable.
    // Multiple ports are OK - will break ties at the end.
    int i=0;
    for (routes_itr = msg->routes.begin(); routes_itr != msg->routes.end(); routes_itr++, i++) {

      int p_out = (*routes_itr).first;
      int cost = cost_vector[i];

      if (cost == min_cost) {
        min_links.push_back(p_out);
      }
    }	
		
    // Multiple ports w/ same cost?  Break the tie..
    pout = (min_links.size() > 1) ? break_tie(min_links) : min_links.front();

    // Check if chosen output port has an available VC??
    if (param_adaptive_1avail > 0) {
      // Make sure winner has at least 1 avail VC
      if (!has_free_vc(msg, pout)) {
        // pick any output port w/ an available VC
        int first_avail_pout = first_avail(msg);
        pout = (first_avail_pout > -1) ? first_avail_pout : pout;

#ifdef DEBUG	
        {
          stringstream tmp;
          tmp << "Chosen pout has no VCs. First avail pout = " << first_avail_pout << ", final pout = " << pout << endl;
          ocin_name_debug(name, tmp.str());
        }
#endif
      }
    }
	
  }
  else {  // Only 1 output port available
    pout = (*routes_itr).first; 
  }
	
  // DELETE
  //cout << "Final p_out = " << pout << endl;

#ifdef DEBUG	
  {
    stringstream tmp;
    tmp   << "Final p_out = " << pout;
    ocin_name_debug(name,tmp.str());
  }
#endif

  return pout;
}


/* break_tie()
 *  - Randomly break ties
 */
inline int ocin_router_sel_local::break_tie(vector <int> min_links) {
  int ports = min_links.size();
  unsigned temp = random();
  unsigned winner = temp % ports;

#ifdef DEBUG	
  {
    stringstream tmp;
    tmp   << "Randomly breaking a tie.. winning p_out = " << min_links[winner];
    ocin_name_debug(name,tmp.str());
  }
#endif

  return min_links[winner];
}


/* has_free_vc()
 *  - checks that the selected output port has at least 1 avail VC
 */
bool ocin_router_sel_local::has_free_vc(ocin_msg *msg, int p_out) {
			
  // ptr to downstream credit base @ selected p_out
  vector <ocin_vc_status> &pout_vcs = parent_router->downstream_vc_stats[p_out];
  // ptr to the valid set of VCs @ selected p_out
  set <int> &vc_set = msg->routes[p_out];
	
  // Iterate over all the VCs @ the selected p_out
  // For each VC, check if it's available & can be used by this msg 
  for (int dest_vc=0; dest_vc < pout_vcs.size(); dest_vc++) { 
    if (pout_vcs[dest_vc].is_avail() &&
        (vc_set.count(dest_vc) > 0))
      {
        // Terminate the search once a valid & avail VC is found
        return true;
      }
  }
	
  return false;
}


/* first_avail()
 *  - finds the first p_out w/ an available VC
 */
int ocin_router_sel_local::first_avail(ocin_msg *msg) {
  map <int, set <int> >::iterator routes_itr; 
  routes_itr = msg->routes.begin();
  int p_out = (*routes_itr).first;
	
  bool match = false;
	
  if (msg->routes.size() > 1) {
    for (; routes_itr != msg->routes.end(); routes_itr++) {
      p_out = (*routes_itr).first;
			
      match = has_free_vc(msg, p_out);
			
      // If an avail VC has been found, terminate the search
      if (match)
        break;
		
    } // for each valid output port
  } // if (routes > 1) 
	
  return (match ? p_out : -1);  
}
