#ifndef OCIN_PRTR_STATS_STR_H_
#define OCIN_PRTR_STATS_STR_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_helper.h"
#include "ocin_node_info.h"
#include "ocin_prtr_stats_node.h"
#include "ocin_router.h"
#include <vector>
#include <map>
#include <deque>
#include <algorithm>

class ocin_prtr_stats_str {
public:
  string name;
  string last_src;
  tsim_u64 last_cycle;

  map <string, ocin_prtr_stats_node *> nodes;

  list <ocin_prtr_stats_node *> unvisited; // list of unvisited nodes

  // builds status structure from the top level node info
  void init(map<string, ocin_node_info> &node_inst_data, 
            map<string, ocin_router> &rtrs );

  void route(ocin_msg * msg, string src, string dest); // find optimal route
                                               // from src to dest.


  void print();                 // Print stats for debugging
  void print_route(string src, deque <int> route);

  void update();

  void reset();                 // reset all nodes

};

#endif /* OCIN_PRTR_STATS_STR_H */
