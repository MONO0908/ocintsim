/* Aggregates congestion info along a given direction.
 * Thus, a single congestion status msg might travel North
 * reflecting the status of South output ports along the
 * given column.
 * At every hop, the cost value carried by the msg is aggregated
 * with the local cost for a given output port, with each 
 * value (local & non-local) weighted based on user-specified params.
 * Default weights are 50-50. 
 */

#include "ocin_router_cost_mgr_1D.h"

void ocin_router_cost_mgr_1D::init(ocin_router *parent, const string &_name) {
  // call base init() w/ fully specified name
  stringstream *ss = new stringstream();
  *ss << parent->node_info->name.c_str() << "." << _name;
  ocin_router_cost_mgr::init(parent, ss->str());
  delete ss;

  // configure the message queue based on # of network ports
  _port_count = parent_router->node_info->port_count;

  // init the msg queue
  _cost_msg_queue = new ocin_cost_msg *[_port_count];
  for (int i=0; i<_port_count; i++) {
    _cost_msg_queue[i] = NULL;
  }
	
  // sum of weights used in computing weighted cost
  _sum_of_weights = param_cost_multiplier_local + param_cost_multiplier_remote;
	
#ifdef COST_DEBUG	
  {
    stringstream tmp;
    tmp   << "Initialized. Cost manager = 1D";
    ocin_name_debug(name,tmp.str());
  }
#endif
}


/* update_cost()
 *  - simple updater, uses the rolling history sum as the cost 
 */
void ocin_router_cost_mgr_1D::update_cost() {
  // for each input port
  for (int i=0; i < _port_count; i++) {
    // next cycle's cost value;
    int new_cost;
    // cost packet
    ocin_cost_msg *msg;
		
    // get last cycle's monitor value
    int reg_cost = _cost_regs[0][i].get_hist_sum();
		
    // merge w/ upstream congestion info 
    if (_cost_msg_queue[i] != NULL) {
      // grab the congestion msg from the input queue
      msg = _cost_msg_queue[i];

#ifdef COST_DEBUG
      {
        stringstream tmp;
        tmp << "Processing msg # " << msg->msg_id 
            << " (p_in " << i << " --> p_out " << msg->p_out << ")" << endl;
        ocin_name_debug(name,tmp.str());
      }
#endif

      _cost_msg_queue[i] = NULL;			
      // aggregate upstream cost w/ the local monitor
      int msg_cost = msg->get_cost();
			
      // weigh the local & non-local costs
      int local_cost = reg_cost * param_cost_multiplier_local;
      int remote_cost = msg_cost * param_cost_multiplier_remote;
      new_cost = (local_cost + remote_cost) / _sum_of_weights;
					
      // update msg w/ new cost
      msg->update_cost(new_cost);
    }
    else {
      new_cost = reg_cost;
      msg = new ocin_cost_msg(new_cost);
			
      // set the output port for the msg
      int p_out = pin2pout_straight(i);
      msg->p_out = p_out;

#ifdef COST_DEBUG
      {
        stringstream tmp;
        tmp   <<  "Generating new msg # " << msg->msg_id << " on p_in " << i;
        ocin_name_debug(name,tmp.str());
      }
#endif
    }
		
    // Enque msg for transmission @ p_out
    // Note that in this scheme, the msg is always sent out through the same
    // output port, as it travels through 1 dimension.
    parent_router->output_units[msg->p_out].enque(msg);
		
    // update local cost
    _cost_regs[0][i].set_cost(new_cost);

#ifdef COST_DEBUG
    {
      stringstream tmp;
      tmp   <<  "Updated cost @ p_out " << i << " = " << new_cost;
      ocin_name_debug(name,tmp.str());
    }
#endif
  }
}


/* enque_cost_msg()
 *  - enque the received cost message for processing by the cost manager
 *    during the update phase
 */
void ocin_router_cost_mgr_1D::enque_cost_msg(ocin_cost_msg *cost_msg) {
  int p_in = cost_msg->p_in;
  _cost_msg_queue[p_in] = cost_msg;
}
