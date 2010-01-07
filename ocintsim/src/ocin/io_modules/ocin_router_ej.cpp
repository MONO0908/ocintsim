#include "ocin_router_ej.h"

void ocin_router_ej::init(ocin_router *parent, const string &_name) {
	stringstream *ss = new stringstream();
	*ss << parent->node_info->name.c_str() << "." << _name;
	name = ss->str();
	delete ss;
	
	parent_router = parent;
	
	int vc_count = param_ej_vc_count;
	int credits = parent_router->node_info->que_depth;
	
	for (int i=0; i<vc_count; i++) {
		ocin_vc_status *v = new ocin_vc_status(credits, i /*vc idx*/, true /*avail*/, &free_vc_list);
		
		// init the status object w/ correct name
		stringstream *ss = new stringstream();
		int port_num = parent->node_info->port_count;
		*ss << parent->node_info->name.c_str() << ".CR" << port_num << "_" << i;
		v->init(ss->str());         
		delete ss;
		
		ej_vc_stats.push_back(*v);
		free_vc_list.push_back(i);
	}
	
	
#ifdef DEBUG
        {
          stringstream tmp;
          tmp  << "Init complete!";
          ocin_name_debug(name,tmp.str());
        }
#endif

}

// TODO: inline this function
void ocin_router_ej::enque(ocin_flit *flit) {

#ifdef DEBUG
  {
    stringstream tmp;
    tmp  << "Receiving a flit.. ";
    ocin_name_debug(name,tmp.str());
  }
#endif 

	ej_fifos[flit->vc_idx].enque(flit);  
}


/* deque()
 *  - deque's the flit from the given VC FIFO
 *  - updates credit count visible to the router
 */
ocin_flit * ocin_router_ej::deque(int vc_idx) {
    // pop the flit from the FIFO
    ocin_flit *flit = ej_fifos[vc_idx].deque();

    // update credits
    ocin_vc_status *v = &ej_vc_stats[vc_idx];

	// Increment the credit count for this ejection VC
	v->inc_credits();

    //return the flit
    return flit;
}

