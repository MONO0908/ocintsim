#include "ocin_router_sel_1match.h"

void ocin_router_sel_1match::init(ocin_router *parent, const string &_name) {
	
	// call base init() w/ fully specified name
	stringstream *ss = new stringstream();
	*ss << parent->node_info->name.c_str() << "." << _name;
	ocin_router_sel::init(parent, ss->str());
	delete ss;
	
	// What's the order of the topology?
	_num_dimensions = parent_router->node_info->coord.size();
}


/* select()
 *  - Pick first available output port
 */
int ocin_router_sel_1match::select(ocin_msg *msg) {
	map <int, set <int> >::iterator routes_itr; 
	routes_itr = msg->routes.begin();
	
	// The key of the first element in the map is the first valid out port
	return (*routes_itr).first;  
}

