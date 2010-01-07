/***********************************
 * 
 ***********************************/

#include "ocin_router_cost_reg.h"

void ocin_router_cost_reg::init(ocin_router *parent, const string &_name, int port_idx) {
  parent_router = parent;
	
  stringstream ss;
  ss << parent->node_info->name.c_str() << "." << _name;
  name = ss.str();
	
  // output port for which the cost is being computed
  p_out = port_idx;
	
  // determine the cost function
  string cost_reg_str = parent_router->node_info->rt_cost_reg;
  if (cost_reg_str == "buff_nohist") {
    cost_reg_fn = REG_BUFF_NOHIST;
  }
  // else if (cost_reg_str == "buff_hist") {
  //   cost_reg_fn = REG_BUFF_HIST;
  // }
  else if (cost_reg_str == "free_vc_nohist") {
    cost_reg_fn = REG_FREE_VC_NOHIST;
  }
  // else if (cost_reg_str == "vc_ages") {
  //   cost_reg_fn = REG_VC_AGES;
  // }
  else if (cost_reg_str == "vc_predictive") {
    cost_reg_fn = REG_VC_PREDICTIVE;
  }
  else if (cost_reg_str == "combined_vc_xb") {
    cost_reg_fn = REG_COMBINED_VC_XB;
  }
  else if (cost_reg_str == "combined_vc_buff") {
    cost_reg_fn = REG_COMBINED_VC_BUFF;
  }
  else if (cost_reg_str == "combined_xb_buff") {
    cost_reg_fn = REG_COMBINED_XB_BUFF;
  }
  else if (cost_reg_str == "combined_vc_xb_buff") {
    cost_reg_fn = REG_COMBINED_VC_XB_BUFF;
  }
  else if (cost_reg_str == "experimental") {
    cost_reg_fn = REG_EXPERIMENTAL;
  }
  else if (cost_reg_str == "vc_stalls") {
    cost_reg_fn = REG_VC_STALLS;
  }
  else if (cost_reg_str == "vc_reqs") {
    cost_reg_fn = REG_VC_REQS;
  }
  else if (cost_reg_str == "vc_demand") {
    cost_reg_fn = REG_VC_DEMAND;
  }
  else if (cost_reg_str == "vc_grants") {
    cost_reg_fn = REG_VC_GRANTS;
  }
  else if (cost_reg_str == "xb_stalls") {
    cost_reg_fn = REG_XB_STALLS;
  }
  else if (cost_reg_str == "xb_reqs") {
    cost_reg_fn = REG_XB_REQS;
  }
  else if (cost_reg_str == "xb_demand") {
    cost_reg_fn = REG_XB_DEMAND;
  }
  else if (cost_reg_str == "xb_grants") {
    cost_reg_fn = REG_XB_GRANTS;
  }
  else if (cost_reg_str == "pkt_delay") {
    cost_reg_fn = REG_PKT_DELAY;
  }
  else {
    stringstream tmp;
    tmp  << "Invalid Cost function: " << cost_reg_str << "\n";     
    ocin_name_fatal(name,tmp.str());
    exit(0); 
  }
	
  // Determine the edge cost function 
  // (used for computing the perceived cost of the edge in aggregating fns)
  if (param_edge_cost_fn == "max") {
    edge_cost_fn = EDGE_MAX;
  }
  else if (param_edge_cost_fn == "min") {
    edge_cost_fn = EDGE_MIN;
  }
  else if (param_edge_cost_fn == "avg") {
    edge_cost_fn = EDGE_AVG;
  }
  else if (param_edge_cost_fn == "local_avg") {
    edge_cost_fn = EDGE_LOCAL_AVG;
  }
  else if (param_edge_cost_fn == "local_fanin") {
    edge_cost_fn = EDGE_LOCAL_FANIN;
  }
  else {
    stringstream tmp;
    tmp  << "Invalid Edge Cost function: " << param_edge_cost_fn << endl;
    tmp  << "Valid options are: max, min, avg, local_avg" << endl;     
    ocin_name_fatal(name,tmp.str());
    exit(0);
  }
	
  // is this an ejection port?
  is_ejector = is_edge = false;
	
  if (p_out == parent_router->node_info->port_count)
    is_ejector = true;
  // is this an unconnected output port?
  if (!is_ejector && (parent_router->output_units[p_out].channel_out == NULL))
    is_edge = true;
		
  // initialize the history fifo
  history =  *(new deque <int>(param_cost_reg_history));
	
  // Clear locals
  hist_sum = 0;
  cost_reg = 0;

  // setup pointers to output channel for the packet_delay monitor
  output_link = parent_router->node_info->output_links[p_out];

	
#ifdef DEBUG	
  {
    stringstream tmp;
    tmp   << "Initialized.  Cost function = " << cost_reg_str;
    ocin_name_debug(name,tmp.str());
  }
#endif

}


