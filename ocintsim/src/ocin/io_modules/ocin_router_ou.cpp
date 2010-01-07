#include "ocin_router_ou.h"

/* Output unit.
 * Drives data, credits & congestion info onto the wires.
 * For each entity, computes & drives the arrival time. 
 */

void ocin_router_ou::init(ocin_router *parent, const string &_name, ocin_channel *c_out) {
	stringstream *ss = new stringstream();
	*ss << parent->node_info->name.c_str() << "." << _name;
	name = ss->str();
	delete ss;
	
	parent_router = parent;
	channel_out = c_out;
	
#ifdef DEBUG
        {
          stringstream tmp;
          tmp   << "Init complete. ";
          ocin_name_debug(name,tmp.str());
        }

	if (c_out != NULL) {
          stringstream tmp;
          tmp  << " Output: " << c_out->name;
          ocin_name_debug(name,tmp.str());
        } else {
          stringstream tmp;
          tmp  << " Output: NONE";
          ocin_name_debug(name,tmp.str());
        }
#endif

}


/* transmit()
 *  - push data & flits into the channel
 */
void ocin_router_ou::transmit() {
	
	/** drive flits **/
	
	// Only 1 flit/cycle is currently supported
	if (_egress_data.size() > 1) {
          {
            stringstream tmp;
            tmp  << "Multiple egress flits enqued. This is currently not supported!";     
            tmp  << "\t deque size = " << _egress_data.size();     
            ocin_name_fatal(name,tmp.str());
          }
          exit(0);
	}
	
	// if there's data to transmit, push it into the channel
	// TODO: to support wide (>1) channels, wrap this w/ a while loop (iterate over egress_data)
	if (!_egress_data.empty()) {
		ocin_flit *flit = _egress_data.front();
		_egress_data.pop_front();
		channel_out->transmit_flit(flit);
	}
	
	/** drive credits **/
	
	// Only 1 credit/cycle is currently supported
	if (_egress_credits.size() > 1) {
          {
            stringstream tmp;
            tmp  << "Multiple egress credits enqued. This is currently not supported!";     
            tmp  << "\t deque size = " << _egress_credits.size();     
            ocin_name_fatal(name,tmp.str());
          }
          exit(0);
	}
	
	// if there is a credit update, push it into the channel
	// TODO: to support wide (>1) channels, wrap this w/ a while loop (iterate over egress_credits)
	if (!_egress_credits.empty()) {
		ocin_credit *credit = _egress_credits.front();
		_egress_credits.pop_front();
		channel_out->transmit_credit(credit);
	}
	
	/** drive congestion info **/
#ifdef OCIN_MON
 {
	int q_size = _egress_cost_msgs.size();
	while (!_egress_cost_msgs.empty()) {
        ocin_cost_msg *cost_msg = _egress_cost_msgs.front();
        _egress_cost_msgs.pop_front();
		if (channel_out != NULL) {

			// For BW-limited RCA, need to model "serialized" updates that  
			// take multiple cycles to complete. This is accomplished in two ways:
			// 1. The actual serialization latency is modeled in the channel,
			//    through an additional latency equal to the serialization delay.
			// 2. To eliminate the pipeline effect, where a fresh update is 
			//    available every cycle after the initial delay, we re-send
			//    the same cost_msg until a new update interval.
			if (param_low_bw_rca) {
				// In the middle of a "serial" update? re-transmit a copy
		   		if (ocin_cycle_count % param_low_bw_rca_latency != 0) {
		   			// delete this update
					delete cost_msg;
		   			
		   			// transmit a stashed copy, if available
		   			// Note that the ordering is somewhat random and assumes
		   			// that everything has the same latency,etc. Generally, multiple
		   			// cost messages only impact Quad RCA.
		   			if (_cost_msg_stash.empty()) {
		   				continue;
		   			}

		   			cost_msg = new ocin_cost_msg( _cost_msg_stash.front() );
					// next forward a copy of a different msg (different quadrant)
		   			_cost_msg_stash.push_back(_cost_msg_stash.front());
		   			_cost_msg_stash.pop_front();
		   		}
		   		else {
		   			// store the msg for subsequent re-transmission
		   			// Delete an old msg by treating the stash as a circular queue.
		   			if (_cost_msg_stash.size() >= q_size) 
		   				_cost_msg_stash.pop_front();
		   			_cost_msg_stash.push_back(cost_msg);

		   			
		   			// transmit a copy
		   			cost_msg = new ocin_cost_msg(cost_msg);
		   		}
			}

	        channel_out->transmit_cost(cost_msg);
		}
		else {
			delete cost_msg;
		}
	}
	
 }
#endif

}
