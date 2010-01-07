/* Non-speculative 2-level XBar allocator with high matching power for EC/BECs.
 * High matching power means that each physical input port can route to any logical 
 * output. This matters for BECs, where the regular XB allocator (xballoc_2level) 
 * only matches logical inputs to logical outputs.
 * The allocator is compatible w/ a basic mesh, ECs and BECs.
 * In the BEC configuration, the allocator has low matching power,
 * as it treats all BEC inputs at a given logical input port as one port, 
 * allowing just one trasfer per cycle from a logical input to a logical output.
 */

#include "ocin_router_xballoc_bec_power_2level.h"

void ocin_router_xballoc_bec_power_2level::init(ocin_router *parent, const string &_name) {
	stringstream *ss = new stringstream();
	*ss << parent->node_info->name.c_str() << "." << _name;
	
	ocin_router_xballoc::init(parent, ss->str());
	delete ss;
	
	int num_pports = _vc_fifos.size();  // input ports
	int num_lports = _lvc_fifos.size(); // output ports
	
	// Priorities for each VC at every input port for resolving L1
	// conflicts (multiple VCs competing for the XBar)
	// ==> Note: to support priority classes, another dimension is *NOT* needed -
	//     we only care about priorities *within* each class!!!!
	for (int i=0; i<num_pports; i++) {                     // for each input port
		list <int> *vc_in = new list<int>(); 
		for (int k=0; k<_vc_fifos[i].size(); k++) {       // for each VC @ this input port
			vc_in->push_back(k);                          // fixed initial priority order
		} // for each input port    
		_vc_priorities.push_back(*vc_in);
	} // for each input port
	
	
	// Priorities for each input port at output ports for resolving L2
	// conflicts (multiple input ports competing for a given output)
	// ==> Note: to support priority classes, another dimension is *NOT* needed -
	//     we only care about priorities *within* each class!!!!
	for (int i=0; i<num_lports; i++) {                      // for each output port
		list <int> *lp_in = new list<int>();
		for (int k=0; k<num_pports; k++) {                  // for each input port
			lp_in->push_back(k);
		}
		_port_priorities.push_back(*lp_in);
	} // for each output port
	
	
#ifdef DEBUG
        {
          stringstream tmp;
          tmp  << "Initialized.";
          ocin_name_debug(name,tmp.str());
        }
#endif

}

/* allocate_xb()
 *  - arbitrates among all reqs for VCs at each output port
 */
