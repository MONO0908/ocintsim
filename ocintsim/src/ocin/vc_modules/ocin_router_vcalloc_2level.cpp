#include "ocin_router_vcalloc_2level.h"

void ocin_router_vcalloc_2level::init(ocin_router *parent, const string &_name) {
  stringstream *ss = new stringstream();
  *ss << parent->node_info->name.c_str() << "." << _name;
	
  ocin_router_vcalloc::init(parent, ss->str());
  delete ss;
	
  // set the index of the first local port
  _local_port = parent_router->node_info->port_count;
	
  int total_ports = _vc_fifos.size();  // inc. local port
	
  /* A little explanation for the following mess is in order:
   * To maintain symmetry, inj and ejection ports have the same
   * port number. To maintain sanity, VC FIFOs at these ports
   * have different unique numbers (vc_id).  This keeps vcid2port,
   * vcid2class, and vcid2vcidx maps simple and coherent.
   * Now, port_priority and vc_priority structures are both indexed
   * by vc_id (since they represent arbiters for each *output* VC.
   * So to keep things simple, I include injection VCs as "outputs"
   * in these lists, although they should never be targeted.
   * This allows me to use same vc_id & port numbers everywhere and 
   * not have to deal w/ "holes" in the vc_id space.
   */
	
  // VC priorities at each input port, indexed by the output VC
  // vc_priorities[output_vc][input_port][input_vc]
  // Used for resolving L1 conflicts (multiple VCs at a given input port
  // competing for the same output VC).
  // ==> Note: to support priority classes, another dimension is *NOT* needed -
  //     we only care about priorities *within* each class.
  // ??? Is this over-kill?  Shouldn't there be just VC priorities
  //     at each input port, regardless of output VC? 
  //     ie: vc_priorities[input_port][input_vc]
  for (int i=0; i<total_ports; i++) {                     // for each output port (exc. ejector)
    for (int k=0; k<_vc_fifos[i].size(); k++) {         // for each VC @ out port
      vector <list <int> > *p_in = new vector <list <int> >();
      for (int l=0; l<total_ports; l++) {             // for each input port
        list <int> *vc_in = new list<int>();
        for (int m=0; m<_vc_fifos[l].size(); m++) { // for each VC @ the input port
          vc_in->push_back(m);                    // fixed initial priority order
        }  // for each VC @ input port
        p_in->push_back(*vc_in);
      } // for each input port
		    
      _vc_priorities.push_back(*p_in);
    } // for each VC @ output port
  } // for each output port
	
  // initialize VC priorities @ the ejector port
  //vector <ocin_router_vc_fifo> *ej_fifos = parent_router->ej_fifos;
  for (int i=0; i<_ej_fifos.size(); i++) {
    for (int k=0; k<_ej_fifos[i].size(); k++) {                // for each VC @ eject port
      vector <list <int> > *p_in = new vector <list <int> >(); 
      for (int l=0; l<total_ports; l++) {                 // for each input port
        list <int> *vc_in = new list<int>();
        for (int m=0; m<_vc_fifos[l].size(); m++) {     // for each VC @ the input port
          vc_in->push_back(m);                        // fixed initial priority order
        }  // for each VC @ input port
        p_in->push_back(*vc_in);
      } // for each input port
      _vc_priorities.push_back(*p_in);
    } // for each VC @ ejection port
  }
	
	
  // For each (output) VC, keep a list of port priorities for resolving 
  // L2 conflicts (multiple ports competing for same VC)
  // port_priorities[output_vc][input_port]
  // ==> Note: to support priority classes, another dimension is *NOT* needed -
  //     we only care about priorities *within* each class!!!!
  for (int i=0; i<total_ports; i++) {                    // for each out port (exc. eject) 
    for (int k=0; k<_vc_fifos[i].size(); k++) {         // for each vc @ out port
      list <int> *p_in = new list<int>();
      for (int l=0; l<total_ports; l++) {             // for each input port
        p_in->push_back(l);
      }
      _port_priorities.push_back(*p_in);
    } // for each VC @ output port
  } // for each output port
	
  // initialize port priorities @ the ejector port
  for (int i=0; i<_ej_fifos.size(); i++) {
    for (int k=0; k<_ej_fifos[i].size(); k++) {         // for each vc @ out port
      list <int> *p_in = new list<int>();
      for (int l=0; l<total_ports; l++) {           // for each input port
        p_in->push_back(l);
      }
      _port_priorities.push_back(*p_in);
    } // for each VC @ output port
  }
	

#ifdef DEBUG
  {
    stringstream tmp;
    tmp  << "Initialized.";
    ocin_name_debug(name,tmp.str());
  }
#endif

}

