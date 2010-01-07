#include "ocin_gen_hotspot.h"

void ocin_gen_hotspot::init (tsim_object *parent, string _name) {

  //cout << "Hotspot generator not configured for the new (per-node) injection process - Sp '08." << endl;
  //cout << "Configure (see gen_rand or any other gen for an example) and " << endl;
  //cout << "make sure the baseline code was correct to start with " << endl;
  cout << "NOTE: Hotspot is not configured for concentration!" << endl;
  //exit(1);
  
  gen_done = false;

  name = _name;

  tsim_module::init (parent, name);
  
  top_p = (ocin_top *) parent;

  // setup the packet monitor
  mon = new ocin_mon_packet;
  mon->init(name, nodes.size());

#ifdef DEBUG
  {
    stringstream tmp;
    tmp  << "Initializing Packet Generator";
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

  vector<string>::iterator dsts_it;
#ifdef DEBUG
    {

      for (dsts_it = dsts.begin(); dsts_it != dsts.end(); dsts_it++) {
        stringstream tmp;
        tmp  << "Packet Gen ejector:"<<dsts_it->c_str();
        ocin_name_debug(name,tmp.str());
      }
    }
#endif

  // now we create the list of four nodes for hotspot traffic. May
  // later want to make the number of hotspot nodes different.

  int dst_size = dsts.size();
  int hs_idx;
  string hs_str;
  for(int x=0; x < 4; x++) {
    bool match;     // used to make sure we don't mark
    // same node as hotspot twice
    do {
      match = false;
      unsigned temp = random();

      hs_idx = temp % dst_size;
      hs_str = dsts[hs_idx];

      for(int y = 0; y < hs_names.size(); y++) {
        if (hs_str == hs_names[y]) {
          match = true;
        }
      }
    } while (match == true);
    hs_names.push_back(hs_str);
#ifdef DEBUG
    {
      stringstream tmp;
      tmp  << "Hot spot node: "<<hs_str.c_str();
      ocin_name_debug(name,tmp.str());
    }
#endif

    cout << "Hot spot node: "<<hs_str.c_str()<<endl;
  }
  
}



// since the generator does not take any inputs from other modules we
// can do all our work in here:
void ocin_gen_hotspot::update() {

  // now do the hotspot traffic (background traffic is done in a
  // different generator)

  // in hotspot first we pick the dest

  unsigned dst_count = hs_names.size();
  unsigned src_count = srcs.size();
  unsigned dst_idx, src_idx;

  // old way of generation...
  // // Figure out how many packets we will generate (4x the frequency of
  // // the background traffic)
  // double selfrand1 = gen_rand(1);
  // unsigned packet_count = (unsigned)((4*selfrand1*param_adj_inj_bw*(double)dst_count*2.0) +.5);

  

  // // Iterate over each packet and create it
  // for (int x = 0; x <packet_count;x++) {

  for (src_idx=0; src_idx < src_count; src_idx++) {

    
    // Roll the dice. 
    double dice = gen_rand(1);
    if (dice > param_adj_inj_bw)
      continue;

#ifdef DEBUG
    {
      stringstream tmp;
      tmp  << "Injecting packet from node :" <<srcs[src_idx].c_str();
      ocin_name_debug(name,tmp.str());
    }
#endif
    do {

      dst_idx = random() % dst_count;

    } while (hs_names[dst_idx] == srcs[src_idx]);

    
    
    ocin_msg * new_msg = gen_packet(srcs[src_idx],hs_names[dst_idx]);
    
    // push each flit into the iu's vector
    int flit_cnt = new_msg->flits.size();
    for (int y= 0; y < flit_cnt; y++) {
      
      local_iu_ptrs[src_idx]->push_flit(new_msg->flits[y]);
    }
  }
}



void ocin_gen_hotspot::finalize() {
}
