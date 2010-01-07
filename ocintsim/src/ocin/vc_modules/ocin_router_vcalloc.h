#ifndef OCIN_ROUTER_VCALLOC_H_
#define OCIN_ROUTER_VCALLOC_H_

#include <list>
#include <vector>
#include "ocin_defs.h"
//#include "ocin_router_vc_fifo.h"
//#include "ocin_router.h"
#include "ocin_helper.h"

class ocin_router;
class ocin_router_vc_fifo;
class ocin_mon_port;

class ocin_router_vcalloc {
  public:
	string name;
    ocin_router *parent_router;
    
    ocin_router_vcalloc(vector <vector <ocin_router_vc_fifo> > &vc_fifos,
                        vector <vector <ocin_router_vc_fifo> > &ej_fifos,
                        vector <deque <int> > &free_vc_list,
                        vector <vector <ocin_mon_port> > &pin_mon,
                        vector <vector <ocin_mon_port> > &pout_mon)
                           : _vc_fifos(vc_fifos), 
                             _ej_fifos(ej_fifos),
                             _free_vc_list(free_vc_list),
                             _pin_mon(pin_mon),
                             _pout_mon(pout_mon)           {};
    virtual void init(ocin_router *parent, const string &_name);
    virtual void allocate_vc() = 0;
  
  protected:
    // for convenience, to avoid extra accesses via the parent_router pointer
    vector <vector <ocin_router_vc_fifo> > &_vc_fifos;   
    vector <vector <ocin_router_vc_fifo> > &_ej_fifos;
    vector <deque <int> > &_free_vc_list;   // index of downstream VCs that can be allocated
    vector <vector <ocin_mon_port> > &_pin_mon;
    vector <vector <ocin_mon_port> > &_pout_mon;
    int _local_port;
  
};

#endif /*OCIN_ROUTER_VCALLOC_H_*/
