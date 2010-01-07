#ifndef OCIN_ROUTER_RU_O1TURN_BEC_H_
#define OCIN_ROUTER_RU_O1TURN_BEC_H_


#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_router.h"


class ocin_router_ru_o1turn_bec : public ocin_router_ru {
  public:

    virtual void init(ocin_router *parent, const string &_name);
    void route (ocin_flit *flit);
};

#endif /*OCIN_ROUTER_RU_O1TURN_BEC_H_*/
