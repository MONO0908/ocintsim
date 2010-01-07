#ifndef OCIN_ROUTER_SEL_H_
#define OCIN_ROUTER_SEL_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_helper.h"
//#include "ocin_router.h"   //--- this messes up the inheritance hierarchy

class ocin_router;
class ocin_router_cost;

class ocin_router_sel {
  public:
    string name;
    ocin_router *parent_router;
    
    ocin_router_sel(ocin_router_cost *route_cost) 
          : _cost_fn(route_cost)  {};
    virtual void init(ocin_router *parent, const string &_name);
    virtual int select(ocin_msg *msg) = 0;

    
  protected:
    int MAX_INT;
	ocin_router_cost *_cost_fn;
	int _num_dimensions;
};

#endif /*OCIN_ROUTER_SEL_H_*/
