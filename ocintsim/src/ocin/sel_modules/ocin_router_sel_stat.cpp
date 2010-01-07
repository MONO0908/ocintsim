/* Statistical selector 
 * 
 * Note: the selector tries to MAXIMIZE the cost. 
 * (eg: highest # of free VCs; highest # of credits)
 */

#include "ocin_router_sel_stat.h"

void ocin_router_sel_stat::init(ocin_router *parent, const string &_name) {
	
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
int ocin_router_sel_stat::select(ocin_msg *msg) {
	// The key in each of the following maps is the output port number
	map <int, set <int> >::iterator routes_itr;
	map <int, int > costs;
	map <int, int > ranges;
	int costs_sum = 0; 
	int pout = -1;  // final choice of output port based on link cost
	
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
			int scaled_cost = cost + 1;
			costs[p_out] = scaled_cost;
			
			// Keep track of the sum of all costs
			costs_sum += scaled_cost;
			
#ifdef DEBUG	
    {
      stringstream tmp;
      tmp   << "Cost = " << cost << " (p_out " << p_out << ")" << endl;
      ocin_name_debug(name,tmp.str());
    }
#endif
	
		}

		// Convert costs to weights
		// TODO: this is appalling in its inefficiency.
		for (routes_itr = msg->routes.begin(); routes_itr != msg->routes.end(); routes_itr++) {
			int p_out = (*routes_itr).first;
			float prob = (float)costs[p_out] / (float)costs_sum;

			// convert a prob into an int weight
			float weight = prob * 100;
			// DELETE
			//int weight_inv = 100 - (int)weight;
			
			ranges[p_out] = (int)weight;			
		}	
		
		// Convert individual weights into additive ranges
		int running_sum = 0;
		for (routes_itr = msg->routes.begin(); routes_itr != msg->routes.end(); routes_itr++) {
			int p_out = (*routes_itr).first;
			ranges[p_out] = ranges[p_out] + running_sum;
			running_sum = ranges[p_out];
		}
		
		// Roll the dice
                unsigned temp = random();
		int random_val = temp % running_sum;
		
		// Find the range corresponding to the random value
		map <int, int>::iterator ranges_itr;
		
		for (ranges_itr = ranges.begin(); ranges_itr != ranges.end(); ranges_itr++) {
			int p_out = (*ranges_itr).first;
			if (random_val < ranges[p_out]) {
				pout = p_out;
				break; 
			}
		}
		
		// Make sure a valid p_out was found
		if (pout < 0) {
			stringstream tmp;
			tmp << "No valid p_out found!  \n";
			tmp << "Random val = " << random_val << endl;
			tmp << "Highest range limit = " << running_sum << endl;
			ocin_name_fatal(name,tmp.str());
			exit(0);
		}
		
	}
	else {  // Only 1 output port available
		pout = (*routes_itr).first; 
	}

#ifdef DEBUG	
        {
          stringstream tmp;
          tmp   << "Final p_out = " << pout << endl;
          ocin_name_debug(name,tmp.str());
        }
#endif

	return pout;
}