/* update()
 *  - update the rolling history & history sum w/
 *    data from selected monitor
 */
void ocin_router_cost_reg::update() {

  //if (is_ejector || is_edge)
  if (is_ejector)
    return;
	
  int new_val;
	
  switch (cost_reg_fn) {
  case REG_BUFF_NOHIST:
    new_val = (int)cost_buff_nohist();
    break;
  // case REG_BUFF_HIST:
  //   new_val = (int)cost_buff_hist();
  //   break;
  case REG_FREE_VC_NOHIST:
    new_val = (int)cost_free_vc_nohist();
    break;
  // case REG_VC_AGES:
  //   new_val = (int)cost_vc_ages();
  //   break;
  case REG_VC_PREDICTIVE:
    new_val = (int)cost_vc_predictive();
    break;
  case REG_COMBINED_VC_XB:
    new_val = (int)cost_combined_vc_xb(); 
    break;
  case REG_COMBINED_VC_BUFF:
    new_val = (int)cost_combined_vc_buff();
    break;
  case REG_COMBINED_XB_BUFF:
    new_val = (int)cost_combined_xb_buff();
    break;
  case REG_COMBINED_VC_XB_BUFF:
    new_val = (int)cost_combined_vc_xb_buff();
    break;
  case REG_EXPERIMENTAL:
    new_val = (int)cost_experimental();
    break;
  case REG_PKT_DELAY:
    new_val = (int)cost_pkt_delay();
    break;
  default:
    // Probably one of the port monitors.
    // If not, it will be caught in cost_pout_mon()
    new_val = cost_pout_mon(cost_reg_fn);
  }

  //  cerr << ocin_cycle_count << ", " << name << ", " << new_val <<endl;
	
  // Shift the value to desired precision
  new_val = new_val << param_cost_precision;

	
  update_history(new_val);
}


/* update_history()
 */
void ocin_router_cost_reg::update_history(int new_val) {
  // Handle updates differently depending on the size of the history fifo.
  // If the history depth is 1, we can get by w/ doing a lot less work.
  if (param_cost_reg_history == 1) {
    if (param_dim_ave == 1) {
      history[0] = (history[0] + new_val)/2;
      hist_sum = history[0];
    } else {

      history[0] = new_val;
      hist_sum = new_val;
    }
  }
  else {
    // grab the oldest value from the fifo, and pop it
    int oldest_val = history.front();
    history.pop_front();

    // push the new (youngest) value at the tail
    history.push_back(new_val);
		
    // update the rolling sum by subtracting the removed val and
    // adding the new one
    hist_sum = hist_sum - oldest_val + new_val;
  }
}


///////////////////////////
// Helpers
///////////////////////////

/* cost_free_vc_nohist() 
 */
