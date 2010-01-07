#ifndef OCIN_ROUTER_RU_ADAPTIVE_XY_H_
#define OCIN_ROUTER_RU_ADAPTIVE_XY_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_router.h"


class ocin_router_ru_adaptive_xy : public ocin_router_ru {
  public:
    
    virtual void init(ocin_router *parent, const string &_name);
    void route (ocin_flit *flit);
};

#endif /*OCIN_ROUTER_RU_ADAPTIVE_XY_H_*/
