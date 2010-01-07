/* Last match selector (to be used w/ adaptive routing function).
 * Similar to First Match (sel_1match), but returns the *last* output
 * port produced by the routing function.  The result is that a packet
 * will traverse dimension in reverse order (eg: Y-X)
 */

#include "ocin_router_sel_lastmatch.h"

void ocin_router_sel_lastmatch::init(ocin_router *parent, const string &_name) {
	
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
int ocin_router_sel_lastmatch::select(ocin_msg *msg) {
	map <int, set <int> >::reverse_iterator routes_ritr;  // Reverse iterator 
	routes_ritr = msg->routes.rbegin();
	
	// The key of the first element in the map is the first valid out port
	return (*routes_ritr).first;  
}