int ocin_router_cost_reg::cost_free_vc_nohist() {
  // normally VCs are walked from 0 to last VC.
  // However, VC0 is an escape VC and can be ignored in congestion estimation.
  int first_vc;
  first_vc = param_ignore_escape_vc ? 1 : 0;
					
  // getting rid of this, no siginificant difference
  // edge cost is "special"
  // if (is_edge) {
  //   int vc_count = parent_router->node_info->vc_count;
		
  //   switch (edge_cost_fn) {
  //   case EDGE_MAX:  { return vc_count; };
  //   case EDGE_MIN:  { return 0; };
  //   case EDGE_AVG:  { return (vc_count / 2); };
  //   case EDGE_LOCAL_AVG: 
  //   case EDGE_LOCAL_FANIN: 
  //     {
  //       int dim = get_dim(p_out);
  //       int orth_dim = (dim + 1) % 2;
  //       vector<int> port_vector;
  //       port_vector = (edge_cost_fn == EDGE_LOCAL_AVG) ? 
  //         get_dim_output_ports(orth_dim) : get_all_network_ports(p_out);
		  	
  //       // go through orthogonal output ports, computing their
  //       // respective costs and averaging them at the end.
  //       int free_vcs = 0;
  //       int valid_ports = 0;
			  	
  //       for (int i=0; i<port_vector.size(); i++) {
  //         int p = port_vector[i];
  //         if (parent_router->cost_regs[0][p].is_edge) 
  //           continue;
			  		
  //         valid_ports++;
  //         vector <ocin_vc_status> &pout_vcs = parent_router->downstream_vc_stats[p];

  //         // Don't count the escape VC in the Y direction only?
  //         if (param_ignore_escape_vc_Ydir && isYdir(p)) {
  //           first_vc = 1;
  //         }
					
  //         int free_vcs_temp = 0;
  //         for (int dest_vc=first_vc; dest_vc < pout_vcs.size(); dest_vc++) { 
  //           if (pout_vcs[dest_vc].is_avail()) {
  //             free_vcs_temp++;	
  //           }
  //         }
					
  //         // If *all* VCs are free, do count the escape VC in the Y dir
  //         // This improves load-balance across links
  //         if (param_ignore_escape_vc_Ydir &&             // basic switch
  //             param_ignore_escape_vc_Ydir_heuristic &&   // extra "heuristic" switch
  //             isYdir(p) &&
  //             (free_vcs_temp == pout_vcs.size()-1) &&
  //             pout_vcs[0].is_avail()) {
  //           free_vcs_temp++;
  //           //cout << fcns(name) << "All edge VCs are free. Adding extra VC." << endl;
  //         }
  //         free_vcs += free_vcs_temp;
  //       }
  //       int free_vcs_avg = free_vcs / valid_ports;

  //       // DELETE
  //       //cout << fcns(name) << "Free VCs = " << free_vcs << ", valid ports = " << valid_ports << " ==> avg = " << free_vcs_avg << endl;

  //       return free_vcs_avg;
  //     }
  //   }
  // }
	
  vector <ocin_vc_status> &pout_vcs = parent_router->downstream_vc_stats[p_out];
  int free_vcs = 0;

  // Don't count the escape VC in the Y direction only? 
  // This is a way to better measure "usable" VCs/buffers	
  if (param_ignore_escape_vc_Ydir && isYdir(p_out)) {
    //cout << fcns(name) << "Ignoring Escape VC on port " << p_out << endl;
    first_vc = 1;
  }
	
  for (int dest_vc=first_vc; dest_vc < pout_vcs.size(); dest_vc++) { 
    if (pout_vcs[dest_vc].is_avail()) {
      free_vcs++;	
    }
  }

  // reversing polarity of this so its compatible w/ Dykstra's algo.
  int occupied_vcs =  (pout_vcs.size() - first_vc) - free_vcs; 

  // If *all* VCs are free, do count the escape VC in the Y dir
  // This improves load-balance across links
  if (param_ignore_escape_vc_Ydir &&             // basic switch
      param_ignore_escape_vc_Ydir_heuristic &&   // extra "heuristic" switch
      isYdir(p_out) &&
      (free_vcs == pout_vcs.size()-1) &&
      !pout_vcs[0].is_avail()) {
    occupied_vcs++;
    //    free_vcs++;
    //			cout << fcns(name) << "All VCs are free. Adding extra VC." << endl;
  }

  // DELETE
  //cout << fcns(name) << "Free VCs = " << free_vcs << endl;
	
  return occupied_vcs;
}


/* cost_buff_nohist() 
 */