void ocin_router_xballoc_bec_power_2level::allocate_xbar() {
	int i;
	int vc_in = 0;
	int pports = _vc_fifos.size(); 
	int lports = _lvc_fifos.size();
	vector <int> ::iterator req_itr;  
	
	map <int, vector <int> > map_pout2pin;
	map <int, vector <int> >::iterator pout2pin_itr;
	
	// iterate over all input ports (inc. injection port)
	for (int p_in=0; p_in < pports; p_in++) {
	
	  // iterate over classes (vc_fifos_j) here
		vector <int> pin_requests;
			
	    // iterate over the VCs @ this port
	    for (vc_in = 0; vc_in < _vc_fifos[p_in].size(); vc_in++) {
	    	//if (!_lvc_fifos[lp_in][lvc_in]->needs_xb)   // speculative
			if (!_vc_fifos[p_in][vc_in].needs_xbar() )  // non-speculative (has_vc=true)
				continue;

	    	int p_out = _vc_fifos[p_in][vc_in].p_out;
	    	int lp_out = parent_router->port2lport[p_out];

			// get the dest_port & vc index for the requesting VC
			int vc_out_idx = _vc_fifos[p_in][vc_in].vc_out_index;

#ifdef DEBUG
                        {
                          stringstream tmp;
                          tmp  << "request: msg #" << _vc_fifos[p_in][vc_in].front_msg()->pid
                          	   << ". p_in " << p_in  << " (vin idx = " << vc_in << "). " 
                               << "P_out = " << p_out << " (vout idx = " << vc_out_idx << "). "
                          	   << "Credits = " << _downstream_vc_stats[p_out][vc_out_idx].credits << endl;
                          ocin_name_debug(name,tmp.str());
                        }
#endif
			
			// check if the dest VC has credits.
			// This check is done only for non-speculative reqs
			if (_downstream_vc_stats[p_out][vc_out_idx].credits < 1) {
#ifdef OCIN_MON	
				// buffer stall
				_pin_mon[p_in][lp_out].xmon.stalls++;
				_pout_mon[lp_out][p_in].xmon.stalls++;
#endif
				continue;
			}
			
			/* track destination VCs requested by msgs @ this input port */
			pin_requests.push_back(vc_in);
#ifdef OCIN_MON	
			// keep track of requests from this input port
			_pin_mon[p_in][lp_out].xmon.inc_reqs();       // inc both local & global reqs @ p_in
			_pout_mon[lp_out][p_in].xmon.global_reqs++;   // inc only global reqs @ p_out
#endif

		} // for all VCs @ this port
		
		// any requests at this input port?
		if (pin_requests.size() == 0)
		  continue;
		
		// Level 1
		//    Choose 1 VC from this input port (v:1) that will arbitrate for an output port
		int src_vc_idx = -1;
		if (pin_requests.size() > 1) {
			/* choose highest priority request */
			src_vc_idx = priority_match( &(_vc_priorities[p_in]), 
				                         &pin_requests,
				                         false /*don't update priorities*/);
				
			if (src_vc_idx < 0) {
                          {
                            stringstream tmp;
                            tmp  << "Error prioritizing among VCs from a given input port \n";     
                            ocin_name_fatal(name,tmp.str());
                            exit(0);
                          }
			}
		}
		else {
			src_vc_idx = pin_requests.front();
		}
		
		// record a local win @ this p_in AND local req @ the p_out
		int p_out = _vc_fifos[p_in][src_vc_idx].p_out;
		int lp_out = parent_router->port2lport[p_out];
#ifdef OCIN_MON	
		_pin_mon[p_in][lp_out].xmon.local_gnts++;
		_pout_mon[lp_out][p_in].xmon.local_reqs++;
#endif

		/* Track destination ports requested by input ports. */
		int src_vc_id = _vc_fifos[p_in][src_vc_idx].vc_id;
		map_pout2pin[lp_out].push_back(src_vc_id);		 
		// for all requested VCs from this input port & class
					
	  // iterate over the classes
	  
	} // for all ports
	
	// Level 2
	//    For each output port, choose the winning input port (p:1 reduction)
	for (pout2pin_itr = map_pout2pin.begin(); pout2pin_itr != map_pout2pin.end(); pout2pin_itr++) {
		int src_vc_id = -1;                         // winner VC (acquires the output port)
		int lp_out = pout2pin_itr->first;
		vector <int> *pout_requests = &(pout2pin_itr->second);
		if (pout_requests->size() > 1) {
			/* choose highest priority req & update priorities */
			src_vc_id = priority_match( &(_port_priorities[lp_out]), 
			                            pout_requests, 
			                            parent_router->vcid2lport  /*by ref*/,
			                            true /*update priority*/);
			
			if (src_vc_id < 0) {
                          {
                            stringstream tmp;
                            tmp  << "Error prioritizing among input ports @ given output port \n";     
                            ocin_name_fatal(name,tmp.str());
                          }

				exit(0);
			}
		}
		else {
			src_vc_id = pout_requests->front();
			/* update priorities */
			int p_in = parent_router->vcid2port[src_vc_id];
			update_priority( &(_port_priorities[lp_out]), p_in);
		}
		
		int p_in = parent_router->vcid2port[src_vc_id];
		int vc_in_index = parent_router->vcid2vcidx[src_vc_id];
		
		// record a win @ this out port
#ifdef OCIN_MON	
		_pin_mon[p_in][lp_out].xmon.global_gnts++;     // inc only global gnts @ p_in
		_pout_mon[lp_out][p_in].xmon.inc_gnts();       // inc both local & global gnts @ p_out
#endif

		/* Update state */
		
		// 1. Grant the winning VC (update VC control)
		ocin_router_vc_fifo *vc_in_ptr = &_vc_fifos[p_in][vc_in_index];
		vc_in_ptr->needs_xb = false;
		
		// 2. update input VC priorities @ the winning input port.
		// Note that input port priorities were updated in place in priority_match(),
		// since the winner at that stage is "final," whereas the winner
		// in the vc_in selection stage is not.
		update_priority( &(_vc_priorities[p_in]), vc_in_index);
		
		// 3. Update the downstream credit base
		int p_out = vc_in_ptr->p_out;
		int vc_out_idx = vc_in_ptr->vc_out_index;
		ocin_vc_status *v = &(parent_router->downstream_vc_stats[p_out][vc_out_idx]);
		bool end_of_msg = vc_in_ptr->front_flit()->is_tail;
		v->dec_credits(end_of_msg);

		// 4. Generate upstream credit
		if (vc_in_ptr->credit_port != NULL) {
			gen_credit_update(vc_in_ptr);
		}
		else {
#ifdef DEBUG
                  {
                    stringstream tmp;
                    tmp  <<"No upstream credit from p_in " << p_in 
                         << " (this is likely an inj port)";
                    ocin_name_debug(name,tmp.str());
                  }

#endif
		}
		
		// 5. Notify the x-bar of the decision
		_xbar->enque(vc_in_ptr);
		
#ifdef DEBUG
                {
                  stringstream tmp;
                  tmp  << "Winner!!! msg #" << vc_in_ptr->front_msg()->pid
                       << ". p_in = " << p_in << ", vc_idx = " << vc_in_index
                       << "(src vc_id: " << src_vc_id << "). "  
                       << "lp_out = " << lp_out << " (vc_out idx = " << vc_in_ptr->vc_out_index <<"). ";
                  tmp << ". Status:" << endl;
                  tmp  << "  updated VC priorities @ p_in: " << list2str(&(_vc_priorities[p_in])) << endl;
                  tmp  << "  updated p_in priorities @ p_out: " << list2str(&(_port_priorities[lp_out]));
                  ocin_name_debug(name,tmp.str());
                }

                //vc_in_ptr->print();
#endif
	} // for all requested VCs 
}


/* gen_credit_update()
 *  - create a new credit object
 *  - enque it in the outbound queue @ the appropriate output unit.
 */
inline void ocin_router_xballoc_bec_power_2level::gen_credit_update(ocin_router_vc_fifo *vc_in_ptr) {
	ocin_credit *cr = new ocin_credit(1 /*inc amt*/, vc_in_ptr->vc_index);
	vc_in_ptr->credit_port->enque(cr);
}

