#include "ocin_gen_randpair.h"

void ocin_gen_randpair::init (tsim_object *parent, string _name) {
  
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

  vector<string>::iterator srcs_it;

  // To speed things up a bit we are creating an order'ed vector of
  // local_iu pointers here and I'll iterate over them instead using
  // the node map directly.

  // iterate over the sources and push pointers onto a list
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

  // Pre-compute a the src->dst map

  // need this to compute the reverse num
  unsigned maxnode_num = srcs.size()-1;
  vector<string> dsts_tmp = dsts; // temporary 


  // we assume that the srcs order is the node numbering to reverse.
  for (unsigned x = 0; x <= maxnode_num; x++) {

    string src = srcs[x];
    string dst;
    unsigned dst_idx;

    unsigned dst_count = dsts_tmp.size();
    if (dst_count > 0) {
      do {
        dst_idx = random()%dst_count; // get a random pair
        
        dst = dsts_tmp[dst_idx];
      } while(src==dst);

      src_to_dst[src.c_str()] = dst.c_str(); // put match in the hash
      
      cout  << "Rand gen pair :"<<src.c_str() << " -> "<< dst.c_str()<<endl;
      
      // remove this dst from the dsts list
      dsts_tmp.erase(dsts_tmp.begin()+dst_idx);
    } else {
      stringstream tmp;
      tmp  << "Ran out of destination nodes on src :" <<srcs[x].c_str();
      ocin_name_warn(name,tmp.str());
      
    }
  }
}




// since the generator does not take any inputs from other modules we
// can do all our work in here:
void ocin_gen_randpair::update() {

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
          
#ifdef DEBUG
    {
      stringstream tmp;
      tmp  << "Injecting packet from node :" <<srcs[src_idx].c_str();
      ocin_name_debug(name,tmp.str());
    }
#endif
    
    // generate packet
    ocin_msg * new_msg = gen_packet(srcs[src_idx], src_to_dst[srcs[src_idx]]);
    
    // push each flit into the iu's vector
    int flit_cnt = new_msg->flits.size();
    for (int y= 0; y < flit_cnt; y++) {
      
      local_iu_ptrs[src_idx]->push_flit(new_msg->flits[y]);
    }
    
  }    
}

void ocin_gen_randpair::finalize() {
}


