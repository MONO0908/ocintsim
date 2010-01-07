#ifndef OCIN_DEFS_H
#define OCIN_DEFS_H

#include "tsim.h"
#include <iostream>
#include <sstream>
#include <string>
#include <deque>

extern tsim_u64 param_seed;

extern tsim_u64 max_cycles;

extern double param_src_sigma;
extern double param_dst_sigma;
extern string param_src_sigma_str;
extern string param_dst_sigma_str;

extern tsim_u64 param_extra_rca_delay;
extern tsim_u64 param_pkt_throttle;
extern tsim_u64 param_dim_ave;
extern tsim_u64 param_repreroute_delay;
extern tsim_u64 param_reroute_limit;
extern tsim_u64 param_preroute_scale;
extern tsim_u64 param_preroute_baseline;
extern tsim_u64 param_repreroute_hops;
extern tsim_u64 param_repreroute_cycles;
extern tsim_u64 param_preroute_pkts;
extern tsim_u64 param_midpoint_file;
extern tsim_u64 param_ocn_4proc;
extern tsim_u64 param_ocn_2proc;

extern tsim_u64 param_network_terminals;
extern tsim_u64 param_num_x_tiles;
extern tsim_u64 param_num_y_tiles;

extern tsim_u64 param_req_size;
extern tsim_u64 param_random_req_size;
extern tsim_u64 param_bimodal_req_size;
extern tsim_u64 param_bimodal_size1;
extern tsim_u64 param_bimodal_size2;
extern tsim_u64 param_bimodal_size1_flits;
extern tsim_u64 param_bimodal_size2_flits;
extern tsim_u64 param_baseline_channel_width;
extern tsim_u64 param_scaled_bisection_width;
extern tsim_u64 param_bisection_channel_count;
extern tsim_u64 param_bisection_channel_width;

extern tsim_u64 param_fgen_rate;

extern tsim_u64 param_show_body_flit_log;

/* brg */
extern tsim_u64 param_wire_delay;      // per-hop
extern tsim_u64 param_hops_per_cycle;  // # of hops traversed on a wire per clock cycle
extern tsim_u64 param_router_pipeline_latency;

extern tsim_u64 param_max_expr_hop;

extern tsim_u64 param_flit_max;

extern tsim_u64 param_inj_vc_count;
extern tsim_u64 param_ej_vc_count;

extern tsim_simulator *simulator;
extern stringstream *logstring;

extern string param_netcfg_file;

extern string param_vis_fifo_type;
extern string param_vis_link_type;

extern string param_tracefile_name;
extern string param_selfsim_trace1;
extern string param_selfsim_trace2;

extern tsim_u64 param_selfsim_inj;

extern tsim_u64 param_incr_stats;
extern tsim_u64 param_incr_chkpt;
extern tsim_u64 param_chkpt_interval;

extern tsim_u64 param_stats_interval;
extern tsim_u64 param_node_stats;

extern tsim_u64 param_cost_reg_history;
extern tsim_u64 param_cost_hop_history;

extern tsim_u64 param_vis_on;
extern tsim_u64 param_vis_start;
extern tsim_u64 param_vis_stop;

extern tsim_u64 param_warmup_cycles;
extern tsim_u64 param_max_packets;
extern tsim_u64 param_node_bw_stats;

extern tsim_u64 param_aggressive_vc_alloc;

extern tsim_u64 param_0delay_cost_msg_update;
extern tsim_u64 param_same_cycle_local_cost;
extern tsim_u64 param_adaptive_1avail;

extern tsim_u64 param_cost_multiplier_local;
extern tsim_u64 param_cost_multiplier_remote;
extern tsim_u64 param_use_max_quadrant_cost;

extern tsim_u64 param_cost_precision;

extern string param_edge_cost_fn;

extern tsim_u64 param_ignore_escape_vc;
extern tsim_u64 param_ignore_escape_vc_Ydir;
extern tsim_u64 param_ignore_escape_vc_Ydir_heuristic;

extern tsim_u64 param_low_bw_rca;
extern tsim_u64 param_low_bw_rca_latency;

extern tsim_u64 param_percent_inj_bw;
extern double param_adj_inj_bw;
extern unsigned param_avg_pkt_size;

extern tsim_u64 ocin_cycle_count;

string ocin_ftos ( float i );
string ocin_ftos_3 ( float i );
string ocin_ftos_5 ( float i );

int log_base2(int n);
/* int biased_bool_random(int i); */
/* int biased_bool_random_base(int i, int max); */

const int DIMENSIONS = 2;
const int X_DIM = 0;
const int Y_DIM = 1;
const int DIRECTIONS = 2;

const int EAST  = 0;
const int WEST  = 1;
const int SOUTH = 2;
const int NORTH = 3;

enum Quadrant           {NE_Q, SE_Q, NW_Q, SW_Q};

enum Cost_Function      {LOCAL, QUADRANT,
	                     // "real-time" cost estimation functions
	                     // (no pipeline delay, most recent buffer/vc info)
                         BUFF_NOHIST, BUFF_HIST, 
	                     FREE_VC_NOHIST,
	                     // 0 propagation delay aggregating cost functions
                         OMNI_1D, OMNI_ALL};

enum Cost_Reg_Function  {REG_BUFF_NOHIST, REG_BUFF_HIST, 
                         REG_FREE_VC_NOHIST,
                         REG_VC_STALLS, REG_VC_REQS, REG_VC_DEMAND, REG_VC_GRANTS,
                         REG_XB_STALLS, REG_XB_REQS, REG_XB_DEMAND, REG_XB_GRANTS,
                         REG_VC_AGES, REG_VC_PREDICTIVE,
                         REG_COMBINED_VC_XB, REG_COMBINED_VC_BUFF, REG_COMBINED_XB_BUFF, REG_COMBINED_VC_XB_BUFF,
                         REG_EXPERIMENTAL, REG_PKT_DELAY};
                    
enum Routing_Algos      {UNKNOWN, XY_DOR, YX_DOR, ADAPTIVE_XY, XY_DOR_BEC, O1TURN_BEC, PREROUTE};

// Number of checkpoints that monitors keep in their history
#ifndef MON_HISTORY_SIZE
	#define MON_HISTORY_SIZE 2
#endif

#endif
