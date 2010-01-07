#include "ocin_gen_modeled.h"

void ocin_gen_modeled::init (tsim_object *parent, string _name) {

  // setup the gsl random number stuff
  gsl_rng_env_setup();

  T = gsl_rng_default;
  r = gsl_rng_alloc (T);


  
  gen_done = false;

  name = _name;

  tsim_module::init (parent, name);
  
  throttling = false;

  top_p = (ocin_top *) parent;
  
  // setup the packet monitor
  mon = new ocin_mon_packet;
  mon->init(name, nodes.size());

#ifdef DEBUG
  {
    stringstream tmp;
    tmp  << "Initializing Packet Generator.";
    ocin_name_debug(name,tmp.str());
  }
#endif


  // To speed things up a bit we are creating an order'ed vector of
  // local_iu pointers here and I'll iterate over them instead using
  // the node map directly.

  // iterate over the sources and push pointers onto a list
  vector<string>::iterator srcs_it;
  for (srcs_it = srcs.begin(); srcs_it != srcs.end(); srcs_it++) {
    string src = terminal2node_map[srcs_it->c_str()];
  	int port = terminal2port_map[srcs_it->c_str()];
    local_iu_ptrs.push_back(&(nodes[src].local_iu[port]));
    
#ifdef DEBUG
    {
      stringstream tmp;
      tmp  << "Packet Gen source:"<<srcs_it->c_str();
      ocin_name_debug(name,tmp.str());
    }
#endif
  }

#ifdef DEBUG
    {
      for (srcs_it = dsts.begin(); srcs_it != dsts.end(); srcs_it++) {
        stringstream tmp;
        tmp  << "Packet Gen ejector:"<<srcs_it->c_str();
        ocin_name_debug(name,tmp.str());
      }
    }
#endif

    set_probabilities();

}

void ocin_gen_modeled::set_probabilities() {
  vector<string>::iterator srcs_it;
  vector<string>::iterator dsts_it;

  double src_cum=0.0;
  double adj_inj_bw = -1.0;
  int count = 0;
  double ave_rate = 0.0;

  dst_cum_tot = 0.0;
  do {
    if (adj_inj_bw > 0.0) {
      adj_inj_bw = adj_inj_bw * .95; // lower it a bit
      cout << "Lowering to : "<< adj_inj_bw<<endl;
    } else {
      adj_inj_bw = param_adj_inj_bw;
    }

    src_cum = 0.0;
    count = 0;
    for (srcs_it = srcs.begin(); srcs_it != srcs.end(); srcs_it++) {
      double value = (gsl_ran_gaussian (r, (param_src_sigma * adj_inj_bw))) + adj_inj_bw;
      
      src_rate[*srcs_it] = value;
      stringstream tmp;
      tmp  << "Setting src rate for "<<*srcs_it<< " to "<< value;
      ocin_name_warn(name,tmp.str());


      count++;
      if (src_rate[*srcs_it] < 0.0) {
        tmp.str("");
        tmp << "Rate to low, zeroing";
        ocin_name_warn(name,tmp.str());
        src_rate[*srcs_it] = 0.0;
      } else {
        src_cum += value;
      }
      
    }
    ave_rate = src_cum/((double)count);
    cout << "Average rate: " <<ave_rate << " expected "<<param_adj_inj_bw<<endl;
  } while (ave_rate > param_adj_inj_bw);

  for (dsts_it = dsts.begin(); dsts_it != dsts.end(); dsts_it++) {
    double k = gsl_ran_gaussian (r,(param_dst_sigma * param_adj_inj_bw) );
    dst_cum_tot  += gsl_ran_gaussian_pdf (k, (param_dst_sigma * param_adj_inj_bw));
    dst_rate[*dsts_it] = dst_cum_tot;

    stringstream tmp;
    tmp<< "Setting dst rate for "<<*dsts_it<< " to "<< dst_rate[*dsts_it]<<endl;
    ocin_name_warn(name,tmp.str());

  }

    
}

// since the generator does not take any inputs from other modules we
// can do all our work in here:
void ocin_gen_modeled::update() {

  unsigned dst_count = dsts.size();
  unsigned src_count = srcs.size();


  unsigned dst_idx, src_idx;

  for (src_idx=0; src_idx < src_count; src_idx++) {

    if (msg_map.size() > param_pkt_throttle){// don't produce so many packets the
      // simulator blows up
      if (throttling == false) {
        stringstream tmp;
        tmp  << "Hit max simultaneous packet limit, started throttling...";
        ocin_name_warn(name,tmp.str());
        throttling = true;
      }
      continue;
    }

    // Roll the dice. 

    double dice = gen_rand(1);
    if (dice > src_rate[srcs[src_idx]])
      continue;


      
#ifdef DEBUG
    {
      stringstream tmp;
      tmp  << "Injecting packet from node :" <<srcs[src_idx].c_str();
      ocin_name_debug(name,tmp.str());
    }
#endif

    // generate packet
    do {
      double randvar = (gsl_rng_uniform(r) * dst_cum_tot); // scaled to size of dist
      vector<string>::iterator dsts_it;
      dsts_it = dsts.begin();                   
      dst_idx = 0;
      while (dst_rate[*dsts_it] < randvar) {
        dsts_it++;
        dst_idx++;
      }

    } while (dsts[dst_idx] == srcs[src_idx]);
    
    //    cout << ocin_cycle_count << " " << srcs[src_idx] << ":" << dsts[dst_idx] <<endl;
    ocin_msg * new_msg = gen_packet(srcs[src_idx],dsts[dst_idx]);

    // push each flit into the iu's vector
    int flit_cnt = new_msg->flits.size();
    for (int y= 0; y < flit_cnt; y++) {
        
      local_iu_ptrs[src_idx]->push_flit(new_msg->flits[y]);
    }

  }    
}

void ocin_gen_modeled::finalize() {
}