/* allocate_vc()
 *  - arbitrates among all reqs for VCs at each output port
 */
void ocin_router_vcalloc_2level::allocate_vc() {
  int i;
  int p_in = 0; 
  int ports = _vc_fifos.size();  
  vector <vector <ocin_router_vc_fifo> >::iterator vc_fifos_i;
  //vector <ocin_router_vc_fifo>::iterator vc_fifos_j;
  vector <ocin_router_vc_fifo>::iterator vc_fifos_k;

  map <int, vector <int> > map_dest2pin;
  map <int, vector <int> >::iterator dest2pin_itr;
  // iterate over all ports (inc. injection port)
  for (vc_fifos_i=_vc_fifos.begin(); vc_fifos_i != _vc_fifos.end(); vc_fifos_i++, p_in++) {
	  
    // iterate over classes (vc_fifos_j) here
	  
    map <int, vector<int> > map_dest2vcin; 
    map_dest2vcin.clear();
    map <int, vector<int> >::iterator dest2vcin_itr;
    // iterate over the VCs @ this port
    for (vc_fifos_k=vc_fifos_i->begin(); vc_fifos_k != vc_fifos_i->end(); vc_fifos_k++) {
			
      // Level 1(a) - for each FIFO requesting a VC, perform v:1 reduction 
      //           so that only 1 output VC is requested by each input VC.
      if (!vc_fifos_k->needs_vc) 
        continue;


      ocin_msg *msg = vc_fifos_k->front_msg();

      // Here we implement the delay for
      // reprerouting due to short preroutes list.
      if (msg->add_rrt_delay == true) {
        if ((msg->flits[0]->arrival_time + param_repreroute_delay) 
            > ocin_cycle_count) {
          continue; // haven't surpassed delay yet
          // so skip this vc fifo
        } else {
          msg->add_rrt_delay = false; // delay complete, clear flag
       }
      }


      // get the dest_port for the requesting VC
      //int p_out = parent_router->route_sel->select(msg);
			
      // Update the routing info for the input FIFO w/ the requested output port.
      // Doing it here simplifies the indexing for monitors here and in XB.
      //vc_fifos_k->p_out = p_out;

      int p_out = vc_fifos_k->p_out;
			
#ifdef DEBUG
      {
        stringstream tmp;
        tmp  << "request: msg #" << msg->pid
             << ". P_in " << p_in << ", vc idx " << vc_fifos_k->vc_index 
             << " (src vc_id = " << vc_fifos_k->vc_id << ")."
             << " P_out = " << p_out;
        ocin_name_debug(name,tmp.str());
      }

#endif

      // grab a ptr to the free_vc_list at the selected output port
      deque <int> *free_vc_list_i = &(_free_vc_list[p_out]);
			
      /* find a matching VC (scan the free list until match w/ vc_set element) */
      int dest_vc_index = find_free_vc(free_vc_list_i, msg, p_out); 
      if (dest_vc_index < 0) { 
#ifdef OCIN_MON	
        // VC stall
        _pin_mon[p_in][p_out].vmon.stalls++;
        _pout_mon[p_out][p_in].vmon.stalls++;
#endif
				
        continue;
      }
			
      /* track destination VCs requested by msgs @ this input port.
       * Note that I'm using symmetry (the fact that # of VCs at input ports is
       * the same as downstream - ie, at output ports) to generate a unique VC tag.
       * In fact, it has nothing to do with a VC w/ same id at this router */
      int dest_vc_id = (p_out >= _local_port) ?
        //(*parent_router->ej_fifos)[dest_vc_index].vc_id :
        _ej_fifos[p_out - _local_port][dest_vc_index].vc_id :
        _vc_fifos[p_out][dest_vc_index].vc_id;

      map_dest2vcin[dest_vc_id].push_back(vc_fifos_k->vc_id);

#ifdef OCIN_MON			
      // keep track of requests from this input port
      _pin_mon[p_in][p_out].vmon.inc_reqs();       // inc both local & global reqs @ p_in
      _pout_mon[p_out][p_in].vmon.global_reqs++;   // inc only global reqs @ p_out
#endif


    } // for all VCs @ this port
		
    // Level 1(b)
    //    For this input port, ensure that there's at most 1 request for each output VC
    //    (perform a v:1 reduction for each output VC).
    //    Note: this still permits multiple output VC requests from this port, but they
    //    must be for different output VCs.
    for (dest2vcin_itr=map_dest2vcin.begin(); dest2vcin_itr != map_dest2vcin.end(); dest2vcin_itr++) {
      int src_vc_id = -1;                         // winner VC (continues to P:1 arbiter)
      int dest_vc_id = dest2vcin_itr->first;
      vector <int> *vcin_vector = &(dest2vcin_itr->second);
      if (vcin_vector->size() > 1) {
        /* choose highest priority request */
        src_vc_id = priority_match( &(_vc_priorities[dest_vc_id][p_in]), 
                                    vcin_vector,
                                    parent_router->vcid2vcidx,
                                    false /*don't update priorities*/);
				
        if (src_vc_id < 0) {
          stringstream tmp;
          tmp  << "Error prioritizing among VCs from a given input port ";     
          tmp  << "\t P_in = " << p_in;      
          tmp  << "\t Requests (untranslated) = " << vector2str(vcin_vector) ;     
          tmp  << "\t Priorities = " << list2str(&_vc_priorities[dest_vc_id][p_in]) ;     
          ocin_name_fatal(name,tmp.str());
          exit(0);
        }
				
      }
      else {
        src_vc_id = vcin_vector->front();
      }
			
#ifdef OCIN_MON	
      // record a local win @ this p_in AND local req @ the p_out
      int p_out = parent_router->vcid2port[dest_vc_id];
      _pin_mon[p_in][p_out].vmon.local_gnts++;
      _pout_mon[p_out][p_in].vmon.local_reqs++;
#endif
			
      /* Track destination VCs requested by input ports.
       * The way we'll do it is by keeping track of source vc_id,
       * which can be mapped to the input port via the router's vc2port map.
       */
      map_dest2pin[dest_vc_id].push_back(src_vc_id); 
    } // for all requested VCs from this input port & class
			
    // iterate over the classes
	  
  } // for all ports
	
  // Level 2
  //    For each output VC, choose the winning input port (p:1 reduction)
  for (dest2pin_itr = map_dest2pin.begin(); dest2pin_itr != map_dest2pin.end(); dest2pin_itr++) {
    int src_vc_id = -1;                         // winner VC (acquires the output VC)
    int dest_vc_id = dest2pin_itr->first;
    vector <int> *pin_vector = &(dest2pin_itr->second);
    if (pin_vector->size() > 1) {
      /* choose highest priority req & update priorities */
      src_vc_id = priority_match( &(_port_priorities[dest_vc_id]), 
                                  pin_vector, 
                                  parent_router->vcid2port  /*by ref*/,
                                  true /*update priority*/);
			
      if (src_vc_id < 0) {
        stringstream tmp;
        tmp  << "Error prioritizing among input ports @ given output port";     
        tmp  << "\t Requests (untranslated) = " << vector2str(pin_vector);     
        tmp  << "\t Priorities = " << list2str(&_port_priorities[dest_vc_id]);     
        ocin_name_fatal(name,tmp.str());
        exit(0);
      }
    }
    else {
      src_vc_id = pin_vector->front();

      /* update priorities */
      int p_in = parent_router->vcid2port[src_vc_id];
      update_priority( &(_port_priorities[dest_vc_id]), p_in);
    }
		
    int p_in = parent_router->vcid2port[src_vc_id];
    int p_out = parent_router->vcid2port[dest_vc_id];
    int vc_in_index = parent_router->vcid2vcidx[src_vc_id];
    int vc_out_index = parent_router->vcid2vcidx[dest_vc_id];
		
#ifdef OCIN_MON	
    // record a win @ this out port
    _pin_mon[p_in][p_out].vmon.global_gnts++;   // inc global gnts @ p_in
    _pout_mon[p_out][p_in].vmon.inc_gnts();   // inc only global reqs @ p_out
#endif

    /* Update state */
		 		
    // 1. Grant the winning VC (update VC FIFO control)
    ocin_router_vc_fifo *vc_in_ptr = &(_vc_fifos[p_in][vc_in_index]); 
    vc_in_ptr->needs_vc = false;
    vc_in_ptr->has_vc = true;
    // Output port is updated immediately following Selection.
    // Although the update is speculative (VC sel might be lost),
    // it's used in stats monitoring.
    //vc_in_ptr->p_out = p_out;
    vc_in_ptr->vc_out_index = vc_out_index;
		
    // 2. Update msg header
    ocin_msg *msg = vc_in_ptr->front_msg();
    msg->p_out = p_out;
    msg->rt_dimension = get_bec_dim(p_out, _local_port);
    msg->vc_out_index =  vc_out_index;
    //cout << fcns(name) << "P_out = " << p_out << ", Dim = " << msg->rt_dimension << ". " << msg->src << "->" << msg->dst << endl; 

    // 3. Update the free_vc_list and downstream credit base
    unfree_vc(p_out, vc_out_index);

    //parent_router->downstream_vc_stats[p_out][vc_out_index].avail = false;
    parent_router->downstream_vc_stats[p_out][vc_out_index].allocate(msg);
				
    // 4. update input VC priorities @ output VC.
    // Note that port priorities were updated in place in priority_match(),
    // since the winner at that stage is "final," whereas the winner
    // in the vc_in selection stage is not.
    update_priority( &(_vc_priorities[dest_vc_id][p_in]), vc_in_index);

#ifdef DEBUG
    {
      stringstream tmp;
      tmp  << "winner!!! msg #" << msg->pid
           << ". P_in = " << p_in << ", vc idx = " << vc_in_index
           << " (src vc_id: " << src_vc_id << "). "
           << "P_out = " << p_out << " (dest vc_idx = " << vc_out_index  << "). Status:";
      ocin_name_debug(name,tmp.str());
    }

    {
      stringstream tmp;
      tmp  << "  updated port priorities: " << list2str(&(_port_priorities[dest_vc_id]));
      ocin_name_debug(name,tmp.str());
    }

    {
      stringstream tmp;
      tmp  << "  updated VC priorities: " << list2str(&(_vc_priorities[dest_vc_id][p_in]));
      ocin_name_debug(name,tmp.str());
    }
                
    //vc_in_ptr->print();
#endif
  } // for all requested VCs 

}

