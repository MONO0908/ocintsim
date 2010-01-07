#include "ocin_gen.h"


// Generate packet on msg_map, generate flits return pointer to msg.
ocin_msg * ocin_gen::gen_packet(string src_term, string dst_term) {
  string src = terminal2node_map[src_term];
  string dst = terminal2node_map[dst_term];

  ocin_node_info & src_info = node_inst_data[src];
  ocin_node_info & dst_info = node_inst_data[dst];

  ocin_msg * msg_ptr = new ocin_msg;

  // clear some vars to static values
  msg_ptr->vc_out_index = 0;
  msg_ptr->rt_dimension = 0;
  msg_ptr->rt_dir = 0;
  msg_ptr->p_out = 0;
  msg_ptr->hops = 0;
  msg_ptr->reroutes = 0;
  msg_ptr->dor = false;         // allow adaptivity initially
  msg_ptr->rrt_msg = false;

  if (ocin_cycle_count < param_warmup_cycles){
    // make sure we don't count the pre-warmup packets when they get
    // to the ejector
    msg_ptr->pid = temp_pid;    // pre-warmup packets have a negative
                                // pid
    msg_ptr->is_counted = false;
    msg_ptr->post_max = false;
  } else if ((pid >= param_max_packets) && (param_max_packets > 0)) {
    // make sure we don't count the post_max packets when they get to
    // the ejector
    msg_ptr->pid = temp_pid;    // post_max packets have a negative
                                // pid
    msg_ptr->is_counted = false;
    msg_ptr->post_max = true;
  } else {                      // these are counted
    msg_ptr->pid = pid;
    msg_ptr->is_counted = true;
    msg_ptr->post_max = false;
  }


  // setup stats
  msg_ptr->gen_time = ocin_cycle_count;
  //  msg_ptr->inj_time = 0;        // this will be set in router
  //msg_ptr->dst_time = 0;        // this will be set in router

  // set up source and dest coordinates in here
  msg_ptr->src_addr = src_info.coord;
  msg_ptr->dest_addr = dst_info.coord;

  msg_ptr->src = src;
  msg_ptr->dst = dst;
  
  // terminal (local ports) info
  msg_ptr->src_t = src_term;
  msg_ptr->dst_t = dst_term;
  msg_ptr->t_out = terminal2port_map[dst_term];
  
  // compute the quadrant
  msg_ptr->quadrant = get_quadrant(msg_ptr->src_addr, msg_ptr->dest_addr);
  
  // Clear the yFirst flag (used only by O1Turn routing)
  msg_ptr->yFirst = false; 

  // Place it in the msg map if this is the test period
  if(msg_ptr->is_counted) {
    msg_map[pid] = msg_ptr;
  }


  // pick rand number of flits, uniform rand dist from 1 to
  // param_flit_max if param_random_req_size is true, else set to
  // param_flit_max.
  unsigned size;
  // asumption here is that param_flit_max is << 256
  if(param_random_req_size) {
    //    size = (ocin_rand_bits(8) % param_flit_max)+1;  
    unsigned temp = random();

    size = (temp % param_flit_max)+1;  
    //    cout <<"param_random_req_size\n";
  }
  else if (param_bimodal_req_size) {
    unsigned temp = random();

    size = temp % 2;
    size = (size == 0 ?  param_bimodal_size1_flits : param_bimodal_size2_flits);
    //    cout <<"param_bimodal_req_size\n";
  } 
  else {
    size = param_flit_max;
    //    cout <<"flit_max" << param_flit_max <<endl;
  }

  // loop to produce flits and place them in the appropriate structures
  for (int x = 0; x < size;x++) {
    ocin_flit * flit = new ocin_flit;

    // init vars in the flit:
    flit->idx = x;

    // setup stats
    flit->gen_time = ocin_cycle_count;
    flit->inj_time = 0;         // this will be set in router
    //flit->dst_time = 0;         // this will be set in router
    flit->arrival_time = ocin_cycle_count;

    // brg.
    // Injection VC will be picked @ router's inj port
    flit->vc_idx = -1;
    
    flit->p_out = 0;
    flit->p_in = 0;
    
    flit->msg = msg_ptr;
    if (x==0) {
      flit->is_header = true;
    } else {
      flit->is_header = false;
    }

    if (x==(size-1)) {
      flit->is_tail = true;
    } else {
      flit->is_tail = false;
    }

    // give msg a pointer to this packet mon
    msg_ptr->mon = mon;

    // put a pointer to the flit in the message map entry
    msg_ptr->flits.push_back(flit);

    if(msg_ptr->is_counted){
      mon->flit_inj++;
      mon->inj_end = ocin_cycle_count;
      if(param_node_bw_stats == 1) {
        src_info.stats.rtr_inj_flits[dst]++;
      }
    }    

  }
  // finally pre-route the packet if necessary
  if (param_preroute_pkts) {
    prertrs[src]->preroute_msg(msg_ptr, src, dst);   
  }

   
#ifdef DEBUG
  {
    stringstream tmp;
    tmp  << "Created msg: " ;
    tmp  << msg2str(msg_ptr, true /*print flits*/);
    ocin_name_debug(name,tmp.str());
  }

#endif

  if(msg_ptr->is_counted) {
    mon->msg_inj++;
    pid++;                        // increment the next pid counter
  } else {
    temp_pid--;             // count temp_pids backwards
  }

  if ((param_max_packets > 0) && (ocin_cycle_count >= param_warmup_cycles) && 
      (pid >= (param_max_packets-1)) ) {
    gen_done = true;
  }

  // if (msg_ptr->pid <0){
  //   stringstream tmp;
  //   tmp  << "Created uncounted msg: " ;
  //   tmp  << msg2str(msg_ptr, false /*print flits*/);
  //   ocin_name_debug(name,tmp.str());
  // }

 

  return msg_ptr;
}


