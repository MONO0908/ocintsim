#include "ocin_router_iu.h"

/* Input Unit
 * Pulls flits & control info from the channel, and routes them to 
 * where they need to be.
 * The reason why this piece is useful (as opposed to directly sticking flits
 * into the fifo) is that it enables flit steering to path sets & early ejection.
 * It's also a more faithful model of bypassing.
 */
 

void ocin_router_iu::init(ocin_router *parent, 
                          const string &_name, 
                          int port_num,
                          ocin_channel *c_in,
                          ocin_router_ru *ru)
//vector <ocin_vc_status> &vc_stats,
//vector <ocin_router_vc_fifo> &input_fifos) 
{
  stringstream *ss = new stringstream();
  *ss << parent->node_info->name.c_str() << "." << _name;
  name = ss->str();
  delete ss;
	
  parent_router = parent;
  channel_in = c_in;

  local_addr = parent_router->node_info->coord; // grab this address
                                                // for later use
  _ru = ru;
  _port_num = port_num;
	
  // create VC status array (keeps track of downstream credits on this channel)
  // TODO: Lacks support for multiple path sets, variable number of credits, or even vc classes!
  int vc_count = parent_router->node_info->vc_count;
  int credits = parent_router->node_info->que_depth;
	
  for (int i=0; i<vc_count; i++) {
    ocin_vc_status *v = new ocin_vc_status(credits, i /*vc idx*/, true /*avail*/, &_free_vc_list);
		
    // init the status object w/ correct name
    stringstream *ss = new stringstream();
    *ss << parent->node_info->name.c_str() << ".CR" << port_num << "_" << i;
    v->init(ss->str());         
    delete ss;
		
    _downstream_vc_stats.push_back(*v);
    _free_vc_list.push_back(i);
  }
	
#ifdef DEBUG
  {
    stringstream tmp;
    tmp   << "Init complete. ";
    ocin_name_debug(name,tmp.str());
  }

  if (c_in != NULL) {
    stringstream tmp;
    tmp  << " Input: " << c_in->name;
    ocin_name_debug(name,tmp.str());
  } else {
    stringstream tmp;
    tmp  << " Input: NONE";
    ocin_name_debug(name,tmp.str());
  }
#endif
}

/* receive()
 *  - check if this is a valid physical interface
 *  - pull arrived flit(s) from the channel and push them into the input fifo
 *  - pull arrived credit(s) from the channel and update the credit map
 */
void ocin_router_iu::receive() {
  if (channel_in == NULL) {
    //cout << name.c_str() << ": Nothing to do this cycle (no input channel) \n";
    return;
  }
	
  tsim_u64 cycles = ocin_cycle_count;  // current cycle count
	
  /* Process flits */
	
  // Receive flits from the input channel & place them into the appropriate input fifo
  ocin_flit *flit;
  while (flit = channel_in->receive_flit() ) {

#ifdef DEBUG
    {
      stringstream tmp;
      tmp  << "Receiving flit.. ";
      ocin_name_debug(name,tmp.str());
    }
#endif 
			
    /* If header flit, perform routing */
    if (flit->is_header) {
      _ru->route(flit);
    }

    // Save this fifo's location into flit for deadlock cycle
    // reconstruction
    flit->curr_addr = local_addr;

    /* Push into a VC FIFO */
    flit->arrival_time =ocin_cycle_count; // used to measure latency
                                          // in fifo.
    _input_fifos[flit->vc_idx].enque(flit);
  }
	
  /* Process credit updates */
  ocin_credit *credit;
  while (credit = channel_in->receive_credit() ) {
    // add the token to the current credit count
    int vc_idx = credit->vc;
    ocin_vc_status *v = &_downstream_vc_stats[vc_idx];

    // Increment the credit count for the upstream VC
    v->inc_credits();

    delete credit;
  }
	
	
  /* Process congestion notification messages */
  ocin_cost_msg *cost_msg;
  while (cost_msg = channel_in->receive_cost() ) {

#ifdef DEBUG
    {
      stringstream tmp;
      tmp << "Receiving cong msg on channel " << channel_in->name;
      tmp << ". P_in = " << _port_num;
      tmp << " (msg->pout = " << cost_msg->p_out << ")" << endl;
    }
#endif
    // mark the input port
    cost_msg->p_in = _port_num;
		
    // pass it on to the congestion manager
    _cost_mgr->enque_cost_msg(cost_msg);
  }
}

// repreroute() This method examines the head of each fifo, if the
// flit there is a header and it needs to be re-prerouted to avoid
// deadlock it does so.
void ocin_router_iu::repreroute() {

  for (int x =0; x < _input_fifos.size(); x++) {
    if (!(_input_fifos[x].is_empty())) {
      ocin_flit *flit = _input_fifos[x].front_flit();
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
          flit->msg->rrt_msg = true;
          _ru->route(flit);

          

          // If the input fifo has already gotten a vc we need to put
          // it back on the free vc list
          if (_input_fifos[x].has_vc) {
            parent_router->free_vc_list[_input_fifos[x].p_out].push_back(_input_fifos[x].vc_out_index);
            _input_fifos[x].has_vc = false;
          }

          // Then reset state in input_fifo to reflect newly re-routed
          // packet
          _input_fifos[x].update_fifo_head();
          _input_fifos[x].empty = false;
          _input_fifos[x].avail = false;

        }
      }
    }
  }
}
        
