#include "ocin_gen_bitcomp.h"

void ocin_gen_bitcomp::init (tsim_object *parent, string _name) {

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

  // To speed things up I'm going to pre-calc the bit-comp destination
  // here.

  // First we need to establish the max for each coord
  int dimensions = DIMENSIONS;
  max_coords.assign( dimensions, 0); // set it up with 0's first

  // find the maximum coordinate (mesh size: 0.0 - X.Y)
  for (srcs_it = srcs.begin(); srcs_it != srcs.end(); srcs_it++) {
    for(int i = 0; i <dimensions; i++) {
       string src = terminal2node_map[srcs_it->c_str()];
       int port = terminal2port_map[srcs_it->c_str()];
       int coord = nodes[src].node_info->terminal_coords[port][i];
       
       if (coord > max_coords[i]) {
         max_coords[i] = coord;
       }
    }
  }

  // now look for the matching pair for each source in the dests
  for (srcs_it = srcs.begin(); srcs_it != srcs.end(); srcs_it++) {
  	string src = terminal2node_map[srcs_it->c_str()];
    int port = terminal2port_map[srcs_it->c_str()];
    vector <int> &src_coords = nodes[src].node_info->terminal_coords[port];
  	
    vector <int> bitcomp_coords;
    bitcomp_coords.assign(dimensions, 0);
    for (int i = 0; i<dimensions; i++) {
      bitcomp_coords[i] = abs(max_coords[i] - src_coords[i]);
    }

    // now that we have the bit comp coords we look for a matching dest
    bool found = false;
    for (dsts_it = dsts.begin(); dsts_it != dsts.end(); dsts_it++) {
   	  string dst = terminal2node_map[dsts_it->c_str()];
      int port = terminal2port_map[dsts_it->c_str()];
      vector <int> &dst_coords = nodes[dst].node_info->terminal_coords[port];

      if (bitcomp_coords == dst_coords) {
        src_to_dst[srcs_it->c_str()] = dsts_it->c_str();
        found = true;

#ifdef DEBUG
        {
          stringstream tmp;
          tmp  << "Bit Compliment pair :"<<srcs_it->c_str() << " -> "<< dsts_it->c_str();
          ocin_name_debug(name,tmp.str());
        }
#endif

//cout  << "Bit Compliment pair :"<< srcs_it->c_str() << " (" << src << ")"; 
//cout  << " -> "<< dsts_it->c_str() << " (" << dst << ")" << endl;
      }
    }

    if (found == false) {
  		stringstream tmp;
    	tmp << "Source " <<srcs_it->c_str()
    		<< " does not have a valid bit complement destination!";
    	ocin_name_fatal(name,tmp.str());
		exit(0);
    }

  }
}



// since the generator does not take any inputs from other modules we
// can do all our work in here:
void ocin_gen_bitcomp::update() {
  

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
    ocin_msg * new_msg = gen_packet(srcs[src_idx], src_to_dst[srcs[src_idx]]);

    //    string temp = msg2sptrace(new_msg);
    //    cout << temp;
    
    // push each flit into the iu's vector
    int flit_cnt = new_msg->flits.size();
    for (int y= 0; y < flit_cnt; y++) {
      
      local_iu_ptrs[src_idx]->push_flit(new_msg->flits[y]);
    }


  }


}

void ocin_gen_bitcomp::finalize() {
}