int ocin_router_cost_reg::cost_buff_nohist() {
  // normally VCs are walked from 0 to last VC.
  // However, VC0 is an escape VC and can be ignored in congestion estimation.
  int first_vc;
  first_vc = param_ignore_escape_vc ? 1 : 0;


  // getting rid of this, no siginificant difference
  // // edge cost is "special"
  // if (is_edge) {
  //   int vc_count = parent_router->node_info->vc_count;
  //   int buffs_per_vc = parent_router->node_info->que_depth;
  //   int buffs_total = vc_count * buffs_per_vc;
		
  //   switch (edge_cost_fn) {
  //   case EDGE_MAX:  { return buffs_total; };
  //   case EDGE_MIN:  { return 0; };
  //   case EDGE_AVG:  { return buffs_total; };
  //   case EDGE_LOCAL_AVG: 
  //   case EDGE_LOCAL_FANIN: 
  //     {
  //       int dim = get_dim(p_out);
  //       int orth_dim = (dim + 1) % 2;
  //       vector<int> port_vector;
  //       port_vector = (edge_cost_fn == EDGE_LOCAL_AVG) ? 
  //         get_dim_output_ports(orth_dim) : get_all_network_ports(p_out);
			  					  	
  //       // go through orthogonal output ports, computing their
  //       // respective costs and averaging them at the end.
  //       int credits = 0;
  //       int valid_ports = 0;
			  	
  //       for (int i=0; i<port_vector.size(); i++) {
  //         int p = port_vector[i];
  //         if (parent_router->cost_regs[0][p].is_edge)
  //           continue;
			  		
  //         valid_ports++;
  //         vector <ocin_vc_status> &pout_vcs = parent_router->downstream_vc_stats[p];
					
  //         // Don't count the escape VC in the Y direction only?
  //         if (param_ignore_escape_vc_Ydir && isYdir(p)) {
  //           first_vc = 1;
  //         }
					
  //         int free_vcs = 0; // just for param_ignore_escape_vc_Ydir_heuristic
  //         for (int dest_vc=first_vc; dest_vc < pout_vcs.size(); dest_vc++) {
  //           credits += pout_vcs[dest_vc].credits;
  //           // just for param_ignore_escape_vc_Ydir_heuristic
  //           if (pout_vcs[dest_vc].is_avail())
  //             free_vcs++;
  //         }
					
  //         // If *all* VCs are free, do count the escape VC in the Y dir
  //         // This improves load-balance across links
  //         if (param_ignore_escape_vc_Ydir &&             // basic switch
  //             param_ignore_escape_vc_Ydir_heuristic &&   // extra "heuristic" switch
  //             isYdir(p) &&
  //             (free_vcs == pout_vcs.size()-1) &&
  //             pout_vcs[0].is_avail()) {
  //           credits += pout_vcs[0].credits;
  //           //cout << fcns(name) << "All edge VCs are free. Counting escape VC's credits" << endl;
  //         }
  //       }
  //       int credits_avg = credits / valid_ports;

  //       // DELETE
  //       //cout << fcns(name) << "Credits = " << credits << ", valid ports = " << valid_ports << " ==> avg = " << credits_avg << endl;

  //       return credits_avg;
  //     }
  //   }
  // }
	
  vector <ocin_vc_status> &pout_vcs = parent_router->downstream_vc_stats[p_out];
  int credits = 0;
  int free_vcs = 0;  // just for param_ignore_escape_vc_Ydir_heuristic
	
  // Don't count the escape VC in the Y direction only?
  // This is a way to better measure "usable" VCs/buffers
  if (param_ignore_escape_vc_Ydir && isYdir(p_out)) {
    first_vc = 1;
  }
		
  // iterate over all VCs @ p_out
  // sum up buffer utilization & capacity. 
  for (int dest_vc=first_vc; dest_vc < pout_vcs.size(); dest_vc++) {
    credits += pout_vcs[dest_vc].credits;
    // just for param_ignore_escape_vc_Ydir_heuristic
    if (pout_vcs[dest_vc].is_avail())
      free_vcs++;
  }
	
  // reversing polarity of this so its compatible w/ Dykstra's algo.
  int occupied_buffers =  (parent_router->node_info->que_depth * (pout_vcs.size() - first_vc)) - credits; 

  // If *all* VCs are free, do count the escape VC in the Y dir
  // This improves load-balance across links
  if (param_ignore_escape_vc_Ydir &&             // basic switch
      param_ignore_escape_vc_Ydir_heuristic &&   // extra "heuristic" switch
      isYdir(p_out) &&
      (free_vcs == pout_vcs.size()-1)) {
    occupied_buffers +=  parent_router->node_info->que_depth - pout_vcs[0].credits;
    //cout << fcns(name) << "All VCs are free. Adding escape VCs credits." << endl;
  }

	
  return occupied_buffers;
}


/* cost_pout_mon()
 *  - Compute the cost based on the value of one of the pout monitors.
 *  - These include # of requests, stalls, demand (reqs+stalls), and grants
 *    for VC and XBar monitors
 */
