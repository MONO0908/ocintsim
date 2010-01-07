/* 
 */

#include "ocin_router_cost_mgr_fanin.h"

void ocin_router_cost_mgr_fanin::init(ocin_router *parent, const string &_name) {
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
	
#ifdef DEBUG	
  {
    stringstream tmp;
    tmp   << "Initialized. Cost manager = fanin";
    ocin_name_debug(name,tmp.str());
  }
#endif
}


/* update_cost()
 *  - simple updater, uses the rolling history sum as the cost 
 */
void ocin_router_cost_mgr_fanin::update_cost() {
  vector <ocin_cost_msg *> output_msg_q;
	
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

#ifdef DEBUG
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
    }
    else {
      new_cost = reg_cost;
      msg = new ocin_cost_msg();
			
      // set the output port for the msg
      int p_out = pin2pout_straight(i);
      msg->p_out = p_out;

#ifdef DEBUG
      {
        stringstream tmp;
        tmp   <<  "Generating new msg # " << msg->msg_id << " on p_in " << i;
        ocin_name_debug(name,tmp.str());
      }
#endif
    }
    // Store the msg for subsequent transmission.
    // Note that it hasn't yet been updated w/ the final cost that it will carry
    output_msg_q.push_back(msg);
		
    // update local cost
    _cost_regs[0][i].set_cost(new_cost);

#ifdef DEBUG
    {
      stringstream tmp;
      tmp   <<  "Updated cost @ p_out " << i << " = " << new_cost;
      ocin_name_debug(name,tmp.str());
    }
#endif
  }
	
  // go through the msgs, updating their respective costs
  for (int i=0; i<output_msg_q.size(); i++) {
    ocin_cost_msg *msg = output_msg_q[i];
    int same_dir_cost = 0;             // cost from the same dir as the msg (50% weight)
    int orthogonal_dirs_cost_sum = 0;  // costs from other directions (weighted evenly)
    int orthogonal_dirs_count = 0;       // used to compute the weights
                		
    int p_out = msg->p_out;
    same_dir_cost = _cost_regs[0][i].cost_reg;

    // DELETE
    //cout << fcns(name) << "Processing msg # " << msg->msg_id << " (p_in " << i << " --> p_out " << msg->p_out << "): " << endl;
    //cout << "\t Same dir cost = " << same_dir_cost << endl;

		
    for (int j=0; j<_port_count; j++) {	
      if ((j != i) && (j != p_out)) {
        orthogonal_dirs_cost_sum += _cost_regs[0][j].cost_reg;
        orthogonal_dirs_count++;
        // DELETE
        //cout << "\t Dir = " << j << ", cost = " << _cost_regs[0][j].cost_reg << ", orth count = " << orthogonal_dirs_count << endl;
      }
    }
		
    // compute new cost
    int orthogonal_dirs_avg = orthogonal_dirs_cost_sum / orthogonal_dirs_count;
    int new_cost = (same_dir_cost + orthogonal_dirs_avg) / 2;
	
    // update msg w/ new cost
    msg->update_cost(new_cost);
		
#ifdef DEBUG
    {
      stringstream tmp;
      tmp   <<  "Updated cost for msg # " << msg->msg_id
            << " (p_in " << i << " --> p_out " << msg->p_out << "). "
            << " New cost = " << new_cost << endl; 
      ocin_name_debug(name,tmp.str());
    }
#endif
		
    // Enque msg for transmission @ p_out
    // Note that the msg is always sent out through the same output port
    // at every hop, although its value is the result of aggregation at 
    // multiple ports
    parent_router->output_units[msg->p_out].enque(msg);
  }
}


/* enque_cost_msg()
 *  - enque the received cost message for processing by the cost manager
 *    during the update phase
 */
void ocin_router_cost_mgr_fanin::enque_cost_msg(ocin_cost_msg *cost_msg) {
  int p_in = cost_msg->p_in;
  _cost_msg_queue[p_in] = cost_msg;
}
