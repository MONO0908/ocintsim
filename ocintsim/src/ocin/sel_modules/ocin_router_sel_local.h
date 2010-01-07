#ifndef OCIN_ROUTER_SEL_LOCAL_H_
#define OCIN_ROUTER_SEL_LOCAL_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_router.h"

class ocin_router;

class ocin_router_sel_local : public ocin_router_sel {
  public:
    
    ocin_router_sel_local(ocin_router_cost *route_cost)
               : ocin_router_sel(route_cost) {};
    void init(ocin_router *parent, const string &_name);
    int select (ocin_msg *msg);
    int break_tie(vector <int> min_links);
    bool has_free_vc(ocin_msg *msg, int p_out);
    int first_avail(ocin_msg *msg);
};

#endif /*OCIN_ROUTER_SEL_LOCAL_H_*/
