#ifndef OCIN_ROUTER_VCALLOC_2LEVEL_H_
#define OCIN_ROUTER_VCALLOC_2LEVEL_H_

#include "ocin_defs.h"
#include "ocin_router.h"
#include "ocin_mon_allocator.h"

class ocin_router;
class ocin_router_vc_fifo;

class ocin_router_vcalloc_2level : public ocin_router_vcalloc {
  public:
    
    ocin_router_vcalloc_2level(vector <vector <ocin_router_vc_fifo> > &vc_fifos,
                               vector <vector <ocin_router_vc_fifo> > &ej_fifos,
                               vector <deque <int> > &free_vc_list,
                               vector <vector <ocin_mon_port> > &pin_mon,
                               vector <vector <ocin_mon_port> > &pout_mon) 
                                  : ocin_router_vcalloc(vc_fifos, 
                                                        ej_fifos,
                                                        free_vc_list,
                                                        pin_mon,
                                                        pout_mon) {};
    virtual void init(ocin_router *parent, const string &_name);
    void allocate_vc();
    
  private:
    // Per input port VC priority
    // TODO: the list should be wrapped by a vector of "classes"
    vector <vector <list <int> > > _vc_priorities;  // using list instead of slist 
                                                    // for performance reasons:
                                                    // list has O(C) insert/erase fns, 
                                                    // while slist's are O(n)
    // Per output port port priority (ie, priority of each p_in at a given p_out)
    vector <list <int> > _port_priorities; 

    inline int find_free_vc(deque <int> *free_vc_list, ocin_msg *msg, int p_out);
    inline void unfree_vc(int p_out, int vc_idx);
};

#endif /*OCIN_ROUTER_VCALLOC_2LEVEL_H_*/
