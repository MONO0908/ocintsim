#ifndef OCIN_ROUTER_COST_REG_H_
#define OCIN_ROUTER_COST_REG_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_credit.h"
#include "ocin_helper.h"
#include "ocin_router.h"
#include "ocin_channel.h"

class ocin_router;

enum Edge_Cost_Fn {EDGE_MAX, EDGE_MIN, EDGE_AVG, EDGE_LOCAL_AVG, EDGE_LOCAL_FANIN};

class ocin_router_cost_reg {
  public:
    string name;
    ocin_router *parent_router;
    int p_out;
    
    ocin_channel * output_link; /* pointer to output port */
    
	int cost_reg;
	int hist_sum;
	
	// is this an ejection port or unconnected output port?
	bool is_ejector;
	bool is_edge;
	
	void init(ocin_router *parent, const string &_name, int port_idx);
	void update();
	void update_history(int new_val);
	
	// setters & getters
	inline void set_cost(int cost) { cost_reg = cost; };
	inline int get_cost()     { return cost_reg; };
	inline int get_hist_sum() { return hist_sum; };

	
  private:
    Cost_Reg_Function cost_reg_fn;
    Edge_Cost_Fn edge_cost_fn;
	deque <int> history;
	
	inline int cost_free_vc_nohist();
	inline int cost_free_vc_hist();
	inline int cost_buff_nohist();
	inline int cost_buff_hist();
	inline int cost_vc_ages();
	inline int cost_vc_predictive();
	inline int cost_combined_vc_xb(); 
	inline int cost_combined_vc_buff();
	inline int cost_combined_xb_buff();
	inline int cost_combined_vc_xb_buff();
	inline int cost_experimental();
	inline int cost_pkt_delay();
	inline int cost_pout_mon(Cost_Reg_Function cost_fn);
	
};

#endif /*OCIN_ROUTER_COST_REG_H_*/
