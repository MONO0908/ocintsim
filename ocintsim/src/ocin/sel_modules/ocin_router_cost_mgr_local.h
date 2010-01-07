#ifndef OCIN_ROUTER_COST_MGR_LOCAL_H_
#define OCIN_ROUTER_COST_MGR_LOCAL_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_router.h"

class ocin_router;

class ocin_router_cost_mgr_local : public ocin_router_cost_mgr {
public:
  
  ocin_router_cost_mgr_local(vector <vector <ocin_router_cost_reg> > &cost_regs)
    : ocin_router_cost_mgr(cost_regs) {};
  void init(ocin_router *parent, const string &_name);
  void update_cost ();
};

#endif /*OCIN_ROUTER_COST_MGR_LOCAL_H_*/
