#ifndef OCIN_ROUTER_COST_H_
#define OCIN_ROUTER_COST_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_credit.h"
#include "ocin_helper.h"
#include "ocin_router.h"

class ocin_router;


class ocin_router_cost {
  public:
    string name;
    ocin_router *parent_router;
    
    ocin_router_cost(vector <vector <ocin_vc_status> > &downstream_vc_stats) 
          : _downstream_vc_stats(downstream_vc_stats)  {};
    virtual void init(ocin_router *parent, const string &_name);
    // cost functions 
    int cost(int p_out, ocin_msg *msg);
    int cost_free_vc_nohist(int p_out, ocin_msg *msg);   // buffer utilization at p_in (history=0)
    int cost_buff_nohist(int p_out, ocin_msg *msg);      // buffer utilization at p_in (history=0)
    int cost_buff_hist(int p_out, ocin_msg *msg, ocin_router *parent_rtr);  // buffer utilization (history > 0)
   	int cost_omni_1D(int p_out, ocin_msg *msg, ocin_router *parent_rtr, int &hops);  // aggregating in 1D
   	int cost_omni_all(int p_out, ocin_msg *msg, ocin_router *parent_rtr, int &max_cost);  // aggregating to dest
    int cost_pin_vcalloc(int p_out, ocin_msg *msg);      // VC alloc contention observed at p_in
    int cost_pin_xballoc(int p_out, ocin_msg *msg);      // XB contention observed at p_in 
    int cost_pout_vcalloc(int p_out, ocin_msg *msg);     // VC alloc contention observed at p_out
    int cost_pout_xballoc(int p_out, ocin_msg *msg);     // XB contention observed at p_out
    
    // helper
    bool is_free_vc(deque <int> *free_vc_list, int vc_idx);
    
  private:
    Cost_Function cost_fn;
    vector <vector <ocin_vc_status> > &_downstream_vc_stats;  // DELETE
};

#endif /*OCIN_ROUTER_COST_H_*/
