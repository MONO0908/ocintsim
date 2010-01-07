#ifndef OCIN_ROUTER_XBALLOC_BEC_POWER_2LEVEL_H_
#define OCIN_ROUTER_XBALLOC_BEC_POWER_2LEVEL_H_

#include "ocin_defs.h"
#include "ocin_router.h"
#include "ocin_mon_port.h"

class ocin_router;
class ocin_router_vc_fifo;
class ocin_router_xbar;

class ocin_router_xballoc_bec_power_2level : public ocin_router_xballoc {
  public:
    
    ocin_router_xballoc_bec_power_2level(vector <vector <ocin_router_vc_fifo> > &vc_fifos,
                                         vector <vector <ocin_router_vc_fifo *> > &lvc_fifos,
                                         vector <vector <ocin_vc_status> > &downstream_vc_stats,
                                         vector <deque <int> > &free_vc_list,
                                         vector <vector <ocin_mon_port> > &pin_mon,
                                         vector <vector <ocin_mon_port> > &pout_mon,
                                         ocin_router_xbar *xbar) 
                                                     : _lvc_fifos(lvc_fifos),
                                                       ocin_router_xballoc(vc_fifos, 
                                                                           downstream_vc_stats, 
                                                                           free_vc_list, 
                                                                           pin_mon,
                                                                           pout_mon,
                                                                           xbar)   {};
    
    void init(ocin_router *parent, const string &_name);
    void allocate_xbar();
    inline void gen_credit_update(ocin_router_vc_fifo *vc_in_ptr);
    
  private:
    // Per input port VC priority
    vector <list <int> >  _vc_priorities; 
    // Per output port port priority (ie, priority of each p_in at a given p_out)
    vector <list <int> > _port_priorities;
	// logical ports used by XB allocator
    vector <vector <ocin_router_vc_fifo *> > _lvc_fifos;
};


#endif /*OCIN_ROUTER_XBALLOC_BEC_POWER_2LEVEL_H_*/
