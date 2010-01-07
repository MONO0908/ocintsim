#ifndef OCIN_ROUTER_XBAR_H_
#define OCIN_ROUTER_XBAR_H_

#include "ocin_defs.h"
#include "ocin_router_vc_fifo.h"
#include "ocin_router.h"

class ocin_router;
class ocin_router_vc_fifo;
class ocin_router_ej;

class ocin_router_xbar {
  public:
    string name;
    ocin_router *parent_router;
        
    deque <ocin_router_vc_fifo *> xbar_q;       // ptrs to fifos which have been granted
                                                // X-bar access by the xbar allocator

    ocin_router_xbar(vector <vector <ocin_router_vc_fifo> > &vc_fifos,
                     deque <ocin_router_ou> &output_units,
                     deque <ocin_router_ej> &local_ou)
                           : _vc_fifos(vc_fifos), 
                             _output_units(output_units), 
                             _ej_port(local_ou)            {};
    void init(ocin_router *parent, const string &_name);
    // transfer flits from VC fifos to output units
    void transfer();
    // access fns for managing the xbar queue
    inline void enque(ocin_router_vc_fifo *src_vc) { xbar_q.push_back(src_vc); };
    
  private:
    vector <vector <ocin_router_vc_fifo> > &_vc_fifos;
    deque <ocin_router_ou> &_output_units; 
    deque <ocin_router_ej> &_ej_port;
    
    int _first_ejector_idx;
};

#endif /*OCIN_ROUTER_XBAR_H_*/
