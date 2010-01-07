#include "ocin_prtr_xy.h"

void ocin_prtr_xy::init (tsim_object *parent, string _name) {
  
  name = _name;
  tsim_module::init (parent, name);

  top_p = (ocin_top *) parent;

#ifdef DEBUG
  {
    stringstream tmp;
    tmp  << "Initializing the prerouter ";
    ocin_name_debug(name,tmp.str());
  }
#endif

  
}



// This function takes a msg pointer, clears its "preroute" vector of
// output ports for each hop and then it initializes it with the
// output ports for each hop from the src to the dst.  Assumption is
// that the src is set to the starting point.
void ocin_prtr_xy::preroute_msg(ocin_msg * msg, string src, string dest) {
  
  vector <int> src_coord = node_inst_data[src].coord; // have to pull
                                                      // from here
                                                      // because msg
                                                      // might be
                                                      // re-routed.
  vector <int> dst_coord = msg->dest_addr;  
  int rt_dir, p_out;            // temp vars

  msg->dor = true;

  int dimensions = src_coord.size();

  // For the moment I'm just encoding XY routing...

  for (int dim =0; dim < dimensions; dim++) {
    while (src_coord[dim] != dst_coord[dim]) {
      // First figure out the output port for this hop
      rt_dir = (dst_coord[dim] > src_coord[dim]) ? INC : DEC ; 
      p_out = get_port(dim, rt_dir);

      // push that port onto the preroute vector
      msg->preroutes.push_back(p_out);

      // goto next hop
      if (rt_dir == INC) {
        src_coord[dim]++;
      }else{
        src_coord[dim]--;
      }

#ifdef DEBUG
      {
        stringstream tmp;
        tmp  << "pid #"<<msg->pid<<" prerouting port:"<< p_out;
        ocin_debug(tmp.str());
      }
#endif

    }

    
  }

  // At the end push on the output port.
  msg->preroutes.push_back(get_port(dimensions,0));


}
  





