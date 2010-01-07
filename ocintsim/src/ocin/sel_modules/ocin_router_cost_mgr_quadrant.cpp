/* 
 */

#include "ocin_router_cost_mgr_quadrant.h"

void ocin_router_cost_mgr_quadrant::init(ocin_router *parent, const string &_name) {
  // call base init() w/ fully specified name
  stringstream *ss = new stringstream();
  *ss << parent->node_info->name.c_str() << "." << _name;
  ocin_router_cost_mgr::init(parent, ss->str());
  delete ss;

  _port_count = parent_router->node_info->port_count;
  _quadrants = _cost_regs.size();
	
  if (_quadrants < 4) {
    stringstream tmp;
    tmp  << "Bad number of quadrants. Check if the correct rt_cost_fn is specified." << endl;
    tmp  << "Currently, only 'rt_cost_fn = quadrants' is supported. " << endl; 
    ocin_name_fatal(name,tmp.str());
    exit(0); 
  }

  // msg queue size should match the number of quadrants
  _cost_msg_queue = *(new vector <map <int, ocin_cost_msg *> >(_quadrants));
	
  // sum of weights used in computing weighted cost
  _sum_of_weights = param_cost_multiplier_local + param_cost_multiplier_remote;
	
#ifdef DEBUG	
  {
    stringstream tmp;
    tmp   << "Initialized. Cost manager = quadrant";
    ocin_name_debug(name,tmp.str());
  }
#endif
}


/* update_cost()
 *  - simple updater, uses the rolling history sum as the cost 
 */
void ocin_router_cost_mgr_quadrant::update_cost() {
  // for each quadrant
  for (int q=0; q < _quadrants; q++) {
    // Integrate the received congestion updates w/ local p_out costs.
    // Note that congestion updates flow in the opposite dir of actual
    // msgs; thus, they are received on quadrant "output" ports and are 
    // sent out via "input" ports
    vector <int> ports = get_quadrant_output_ports(static_cast<Quadrant>(q));

    vector <int> ::iterator pitr;
    for (pitr = ports.begin(); pitr!=ports.end(); pitr++) {
      int p = *pitr;
      ocin_cost_msg *msg;
      int new_reg_cost;
      // get the most recent available monitor value
      int reg_cost = _cost_regs[q][p].get_hist_sum(); 
			
      // check if a congestion update msg has been received on this port
      if(_cost_msg_queue[q].find(p) != _cost_msg_queue[q].end()) {
        msg = _cost_msg_queue[q][p];   // get the msg

#ifdef DEBUG
        {
          stringstream tmp;
          tmp << "Processing msg # " << msg->msg_id 
              << " (p_in " << msg->p_in << ", quadrant " << quadrant2str(msg->quadrant) << ")";
          ocin_name_debug(name,tmp.str());
        }
#endif
        // combine local & non-local costs using user-specified weights
        int local_cost = reg_cost * param_cost_multiplier_local;
        int remote_cost = msg->get_cost() * param_cost_multiplier_remote;
        new_reg_cost = (local_cost + remote_cost) / _sum_of_weights;

        // delete the msg
        delete msg;
      }
      else {
        new_reg_cost = reg_cost;
      }
      // update the cost reg at the given output port for this quadrant
      _cost_regs[q][p].set_cost(new_reg_cost);
				
      // DELETE
      //cout <<  "Updated cost for quadrant " << quadrant2str(static_cast<Quadrant>(q)) << " @ p_out " << p << " = " << new_reg_cost << endl;
			
#ifdef DEBUG
      {
        stringstream tmp;
        tmp   <<  "Updated cost for quadrant " << quadrant2str(static_cast<Quadrant>(q)) << " @ p_out " << p << " = " << new_reg_cost;
        ocin_name_debug(name,tmp.str());
      }
#endif
    } // for all output ports
		
		
    // Generate congestion updates for this quadrant.
    // First, this requires computing the cost for this quadrant at this node.
    // Currently, 2 modes of computing the cost for the quadrant are supported:
    // 1. Max cost (choose the p_out w/ highest cost and use that)
    // 2. Avg cost (avg the costs across all of quadrant's p_outs)
    int new_quad_cost = param_use_max_quadrant_cost ? -100000 : 0;  
		
    for (pitr = ports.begin(); pitr!=ports.end(); pitr++) {
      int p = *pitr;
      if (param_use_max_quadrant_cost) {
        if (_cost_regs[q][p].cost_reg > new_quad_cost) {
          new_quad_cost = _cost_regs[q][p].cost_reg; 
        }
      }
      else {
        new_quad_cost += _cost_regs[q][p].cost_reg;
      }
    } // for all input ports

    // compute the avg cost for the quadrant if not using the max cost
    if (!param_use_max_quadrant_cost) {
      new_quad_cost = new_quad_cost / (int)ports.size();
    } 

    // generate a new cost msg & send a copy of it on each one of
    // the quadrant's ports.
    // Again, as congestion updates flow opposite of actual packets,
    // they will be sent out via "input" ports for the quadrant		
    ports = get_quadrant_input_ports(static_cast<Quadrant>(q));
		
    for (pitr = ports.begin(); pitr!=ports.end(); pitr++) {
      int p = *pitr;
      ocin_cost_msg *msg = new ocin_cost_msg(new_quad_cost);
      msg->p_out = p;
      msg->quadrant = static_cast<Quadrant>(q);
      parent_router->output_units[p].enque(msg);
                			
#ifdef DEBUG
      {
        stringstream tmp;
        tmp   <<  "Generated new msg # " << msg->msg_id
              << " (p_out " << msg->p_out << ", quadrant " << quadrant2str(msg->quadrant) << ")"
              << " with cost = " << new_quad_cost << endl; 
        ocin_name_debug(name,tmp.str());
      }
#endif
    }
		
    // clear the msg_queue for this quadrant
    _cost_msg_queue[q].clear();
		
  } // for all quadrants
}


/* enque_cost_msg()
 *  - enque the received cost message for processing by the cost manager
 *    during the update phase
 */
void ocin_router_cost_mgr_quadrant::enque_cost_msg(ocin_cost_msg *cost_msg) {
  int q = cost_msg->quadrant;
  int p_in = cost_msg->p_in;
  _cost_msg_queue[q][p_in] = cost_msg;
}
