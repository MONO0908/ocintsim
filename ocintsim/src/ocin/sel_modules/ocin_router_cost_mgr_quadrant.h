#ifndef OCIN_ROUTER_COST_MGR_QUADRANT_H_
#define OCIN_ROUTER_COST_MGR_QUADRANT_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_router.h"

class ocin_router;

class ocin_router_cost_mgr_quadrant : public ocin_router_cost_mgr {
public:
  
  ocin_router_cost_mgr_quadrant(vector <vector <ocin_router_cost_reg> > &cost_regs)
    : ocin_router_cost_mgr(cost_regs) {};

  void init(ocin_router *parent, const string &_name);
  void update_cost ();
  void enque_cost_msg(ocin_cost_msg *cost_msg);
  
private:
  // a cost msg queue for each quadrant
  vector <map <int, ocin_cost_msg *> > _cost_msg_queue; 
  int _port_count;
  int _quadrants;
  
  int _sum_of_weights;  // for computing the weighted avg cost
};

#endif /*OCIN_ROUTER_COST_MGR_QUADRANT_H_*/
