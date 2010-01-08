#include "ocin_router_xbar.h"


void ocin_router_xbar::init(ocin_router *parent, const string &_name) {
    stringstream *ss = new stringstream();
	*ss << parent->node_info->name.c_str() << "." << _name;
	name = ss->str();
	delete ss;
	
	parent_router = parent;
	
	xbar_q.clear();
	
	// set up the index of the first local port
	_first_ejector_idx = _output_units.size();
	
#ifdef DEBUG
        {
          stringstream tmp;
          tmp  << "Initialized.";
          ocin_name_debug(name,tmp.str());
        }
#endif
}


/* transfer():
 *  - transfer flits from VCs which have been granted xbar access 
 *    by the VC allocator to the destination output units
 */
void ocin_router_xbar::transfer() {
	while (!xbar_q.empty()) {
		// grab the pointer to a source VC
		ocin_router_vc_fifo *src_vc = xbar_q.front();
		xbar_q.pop_front();
		   
		// De-queue the flit @ the head of the queue
		ocin_flit *flit = src_vc->mydeque();
		flit->vc_idx = src_vc->vc_out_index;
		
		// route the flit to the output port
		int p_out = src_vc->p_out;

#ifdef DEBUG
                {
                  stringstream tmp;
                  tmp  << "Transferring flit from msg #" << flit->msg->pid 
                       << ". P_in " << parent_router->vcid2port[src_vc->vc_id]
                       << "(vc_idx " << src_vc->vc_index << ") to p_out " << p_out;
                  ocin_name_debug(name,tmp.str());
                }


#endif

 		// Ejection port is *not* an output port object, so handle it differently
 		if (p_out >= _first_ejector_idx) {
 			int ej_idx = p_out - _first_ejector_idx;
			_ej_port[ej_idx].enque(flit);
		}
		else {
			_output_units[p_out].enque(flit);
		}

	}
}
