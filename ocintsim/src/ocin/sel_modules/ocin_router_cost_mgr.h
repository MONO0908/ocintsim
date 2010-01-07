#ifndef OCIN_ROUTER_COST_MGR_H_
#define OCIN_ROUTER_COST_MGR_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_helper.h"
#include "ocin_cost_msg.h"

class ocin_router;
class ocin_router_cost_reg;

class ocin_router_cost_mgr {
  public:
    string name;
    ocin_router *parent_router;
    
    ocin_router_cost_mgr(vector <vector <ocin_router_cost_reg> > &cost_regs) 
                                            : _cost_regs(cost_regs)  {};
    virtual void init(ocin_router *parent, const string &_name);
    virtual void update_cost() = 0;
    virtual void enque_cost_msg(ocin_cost_msg *cost_msg);

    
  protected:
    vector <vector <ocin_router_cost_reg> > &_cost_regs;
};

#endif /*OCIN_ROUTER_COST_MGR_H_*/
