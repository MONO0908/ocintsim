#ifndef OCIN_ROUTER_COST_MGR_FANIN_H_
#define OCIN_ROUTER_COST_MGR_FANIN_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_router.h"

class ocin_router;

class ocin_router_cost_mgr_fanin : public ocin_router_cost_mgr {
public:
  
  ocin_router_cost_mgr_fanin(vector <vector <ocin_router_cost_reg> > &cost_regs)
    : ocin_router_cost_mgr(cost_regs) {};
  
  void init(ocin_router *parent, const string &_name);
  void update_cost ();
  void enque_cost_msg(ocin_cost_msg *cost_msg);
  
private:
  ocin_cost_msg *  *_cost_msg_queue;  // array of pointers
  int _port_count;
    
  int _sum_of_weights;  // for computing the weighted avg cost
};

#endif /*OCIN_ROUTER_COST_MGR_FANIN_H_*/
