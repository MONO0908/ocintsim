#include "ocin_gen_bitrev.h"

void ocin_gen_bitrev::init (tsim_object *parent, string _name) {
	
{
  // BitRev is broken	
  stringstream tmp;
  tmp << "BitRev generator is broken! Reasons: \n"
      << " a) it doesn't support terminal nodes (source->dest mapping) \n"
      << " b) it's operating on node numbers (instead of coords, eg: bit-comp, transpose). Double check!!";
  ocin_name_fatal(_name,tmp.str());
  exit(0);
}
  name = _name;
  gen_done = false;

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

  // we assume that the srcs order is the node numbering to reverse.
  for (unsigned x = 0; x <= maxnode_num; x++) {
    unsigned bitrev = revbits(x, maxnode_num);
    
    string rev_name = srcs[bitrev];
    // now that we have the bitrev's name we look for a matching dest
    bool found = false;
    for (dsts_it = dsts.begin(); dsts_it != dsts.end(); dsts_it++) {
      
      if (*dsts_it == rev_name) {
        src_to_dst[srcs[x].c_str()] = dsts_it->c_str();
        found = true;
        
#ifdef DEBUG
        {
          stringstream tmp;
          tmp  << "Bit Reversal pair :"<<srcs[x].c_str() << " -> "<< dsts_it->c_str();
          ocin_name_debug(name,tmp.str());
        }
#endif
        
        
      }
    }
    
    if (found == false) {
      cout << name << ": Warning source " <<srcs[x].c_str() <<
        " does not have a valid bit reversal destination, it will be skipped as a source\n";
    }
  }
}



// since the generator does not take any inputs from other modules we
// can do all our work in here:
void ocin_gen_bitrev::update() {

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

void ocin_gen_bitrev::finalize() {
}


unsigned ocin_gen_bitrev::revbits(unsigned value, unsigned maxval) {
  unsigned maxbit = (unsigned) ((log ((float)maxval))/( log(2.0)));
  unsigned mask = 1 << maxbit;
  unsigned result = 0;
   while ( value ) /* skip most significant bits that are zero */
   {
      if ( value & 1 ) /* replace mod (machine dependency) */
      {
         result |= mask;
      }
      mask  >>= 1;
      value >>= 1;
   }
   return result;
}
