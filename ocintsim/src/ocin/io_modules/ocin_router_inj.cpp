#include "ocin_router_inj.h"

void ocin_router_inj::init(ocin_router *parent, 
                          const string &_name,
                          /*gen ptr,*/
                          ocin_router_ru *ru) 
{
	stringstream *ss = new stringstream();
	*ss << parent->node_info->name.c_str() << "." << _name;
	name = ss->str();
	delete ss;
	
	parent_router = parent;
	//_inj_fifos = inj_fifos;
	_ru = ru;

#ifdef DEBUG	
        {
          stringstream tmp;
          tmp  << "Init complete!";
          ocin_name_debug(name,tmp.str());
        }
#endif

}


/* req_pkt() - DEBUG ONLY!!!!!!!!!
 */
void ocin_router_inj::req_pkt() { 
	
    ocin_msg *msg = gen_msg(1);
    msg->src_addr = parent_router->node_info->coord;
    // initialize the destination address
    msg->dest_addr.clear();
    for (int i=0; i<parent_router->node_info->coord.size(); i++) { 
    	msg->dest_addr.push_back(1);
    }
    msg->rt_dimension = 0;
    msg->rt_dir = 0;
    msg->p_out = 0;

    vector<ocin_flit *> ::iterator fitr;
    for (fitr=msg->flits.begin(); fitr != msg->flits.end(); fitr++) {
      (*fitr)->vc_idx=0;
      (*fitr)->idx=0;
      (*fitr)->p_out=0;
      (*fitr)->p_in=0;
      (*fitr)->inj_time=10;
     
      data.push_back(*fitr);
    }

#ifdef DEBUG
    {
      stringstream tmp;
      tmp  << "Generated new msg: " ;
      ocin_name_debug(name,tmp.str());
    }

    {
      stringstream tmp;
      tmp  << msg2str(msg, true /*print flits*/);
      ocin_name_debug(name,tmp.str());
    }

#endif
}

// This function called by the generator to inject flits into the
// injector port.
void ocin_router_inj::push_flit(ocin_flit * flit) {
  
  data.push_back(flit);
}


/* receive()
 *  - determine if the vc fifo can take a new flit
 *  - note that the VC has not been pre-allocated.
 *  - req a new packet from the generator
 */
void ocin_router_inj::receive() {
	// DEBUG
  //	if ((parent_router->node_info->name == "rt0") && (parent_router->cycles()==10)){
  		//req_pkt();
  //	}
	
	// Anything to send?
	if (!data.empty()) {
		ocin_flit *flit = data.front();
		if (flit->is_header) {
			int vc = find_free_vc();
			if (vc < 0)
		  		return;

		  	// got an inj VC. 
		  	// Initialize all flits to this VC
		  	vector <ocin_flit *>::iterator flit_iter;
		  	for (flit_iter = flit->msg->flits.begin(); flit_iter != flit->msg->flits.end(); flit_iter++) {
		  		(*flit_iter)->vc_idx = vc;
		  	} 
		}
		int vc = flit->vc_idx;
		
		if (!_inj_fifos[vc].is_full()) {  // flit can be accepted
			// If header flit, perform routing
			if (flit->is_header) {
				_ru->route(flit);    
			}
			// TODO: to perform look-ahead routing, need an extra RU 
			//       (1 for current hop, 1 for look-ahead) 
			
            // mark when the flit has been injected
            flit->inj_time = ocin_cycle_count;
            if (flit->is_header)
              flit->msg->inj_time = ocin_cycle_count;
//cout << fcns(name) << "Injected a flit into VC " << vc <<endl;

#ifdef DEBUG
            {
              stringstream tmp;
              tmp  << "Injected a flit into INJ VC " << vc;
              ocin_name_debug(name,tmp.str());
            }

            {
              stringstream tmp;
              tmp  << flit2str(flit);
              ocin_name_debug(name,tmp.str());
            }

#endif			
			// Push into a VC FIFO
			_inj_fifos[flit->vc_idx].enque(flit);
			
			// Remove the flit from the input queue
			data.pop_front();
		}
	}
}


/* find_free_vc()
 *  - searches the sets of injection VC and returns the one w/ the shortest que
 */
int ocin_router_inj::find_free_vc() {
	int min_q_idx = -1;  // index of the VC w/ the shortest que
	int min_q_size = 1000000;
	bool found;
	
	for (int i=0; i<_inj_fifos.size(); i++) {
		if (_inj_fifos[i].data.size() < min_q_size) {
		  min_q_idx = i;
		  min_q_size = _inj_fifos[i].data.size();
		  found = true;
		}
	}
	return min_q_idx;
}

// repreroute() This method examines the head of each fifo, if the
// flit there is a header and it needs to be re-prerouted to avoid
// deadlock it does so.
void ocin_router_inj::repreroute() {

  for (int x =0; x < _inj_fifos.size(); x++) {
    if (!(_inj_fifos[x].is_empty())) {
      ocin_flit *flit = _inj_fifos[x].front_flit();
      if (flit->is_header){     // can't reroute a body flit! Do this
                                // test first to avoid unnecessary
                                // accesses

        // only reroute packets delayed more cycles than the
        // repreroute_cycles threshold
        if ((flit->arrival_time + param_repreroute_cycles)
            < ocin_cycle_count) {

          if(flit->msg->dor) {
            continue;           // if the message is already in the
                                // escape channel don't reroute it
                                // otherwise we loose any vc we may
                                // have acquired. Can cause wierd
                                // livelock problem w/ vc allocation.
          }

          // reset the latency so it doesn't count against new channel
          flit->arrival_time = ocin_cycle_count;

#ifdef DEBUG
          {
            stringstream tmp;
            tmp  << "Reprerouting pid: "<< flit->msg->pid <<" Because delay is: "<< 
              (flit->arrival_time + param_repreroute_cycles - ocin_cycle_count);
            ocin_name_debug(name,tmp.str());
          }
#endif

          


          // clear out the existing preroutes and then the route step
          // will re-preroute it
          flit->msg->reroutes++;
          flit->msg->preroutes.clear();
          _ru->route(flit);

          

          // If the input fifo has already gotten a vc we need to put
          // it back on the free vc list
          if (_inj_fifos[x].has_vc) {
            parent_router->free_vc_list[_inj_fifos[x].p_out].push_back(_inj_fifos[x].vc_out_index);
            _inj_fifos[x].has_vc = false;
          }

          // Then reset state in input_fifo to reflect newly re-routed
          // packet
          _inj_fifos[x].update_fifo_head();
          _inj_fifos[x].empty = false;
          _inj_fifos[x].avail = false;

        }
      }
    }
  }
}
