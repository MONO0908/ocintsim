#ifndef OCIN_ROUTER_XBALLOC_H_
#define OCIN_ROUTER_XBALLOC_H_

#include <list>
#include <vector>
#include "ocin_defs.h"
//#include "ocin_router_vc_fifo.h"
//#include "ocin_router.h"
#include "ocin_helper.h"

class ocin_router;
class ocin_router_vc_fifo;
class ocin_vc_status;
class ocin_router_xbar;
class ocin_mon_port;

class ocin_router_xballoc {
  public:
    string name;
    ocin_router *parent_router;
    
    ocin_router_xballoc(vector <vector <ocin_router_vc_fifo> > &vc_fifos,
                        vector <vector <ocin_vc_status> > &downstream_vc_stats,
                        vector <deque <int> > &free_vc_list,
                        vector <vector <ocin_mon_port> > &pin_mon,
                        vector <vector <ocin_mon_port> > &pout_mon,
                        ocin_router_xbar *xbar)
                           : _vc_fifos(vc_fifos), 
                             _downstream_vc_stats(downstream_vc_stats),
                             _free_vc_list(free_vc_list), 
                             _pin_mon(pin_mon),
                             _pout_mon(pout_mon),
                             _xbar(xbar)                   {};
    virtual void init(ocin_router *parent, const string &_name);
    virtual void allocate_xbar() = 0;
    virtual inline void gen_credit_update(ocin_router_vc_fifo *vc_in_ptr) = 0;
    virtual void alloc_finalize() {};
  
  protected:
    // for convenience, to avoid extra accesses via the parent_router pointer
    vector <vector <ocin_router_vc_fifo> > &_vc_fifos;   
    vector <vector <ocin_vc_status> > &_downstream_vc_stats;
    vector <deque <int> > &_free_vc_list;   // index of downstream VCs that can be allocated
    vector <vector <ocin_mon_port> > &_pin_mon;
    vector <vector <ocin_mon_port> > &_pout_mon;
    ocin_router_xbar *_xbar;
};

#endif /*OCIN_ROUTER_XBALLOC_H_*/
