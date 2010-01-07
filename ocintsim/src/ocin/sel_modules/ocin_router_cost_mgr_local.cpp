/* Simple manager that updates the cost register 
 * with its rolling history sum. 
 */

#include "ocin_router_cost_mgr_local.h"

void ocin_router_cost_mgr_local::init(ocin_router *parent, const string &_name) {
	
	// call base init() w/ fully specified name
	stringstream *ss = new stringstream();
	*ss << parent->node_info->name.c_str() << "." << _name;
	ocin_router_cost_mgr::init(parent, ss->str());
	delete ss;
	
#ifdef DEBUG	
        {
          stringstream tmp;
          tmp   << "Initialized. Cost manager = LOCAL";
          ocin_name_debug(name,tmp.str());
        }
#endif
}


/* update_cost()
 *  - simple updater, uses the rolling history sum as the cost 
 */
void ocin_router_cost_mgr_local::update_cost() {
  for (int i=0; i < _cost_regs[0].size(); i++) {
    int cost = _cost_regs[0][i].get_hist_sum(); 
    _cost_regs[0][i].set_cost(cost);
    
// #ifdef DEBUG	
//     {
//       stringstream tmp;
//       tmp   << "Cost reg @ pout " << i << " = " << cost;
//       ocin_name_debug(name,tmp.str());
//     }
// #endif
    
  }
}
