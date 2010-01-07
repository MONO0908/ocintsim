#include "ocin_router_cost_mgr.h"

void ocin_router_cost_mgr::init(ocin_router *parent, const string &_name) {
	parent_router = parent;
	name = _name;
}


/* enque_cost_msg()
 *  - implementing classes must override.
 *  - this is the fall-back that will die w/ a message
 */
void ocin_router_cost_mgr::enque_cost_msg(ocin_cost_msg *cost_msg) {
	stringstream tmp;
	tmp << "enque_cost_msg() called. \n"  
		<< "Cost message propagation & aggregation not supported in this cost manager. \n"
		<< "Use a different cost mgr (eg: 1D) or write your own. \n";      
	ocin_name_fatal(name,tmp.str());
	
	exit(0);
}
