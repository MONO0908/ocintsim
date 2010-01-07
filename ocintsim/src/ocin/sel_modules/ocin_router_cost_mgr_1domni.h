#ifndef OCIN_ROUTER_COST_MGR_1DOMNI_H_
#define OCIN_ROUTER_COST_MGR_1DOMNI_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_router.h"


class ocin_router;


class ocin_router_cost_mgr_1domni : public ocin_router_cost_mgr {
public:

  map<string, ocin_router> *nodes; 


  ocin_router_cost_mgr_1domni(vector <vector <ocin_router_cost_reg> > &cost_regs)
    : ocin_router_cost_mgr(cost_regs) {};
  void init(ocin_router *parent, const string &_name);
  void update_cost ();

  
private:
  ocin_cost_msg *  *_cost_msg_queue;  // array of pointers
  int _port_count;
  
  int _sum_of_weights;  // for computing the weighted avg cost
};

#endif /*OCIN_ROUTER_COST_MGR_1domni_H_*/