int ocin_router_cost_reg::cost_pout_mon(Cost_Reg_Function cost_fn) {
  int vc_count = parent_router->node_info->vc_count; 

  // edge cost is "special"
  if (is_edge) {
		
    switch (edge_cost_fn) {
    case EDGE_MAX:  { return 0; };
    case EDGE_MIN:  { return -vc_count; };
    case EDGE_AVG:  { return -((vc_count / 2) - 1); };
    case EDGE_LOCAL_AVG:
    case EDGE_LOCAL_FANIN: 
      {
        int dim = get_dim(p_out);
        int orth_dim = (dim + 1) % 2;
        vector<int> port_vector;
        port_vector = (edge_cost_fn == EDGE_LOCAL_AVG) ? 
          get_dim_output_ports(orth_dim) : get_all_network_ports(p_out);

        // go through orthogonal output ports, computing their
        // respective costs and averaging them at the end.
        int cost = 0;
        int valid_ports = 0;
			  	
        for (int i=0; i<port_vector.size(); i++) {
          int p = port_vector[i];
          if (parent_router->cost_regs[0][p].is_edge)
            continue;
			  		
          valid_ports++;
          vector <ocin_mon_port> & pout_mons = parent_router->pout_mon[p]; 
			  		
          for (int pmon=0; pmon < pout_mons.size(); pmon++) {
            cost -= pout_mons[pmon].xmon.global_demand(false);
          }
			  		
        }
        int cost_avg = cost / valid_ports;

        // DELETE
        //cout << fcns(name) << "Cost = " << cost << ", valid ports = " << valid_ports << " ==> avg = " << cost_avg << endl;

        return cost_avg;
      }
    }
  }
	
  vector <ocin_mon_port> & pout_mons = parent_router->pout_mon[p_out]; 
  int cost = 0;
		
  for (int i=0; i < pout_mons.size(); i++) {
    switch (cost_fn) {
    case REG_VC_STALLS:
      cost += pout_mons[i].vmon.get_curr_stalls();
      break;
    case REG_VC_REQS:
      cost += pout_mons[i].vmon.get_curr_global_reqs();
      break;
    case REG_VC_DEMAND:
      cost += pout_mons[i].vmon.global_demand(false);
      break;
    case REG_VC_GRANTS:
      cost += pout_mons[i].vmon.get_curr_global_gnts();
      break;
    case REG_XB_STALLS:
      cost += pout_mons[i].xmon.get_curr_stalls();
      break;
    case REG_XB_REQS:
      cost += pout_mons[i].xmon.get_curr_global_reqs();
      break;
    case REG_XB_DEMAND:
      cost += pout_mons[i].xmon.global_demand(false);
      //cost -= (pout_mons[i].xmon.get_curr_stalls() + pout_mons[i].xmon.get_curr_global_reqs()); 
      break;
    case REG_XB_GRANTS:
      cost += pout_mons[i].xmon.get_curr_global_gnts();
      break;
    default:
      stringstream tmp;
      tmp  << "Invalid enum value for the Cost function: " << cost_fn << "\n";     
      ocin_name_fatal(name,tmp.str());
      exit(0);
    }	
  }
  //  cout << "returning cost of :" <<cost<<endl;
	
  return cost;
}


/* cost_combined_vc_xb()
 */
int ocin_router_cost_reg::cost_combined_vc_xb() {
  int free_vc_count = cost_free_vc_nohist();
  int xb_demand = cost_pout_mon(REG_XB_DEMAND);

  int combined_cost = free_vc_count + xb_demand;

  // DELETE
  //cout << fcns(name) << "VC-XB:  free_vc = " << free_vc_count << ", xb = " << xb_demand << ". Total = " << combined_cost << endl;

  return combined_cost;
}
 

/* cost_combined_vc_buff()
 */
int ocin_router_cost_reg::cost_combined_vc_buff() {
  int buffs_per_vc = parent_router->node_info->que_depth;
	
  int free_vc_count = cost_free_vc_nohist();
  int free_buff_count = cost_buff_nohist() / buffs_per_vc;

  int combined_cost = free_vc_count + free_buff_count;	
	
  return combined_cost;
}


/* cost_combined_xb_buff()
 */ 
