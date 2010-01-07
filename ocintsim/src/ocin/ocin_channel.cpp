#include "ocin_channel.h"
#include "ocin_helper.h"
  
void ocin_channel::init(const string &_name, int delay) {
  name = _name;
  
  // set the wire delay
  _wire_delay = delay;
  
  // Compute the additional latency for low-BW RCA.
  _rca_serial_delay = param_low_bw_rca ? param_low_bw_rca_latency-1 : 0;
  
  mon.init(name);
}

/* transmit_flit()
 *  - compute arrival cycle for the flit (similar for credit)
 *  - push arrival cycle & flit itself onto respective queues. 
 */
void ocin_channel::transmit_flit(ocin_flit *flit) {
  // Components that determine flit's ready (arrival) time:
  // 1. (Current cycle count + 1) -- latch/fifo at receiving end
  // 2.  + wire_delay             -- wire latency
  // 3.  + router_pipeline_lat    -- additional router pipeline latency
  //                                 (eg: XB traversal)

  
  tsim_u64 rdy_time = ocin_cycle_count + 1 
    + _wire_delay 
    + param_router_pipeline_latency;


  flit->arrival_time = rdy_time;

  _data.push_back(flit);
	
  //#ifdef PARANOID
  // Keep track of the hop count
  if (flit->is_header) {
    flit->msg->hops++;
  }
  //#endif

#ifdef DEBUG
  //debug (fcns(name) + "Enquing data. Ready time = " + rdy_time + "\n");
  {
    stringstream tmp;
    tmp  << "Enquing flit. msg #" << flit->msg->pid 
         << ". Ready time = " << rdy_time;
    ocin_name_debug(name,tmp.str());
  }

#endif

}


/* transmit_credit()
 */
void ocin_channel::transmit_credit(ocin_credit *credit) {

  tsim_u64 rdy_time = ocin_cycle_count + 1
    + _wire_delay; 
  //+ param_wire_delay;
  credit->arrival_time = rdy_time;
  _credits.push_back(credit);

#ifdef DEBUG
  {
    stringstream tmp;
    tmp  << "Enquing credit. Ready time = " << rdy_time; 
    ocin_name_debug(name,tmp.str());
  }

#endif
}


/* transmit_cost()
 */
void ocin_channel::transmit_cost(ocin_cost_msg *cost_msg) {
  // includes the serialization delay for bandwidth-conserving RCA
  tsim_u64 rdy_time = ocin_cycle_count + param_wire_delay + _rca_serial_delay + 1 + param_extra_rca_delay;
  cost_msg->arrival_time = rdy_time;
  _cost.push_back(cost_msg);

#ifdef COST_DEBUG
  {
    stringstream tmp;
    tmp  << "Enquing cost msg. Ready time = " << rdy_time; 
    ocin_name_debug(name,tmp.str());
  }

#endif	
}


/* receive_flit()
 *  - if data in channel & arrival time has come, return flit at the head
 */
ocin_flit * ocin_channel::receive_flit() {
  if (_data.empty()) {
    return NULL;
  }

  tsim_u64 rdy_time = _data.front()->arrival_time;
	
  if (ocin_cycle_count > rdy_time) {
    {
      stringstream tmp;
      tmp  << "Error! Data arrival time has passed."      
           << " Ready time = " << rdy_time
           << " Current time = " << ocin_cycle_count;
//      ocin_name_fatal(name,tmp.str());	//	SP DELETE
    }
//    exit(0);	//	SP DELETE
  }
	
  if (ocin_cycle_count == rdy_time) {
    ocin_flit *flit = _data.front();
    if (flit == NULL) {
      {
        stringstream tmp;
        tmp  << "No flit in channel " << name << " despite a valid time stamp!"<< ", data ready time = " << rdy_time ;     
        ocin_name_fatal(name,tmp.str());
      }

      exit(0);
    }
    _data.pop_front();

    if (flit->msg->is_counted ){
      mon.flit_count++; // only increment when the flit is
      // good and ready...
    }
    return flit;
  }
  else {
    return NULL;
  }
}


/* receive_credit()
 */
ocin_credit * ocin_channel::receive_credit() {

  if (_credits.empty()) {
    return NULL;
  }

  tsim_u64 rdy_time = _credits.front()->arrival_time;
	
  if (ocin_cycle_count > rdy_time) {
    {
      stringstream tmp;
      tmp  << fcns(name) << "Error! Credit arrival time has passed."  
           << " Ready time = " << rdy_time;
            
      ocin_name_fatal(name,tmp.str());
    }
    exit(0);
  }
	
  if (ocin_cycle_count == rdy_time) {
    ocin_credit *credit = _credits.front();
    if (credit == NULL) {
      {
        stringstream tmp;
        tmp  << "No credit in channel " << name << " despite a valid time stamp! " 
             << ", credit ready time = " << rdy_time;     
        ocin_name_fatal(name,tmp.str());
      }

      exit(0);
    }
    _credits.pop_front();
	 
    return credit;
  }
  else {
    return NULL;
  }
}


/* receive_cost()
 */
ocin_cost_msg * ocin_channel::receive_cost() {

  if (_cost.empty()) {
    return NULL;
  }

  tsim_u64 rdy_time = _cost.front()->arrival_time;
	
  if (ocin_cycle_count > rdy_time) {
    {
      stringstream tmp;
      tmp  << fcns(name) << "Error! Cost msg arrival time has passed." 
           << " Ready time = " << rdy_time;
            
      ocin_name_fatal(name,tmp.str());
    }
    exit(0);
  }
	
  if (ocin_cycle_count == rdy_time) {
    ocin_cost_msg *cost_msg = _cost.front();
    if (cost_msg == NULL) {
      {
        stringstream tmp;
        tmp  << "No cost msg in channel " << name << " despite a valid time stamp! " 
             << ", cost msg ready time = " << rdy_time;     
        ocin_name_fatal(name,tmp.str());
      }

      exit(0);
    }
    _cost.pop_front();
	 
    return cost_msg;
  }
  else {
    return NULL;
  }
}
