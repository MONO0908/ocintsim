#include "ocin_gen_transpose.h"

void ocin_gen_transpose::init (tsim_object *parent, string _name) {
  
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
    tmp  << "Initializing Packet Generator";
    ocin_name_debug(name,tmp.str());
  }
#endif

  vector<string>::iterator srcs_it;
  srcs_it = srcs.begin();
  
  // Currently, only 2-D topologies are supported
  string src = terminal2node_map[srcs_it->c_str()];
  int dimensions = nodes[src].node_info->coord.size();
  if (dimensions != 2) {
  	  stringstream tmp;
	  tmp << "Transpose traffic pattern is only defined for 2-D topologies!" << endl;
	  ocin_name_fatal(name,tmp.str());
  	  exit(0);
  }


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

  // Pre-compute a src-dest map

  // For each src <c0, c1>, dest = <c1, c0>
  srcs_it = srcs.begin();
  // look for the matching pair for each source in the dests
  for (srcs_it = srcs.begin(); srcs_it != srcs.end(); srcs_it++) {
  	string src = terminal2node_map[srcs_it->c_str()];
    int port = terminal2port_map[srcs_it->c_str()];
    vector <int> &src_coords = nodes[src].node_info->terminal_coords[port];

    vector <int> transpose_coords;
	vector <int>::reverse_iterator coord_rit;
	for (coord_rit = src_coords.rbegin(); coord_rit != src_coords.rend(); coord_rit++) {
		transpose_coords.push_back(*coord_rit);
	}

    // now that we have the bit comp coords we look for a matching
    // dest
    bool found = false;
    for (dsts_it = dsts.begin(); dsts_it != dsts.end(); dsts_it++) {
	  string dst = terminal2node_map[dsts_it->c_str()];
      int port = terminal2port_map[dsts_it->c_str()];
      vector <int> &dst_coords = nodes[dst].node_info->terminal_coords[port];

      if (transpose_coords == dst_coords) {
        src_to_dst[srcs_it->c_str()] = dsts_it->c_str();
        found = true;

#ifdef DEBUG
        {
          stringstream tmp;
          tmp  << "Transpose pair :"<<srcs_it->c_str() << " -> "<< dsts_it->c_str();
          ocin_name_debug(name,tmp.str());
        }
#endif

//cout  << "Transpose pair :"<<srcs_it->c_str() << " -> "<< dsts_it->c_str() << endl;
      }
    }

    if (found == false) {
      cout << name << ": Warning source " <<srcs_it->c_str() << 
        " does not have a valid bit complement destination, it will be skipped as a source\n";
    }

  }
}



// since the generator does not take any inputs from other modules we
// can do all our work in here:
void ocin_gen_transpose::update() {


  unsigned src_count = srcs.size();
  unsigned src_idx;
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
    ocin_msg * new_msg = gen_packet(srcs[src_idx], src_to_dst[srcs[src_idx]]);
    
    // push each flit into the iu's vector
    int flit_cnt = new_msg->flits.size();
    for (int y= 0; y < flit_cnt; y++) {
      
      local_iu_ptrs[src_idx]->push_flit(new_msg->flits[y]);
    }
    
  }    

}

void ocin_gen_transpose::finalize() {
}