int ocin_router_cost_reg::cost_combined_xb_buff() {
  int buffs_per_vc = parent_router->node_info->que_depth;
	
  int free_buff_count = cost_buff_nohist() / buffs_per_vc;
  int xb_demand = cost_pout_mon(REG_XB_DEMAND);
	
  int combined_cost = free_buff_count + xb_demand;

  return combined_cost;
}

/* cost_combined_vc_xb_buff()
 */ 
int ocin_router_cost_reg::cost_combined_vc_xb_buff() {
  int free_vc_count = cost_free_vc_nohist();

  int buffs_per_vc = parent_router->node_info->que_depth;
	
  int free_buff_count = cost_buff_nohist() / buffs_per_vc;
  int xb_demand = cost_pout_mon(REG_XB_DEMAND);
	
  int combined_cost = free_vc_count + free_buff_count + xb_demand;

  return combined_cost;
}
	

/* cost_experimental()
 */
int ocin_router_cost_reg::cost_experimental() {
  int exp_cost = 0;
	
}


// This reg keeps a diminishing average of the pkt delay for a given
// link
int ocin_router_cost_reg::cost_pkt_delay() {
  
  if (output_link != NULL) {

    int delay_sum = 0;

    // go through each vc fifo looking for flits headed for this
    // output port and sum up thier latency
    vector <vector <ocin_router_vc_fifo> > &vc_fifos = parent_router->vc_fifos;

    vector <vector <ocin_router_vc_fifo> >::iterator vc_fifos_i;
    vector <ocin_router_vc_fifo>::iterator vc_fifos_k;
  
    // iterate over all input vc_fifos (inc. injection port)
    for (vc_fifos_i=vc_fifos.begin(); vc_fifos_i != vc_fifos.end(); vc_fifos_i++) {
      // iterate over classes (vc_fifos_j) here	
      for (vc_fifos_k=vc_fifos_i->begin(); vc_fifos_k != vc_fifos_i->end(); vc_fifos_k++) {
        if (!(vc_fifos_k->is_empty())) {
          ocin_flit * flit = vc_fifos_k->front_flit();
          // Only if this output port is one of the the ports in the
          // flit's routes map
          if (flit->msg->routes.find(p_out) != flit->msg->routes.end()) { 
            // add to delay sum
            delay_sum += ocin_cycle_count - flit->arrival_time;
          }
        }
      }
    }   

    return delay_sum;
  } else {
    return 0;
  }

}

//Removing old/broken cost reg fns

// /* cost_buff_hist() 
//  */
// int ocin_router_cost_reg::cost_buff_hist() {
//   int buffers_free = 0;
//   vector <ocin_router_vc_fifo> &vc_fifos = 
//     parent_router->node_info->next_hop_input_ports[p_out]->_input_fifos;
		
//   // iterate over all VCs @ p_out
//   // sum up buffer utilization & capacity. 
//   for (int i=0; i < vc_fifos.size(); i++) {
//     buffers_free += vc_fifos[i].mon->get_chpt_buffers_free();
//   }

//   return buffers_free;
// }


// /* cost_vc_ages()
//  *  - cost is a function of the number of cycles since last deque from a VC.
//  *  - recent deque -> higher cost (higher port throughput is favorable)
//  */ 
// int ocin_router_cost_reg::cost_vc_ages() {
//   int vc_ages = 0;
//   vector <ocin_router_vc_fifo> &vc_fifos =
//     parent_router->node_info->next_hop_input_ports[p_out]->_input_fifos;
			
//   // iterate over all VCs @ p_out
//   // sum up deque stats
//   for (int i=0; i < vc_fifos.size(); i++) {
//     vc_ages -= vc_fifos[i].mon->get_curr_flit_stall_cycles();
//   }
	
//   return vc_ages; 
// }


/* cost_vc_predictive()
 */
int ocin_router_cost_reg::cost_vc_predictive() {
  int predictive_cost = 0;
  vector <ocin_vc_status> &pout_vcs = parent_router->downstream_vc_stats[p_out];
	
  // iterate over all VCs @ p_out
  // sum up buffer utilization & capacity. 
  for (int i=0; i < pout_vcs.size(); i++) {
    predictive_cost += pout_vcs[i].cost;
  }	
  return predictive_cost;
}

