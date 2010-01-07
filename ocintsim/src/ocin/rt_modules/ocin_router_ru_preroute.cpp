#include "ocin_router_ru_preroute.h"


void ocin_router_ru_preroute::init(ocin_router *parent, const string &_name) {
  // make a "local" copy of this node's coordinates to avoid an expensive
  // lookup for each packet.
  _coord = parent->node_info->coord;
	
  // call base init() w/ fully specified name
  stringstream *ss = new stringstream();
  *ss << parent->node_info->name.c_str() << "." << _name;
  ocin_router_ru::init(parent, ss->str());
  delete ss;
	
  // initialize the port count
  port_count = parent->node_info->port_count;
  node_name = parent->node_info->name;


}


/* route() - This routing function simply peels off an output port
 *  from the preroute vector.  Only one output port is placed in
 *  routes.  Removed all the dor detection stuff because I'm now using
 *  deadlock recovery via deflection.
 */
void ocin_router_ru_preroute::route (ocin_flit *flit) {
  ocin_msg *msg = flit->msg;
  int num_dimensions = _coord.size();
  int rt_dir; 
  int p_out;
  bool done = false;
  map <int, set <int> > routes;   // key = p_out, value = set of valid VC's @ this p_out

  routes.clear();
  msg->vc_out_index =  -1;      // signifies that we don't have a vc yet


  // if we have already re-routed this packet too many times drop to
  // DOR and use channel 0.
  if(msg->reroutes > param_reroute_limit) {

    msg->dor = true;            // just making sure...

    // 1. figure out the dimension for next hop
    int curr_dim = 0;
    while (msg->dest_addr[curr_dim] == _coord[curr_dim]) {
      curr_dim++;
      // have we traversed all dimensions?
      if (curr_dim == num_dimensions) {
        done = true;
        break;
      }
    }
	
    // within the dimension, are we traveling in increasing or decreasing direction?
    if (done) {
      // dest: local port
      rt_dir = 0;
    }
    else {
      rt_dir = (msg->dest_addr[curr_dim] > _coord[curr_dim]) ? INC : DEC ; 
    }
	
    // 2. map the dimension to an output port
    // Note: the mapping assumes that local port is the LAST port.
    p_out = get_port(curr_dim, rt_dir);
    // 3. Use VC 0 for these DOR routed packets
    routes[p_out].insert(0);

  } else {                      // if we are not over a certain number
                                // of re-routes then we can use
                                // adaptive re-routing.

    // 1. Grab route from preroutes vector
    
    // if  we are out of routes  and we never hit  the output port
    // then re-prerouting is necessary...
    if ((msg->preroutes.empty())||(msg->rrt_msg ==true)) {
#ifdef DEBUG
      {
        stringstream tmp;
        tmp  << "Reprerouting pid: "<< msg->pid <<" because it's list is empty";    
    ocin_name_debug(name,tmp.str());
      }
#endif

      if (msg->rrt_msg ==true) {
        msg->rrt_msg = false;   // we are here because we had a
                                // latency in getting out of the input
                                // fifo.
        msg->preroutes.clear();
      } else {
        msg->add_rrt_delay = true; // if we are rerouting due to lack
                                   // of preroutes we want to delay
                                   // vcallocation
      }
      
      parent_router->prtr_ptr->preroute_msg(msg, node_name, msg->dst);
      
    }
    
    p_out =  msg->preroutes.front();
    
    msg->preroutes.pop_front();
    
    // 2. produce a set of valid VCs at the output port
    
    if (p_out <= port_count  /*OUs + eject port*/) {
      vector <ocin_vc_status> *vc_info = &(parent_router->downstream_vc_stats[p_out]);
      // Iterate over all VCs at this output port except 0 which is
      // reserved as an escape channel.
      for (int dest_vc=1; dest_vc < vc_info->size(); dest_vc++) {
        routes[p_out].insert(dest_vc);
      }
    } else {
      {
        stringstream tmp;
        tmp  << "Invalid out port requested!! \n"      
             << "\t Last valid port index (ejector) = " << port_count
             << "\t Requested port = " << p_out << endl;
        ocin_name_fatal(name,tmp.str());
      }
      exit(0);
    }
  }
  msg->routes = routes;
  
#ifdef DEBUG
  {
    int src_vc = flit->vc_idx;
    int p_out = (*msg->routes.begin()).first;
    stringstream tmp;
    tmp << "PID: " <<msg->pid 
        << ", Curr D: " << msg->rt_dimension
        << ", new D: " << get_dim(p_out)
        << ". Curr dir: " << msg->rt_dir
        << ", new dir: " << get_dir(p_out)
        << ". P_out: " << p_out
        << ". Routes size: " << msg->routes.size() 
        << ". preroutes size: " << msg->preroutes.size() << endl;
    tmp << "\t Src: " << vector2str(&msg->src_addr)
        <<  ", Dest: " << vector2str(&msg->dest_addr)
        <<  ", This: " << vector2str(&_coord)
        <<  ", src VC: " << src_vc << endl;
    set <int> *vcs = &msg->routes[p_out];
    tmp << "\t pout " << p_out << ": " << set2str(vcs) << endl;
    
    ocin_name_debug(name,tmp.str());
  }
  
  {
    stringstream tmp;
    tmp  << "Packet P_out: " << p_out;
    ocin_name_debug(name,tmp.str());
  }
  
  //cout << msg2str(msg, false /*print flits*/);
#endif
  

}
