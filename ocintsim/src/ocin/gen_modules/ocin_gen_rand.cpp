#include "ocin_gen_rand.h"

void ocin_gen_rand::init (tsim_object *parent, string _name) {

  
  gen_done = false;

  name = _name;

  throttling = false;

  tsim_module::init (parent, name);
  
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

}



// since the generator does not take any inputs from other modules we
// can do all our work in here:
void ocin_gen_rand::update() {
  
  
  unsigned dst_count = dsts.size();
  unsigned src_count = srcs.size();
  unsigned dst_idx, src_idx;
/*  
  // First figure out how many packets we will generate
  double selfrand1 = gen_rand(1);
  unsigned packet_count = (unsigned)((selfrand1*param_adj_inj_bw*(double)src_count*2.0) +.5);
  
  // Iterate over each packet and create it
  for (int x = 0; x <packet_count;x++) {
    src_idx = random()%src_count;
*/

  for (src_idx=0; src_idx < src_count; src_idx++) {


    // Roll the dice. 
    double dice = gen_rand(1);
    if (dice > param_adj_inj_bw)
      continue;

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
      
#ifdef DEBUG
    {
      stringstream tmp;
      tmp  << "Injecting packet from node :" <<srcs[src_idx].c_str();
      ocin_name_debug(name,tmp.str());
    }
#endif

    // generate packet
    do {

      dst_idx = random() % dst_count;

    } while (dsts[dst_idx] == srcs[src_idx]);
    
    //cout << ocin_cycle_count << " " << srcs[src_idx] << ":" << dsts[dst_idx] <<endl;
    ocin_msg * new_msg = gen_packet(srcs[src_idx],dsts[dst_idx]);  
    // string temp = msg2sptrace(new_msg);
    // cout << temp;
    
    // push each flit into the iu's vector
    int flit_cnt = new_msg->flits.size();
    for (int y= 0; y < flit_cnt; y++) {
      
      local_iu_ptrs[src_idx]->push_flit(new_msg->flits[y]);
    }
    
  }    
  
}

void ocin_gen_rand::finalize() {
}
