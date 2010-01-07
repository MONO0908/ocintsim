#include "ocin_prtr_stats_node.h"


void ocin_prtr_stats_node::update() {
  for (int x =0; x < edges.size(); x++) {
    if (edges[x] == NULL) {
      continue;
    }

    rt_ptr->cost_regs[0][x].update();
    weights[x] = (int)(((double)rt_ptr->cost_regs[0][x].get_hist_sum())*((double)param_preroute_scale/100.0)) + param_preroute_baseline;

    //    cout << ocin_cycle_count << " Got weight " << weights[x]<< endl;
  }
}
