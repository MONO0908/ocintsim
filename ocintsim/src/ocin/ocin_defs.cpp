#include "ocin_defs.h"

// This file has various global variables and some useful
// functions. Parameters are defined here as globals.
double param_src_sigma;
double param_dst_sigma;
string param_src_sigma_str;
string param_dst_sigma_str;

tsim_u64 param_extra_rca_delay;
tsim_u64 param_pkt_throttle;
tsim_u64 param_dim_ave;
tsim_u64 param_repreroute_delay;
tsim_u64 param_reroute_limit;
tsim_u64 param_preroute_scale;
tsim_u64 param_preroute_baseline;
tsim_u64 param_repreroute_hops;
tsim_u64 param_repreroute_cycles;
tsim_u64 param_preroute_pkts;


tsim_u64 param_midpoint_file;
tsim_u64 param_ocn_4proc;
tsim_u64 param_ocn_2proc;
tsim_u64 param_seed;

tsim_u64 param_network_terminals;
tsim_u64 param_num_x_tiles;
tsim_u64 param_num_y_tiles;

tsim_u64 param_req_size;
tsim_u64 param_random_req_size;
tsim_u64 param_bimodal_req_size;
tsim_u64 param_bimodal_size1;
tsim_u64 param_bimodal_size2;
tsim_u64 param_bimodal_size1_flits;
tsim_u64 param_bimodal_size2_flits;
tsim_u64 param_baseline_channel_width;
tsim_u64 param_scaled_bisection_width;
tsim_u64 param_bisection_channel_count;
tsim_u64 param_bisection_channel_width;

tsim_u64 param_fgen_rate;

tsim_u64 param_show_body_flit_log;

/* brg */
tsim_u64 param_wire_delay;
tsim_u64 param_hops_per_cycle;
tsim_u64 param_router_pipeline_latency;

tsim_u64 param_max_expr_hop;

tsim_u64 param_flit_max;

tsim_u64 param_inj_vc_count;
tsim_u64 param_ej_vc_count;

tsim_u64 param_incr_stats;
tsim_u64 param_incr_chkpt;
tsim_u64 param_chkpt_interval;

tsim_u64 param_stats_interval;
tsim_u64 param_node_stats;

tsim_u64 param_cost_reg_history;
tsim_u64 param_cost_hop_history;

tsim_u64 param_vis_on;
tsim_u64 param_vis_start;
tsim_u64 param_vis_stop;

tsim_u64 param_warmup_cycles;
tsim_u64 param_max_packets;
tsim_u64 param_node_bw_stats;

tsim_u64 param_aggressive_vc_alloc;

tsim_u64 param_0delay_cost_msg_update;
tsim_u64 param_same_cycle_local_cost;
tsim_u64 param_adaptive_1avail;

tsim_u64 param_cost_multiplier_local;
tsim_u64 param_cost_multiplier_remote;
tsim_u64 param_use_max_quadrant_cost;

tsim_u64 param_cost_precision;

string param_edge_cost_fn;

tsim_u64 param_ignore_escape_vc;
tsim_u64 param_ignore_escape_vc_Ydir;
tsim_u64 param_ignore_escape_vc_Ydir_heuristic;

tsim_u64 param_low_bw_rca;
tsim_u64 param_low_bw_rca_latency;

tsim_u64 param_percent_inj_bw;
double param_adj_inj_bw;
unsigned param_avg_pkt_size;

string param_netcfg_file;

string param_tracefile_name;
string param_selfsim_trace1;
string param_selfsim_trace2;

tsim_u64 param_selfsim_inj;

string param_vis_fifo_type;
string param_vis_link_type;

stringstream *logstring;

tsim_u64 ocin_cycle_count;


/* return log of a number to the base 2 */
int log_base2(int n) {
  int power = 0;

  if (n <= 0 || (n & (n-1)) != 0) {
    cerr << "\nlog2() only works for positive power of two values\n";
    exit(0);
  }

  while (n >>= 1)
    power++;

  return power;
}

// int biased_bool_random(int i) {
//   int x;
//   x =  random() % 100;
//   if (x < i) return 1;
//   return 0;
// }

// int biased_bool_random_base(int i, int max) {
//   int x;
//   x =  random() % max;
//   if (x < i) return 1;
//   return 0;
// }

string ocin_ftos ( float i )
{

   char temp[32];
   sprintf ( temp, "%.1f", i);
   return string( temp );
}

// TODO: looks like a typo - ftos3 or ftos2???
string ocin_ftos_3 ( float i )
{

   char temp[32];
   sprintf ( temp, "%.2f", i);
   return string( temp );
}

string ocin_ftos_5 ( float i )
{

   char temp[32];
   sprintf ( temp, "%.5f", i);
   return string( temp );
}

