#ifndef OCIN_ROUTER_SEL_1MATCH_H_
#define OCIN_ROUTER_SEL_1MATCH_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_router.h"

class ocin_router;

class ocin_router_sel_1match : public ocin_router_sel {
  public:
    
    ocin_router_sel_1match(ocin_router_cost *route_cost)
               : ocin_router_sel(route_cost) {};
    void init(ocin_router *parent, const string &_name);
    int select (ocin_msg *msg);
};


#endif /*OCIN_ROUTER_SEL_1MATCH_H_*/