// building list of legal sources on initilazation.
void ocin_gen::push_src(string src) {
  srcs.push_back(src);
}

// building list of legal dests on initilazation.
void ocin_gen::push_dst(string dst) {
  dsts.push_back(dst);
}


// This produces a random or self similar number.  The argument is
// either a 1 or a 2.  1 means this is for the injection process and 2
// means this is for the src/dst selection process.  1 will either be
// self sim or random based on the value of param_selfsim_inj. 2 will
// always be self sim, using the second values file.
double ocin_gen::gen_rand(int flag) {
  unsigned r_val;

  if(flag ==1) {

    if (param_selfsim_inj == 1) {
      if (!selfsim_file1.is_open()) {
        openfile(1);
      }
      
      selfsim_file1 >> r_val;
      
      // if its at the end close and reopen
      if (selfsim_file1.eof()) {
        selfsim_file1.clear();
        selfsim_file1.close();
        openfile(1);
        selfsim_file1 >> r_val;
        cout << "Reopening the file1\n";
      }
      
    } else {                    // if the inj process is just random
      r_val = random()%10000;
    }

  } else {                      // flag ==2

    if (!selfsim_file2.is_open()) {
      openfile(2);
    }

    selfsim_file2 >> r_val;
    
    // if its at the end close and reopen
    if (selfsim_file2.eof()) {
      selfsim_file2.clear();
      selfsim_file2.close();
      openfile(2);
      selfsim_file2 >> r_val;
      cout << "Reopening the file2\n";
    }
  }
  
  return (double) r_val / (double) 10000; // convert to a double
                                               // between 0 and 1
}

// here we open a self similar values trace file.
void ocin_gen::openfile(int flag) {
  
  if (flag == 1) {
    selfsim_file1.open(param_selfsim_trace1.c_str());
    if(!selfsim_file1)  {
      stringstream tmp;
      tmp << "Error: Unable to open file "<< param_selfsim_trace1.c_str();
      ocin_name_fatal(name,tmp.str());
      exit(1);
    }
    
#ifdef DEBUG
    {
      stringstream tmp;
      tmp  << "Opened  self-similar values file "<< param_selfsim_trace1.c_str() <<endl
        ;
      ocin_name_warn(name,tmp.str());
    }
#endif
  } else {
    
    selfsim_file2.open(param_selfsim_trace2.c_str());
    if(!selfsim_file2)  {
      stringstream tmp;
      tmp << "Error: Unable to open file "<< param_selfsim_trace2.c_str();
      ocin_name_fatal(name,tmp.str());
      exit(1);
    }
    
#ifdef DEBUG
    {
      stringstream tmp;
      tmp  << "Opened  self-similar values file "<< param_selfsim_trace2.c_str() <<endl
        ;
      ocin_name_warn(name,tmp.str());
    }
#endif
  }
}


// here we check to see if we should mark gen_done true because of
// max_packets (simple place to put it that every gen calls)
void ocin_gen::update_gen_done() {

   
}
