#ifndef OCIN_ROUTER_OU_H_
#define OCIN_ROUTER_OU_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_router.h"

class ocin_router;

/* Output unit.
 * See .cpp file for details 
 */
class ocin_router_ou {
  public:
	string name;
	ocin_router *parent_router;
	ocin_channel *channel_out;
//	vector <ocin_vc_status> &downstream_vc_stats;  // VC credits @ downstream node
	
//	ocin_router_ou(vector <ocin_vc_status> &vc_stats) : downstream_vc_stats(vc_stats) {};
   	void init(ocin_router *parent, const string &_name, ocin_channel *c_out);
   	void transmit();
   	
   	/* enque (flit or credit)
     *  - push the flit onto the egress_data queue
     *  - no error checking is performed here: all of that is done by transmit().
     *    This should reduce the overhead of error checking, since enque() might be called 
     *    multiple times/cycle, while transmit() should only be called once.
     */
   	inline void enque(ocin_flit *flit)         { _egress_data.push_back(flit); };
   	inline void enque(ocin_credit *credit)     { _egress_credits.push_back(credit); }; 
    inline void enque(ocin_cost_msg *cost_msg) { _egress_cost_msgs.push_back(cost_msg); };
  
  private:
    deque <ocin_flit *>     _egress_data;
    deque <ocin_credit *>   _egress_credits;
    deque <ocin_cost_msg *> _egress_cost_msgs;
    
    // Store cost msgs to support BW-limited RCA
    deque <ocin_cost_msg *> _cost_msg_stash; 
};

#endif /*OCIN_ROUTER_OU_H_*/