/* find_free_vc():
 *  - iterate over the free_vc_list
 *  - if an available VC matches one of the options produced by the RU, return it.
 *  - if a VC cannot be allocated, return -1
 */
inline int ocin_router_vcalloc_2level::find_free_vc(deque <int> *free_vc_list, ocin_msg *msg, int p_out) {
  if (free_vc_list->empty()) {

#ifdef DEBUG
    {
      stringstream tmp;
      tmp  << "free_vc_list is empty.";
      ocin_name_debug(name,tmp.str());
    }
          
#endif
    return -1;
  }
	  
  set <int> *vc_set = &(msg->routes[p_out]);

#ifdef DEBUG
  {
    stringstream tmp;
    tmp  << "Finding a free VC... ";
    tmp  << "\t vc_set = " << set2str(vc_set);
    tmp  << "\t free_vc_list = " << deque2str(free_vc_list);
    ocin_name_debug(name,tmp.str());
  }
#endif

  deque <int>::iterator itr;
  for (itr=free_vc_list->begin(); itr != free_vc_list->end(); itr++) {
    int vc_idx = *itr;                  // iter points to the index of a free VC on the free_list
    if (vc_set->count(vc_idx) > 0)
      return vc_idx;
  }
  return -1;
}


/* unfree_vc()
 *  - remove a VC from the free list
 *  TODO: free_list is a deque, making erase inefficient. Change it to list.
 */
inline void ocin_router_vcalloc_2level::unfree_vc(int p_out, int vc_idx) {
  deque <int> *free_vc_list_i = &(_free_vc_list[p_out]);
  deque <int>::iterator itr;
  int i=0;
  for (itr=free_vc_list_i->begin(); itr != free_vc_list_i->end(); itr++, i++) {
    if (*itr == vc_idx) {
      free_vc_list_i->erase(itr);
      return;
    }
  }
}
