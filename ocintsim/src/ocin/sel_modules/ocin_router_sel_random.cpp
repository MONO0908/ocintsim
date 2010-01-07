#include "ocin_router_sel_random.h"

void ocin_router_sel_random::init(ocin_router *parent, const string &_name) {
	
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
int ocin_router_sel_random::select(ocin_msg *msg) {
	int pout = -1;  // winning output port
	int valid_ports = msg->routes.size();
	map <int, set <int> >::iterator routes_itr;
	routes_itr = msg->routes.begin();
	
	// Don't bother if only 1 out port is available
	if (valid_ports > 1) {
	  // randomly pick the *index* of the winning port
          unsigned temp = random();

		int pout_idx = temp % valid_ports;
		
		// now figure out the port that this index maps to
		// by adding the index as an offset to the iterator		
		for (int i=0; i<pout_idx; i++) {
			routes_itr++;
		}

#ifdef PARANOID
		// Make sure a valid p_out was found
		if (pout < 0) {
			stringstream tmp;
			tmp << "No valid p_out found!  \n";
			tmp << "Random val = " << pout_idx << endl;
			ocin_name_fatal(name,tmp.str());
			exit(0);
		}
#endif

	}
	pout = (*routes_itr).first; 
	
	return pout;
}

