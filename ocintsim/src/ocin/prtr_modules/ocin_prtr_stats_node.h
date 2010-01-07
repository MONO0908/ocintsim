#ifndef OCIN_PRTR_STATS_NODE_H_
#define OCIN_PRTR_STATS_NODE_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_helper.h"
#include "ocin_router.h"
#include <vector>
#include <map>
#include <deque>




class ocin_prtr_stats_node {
public:
  string name;
  vector <ocin_prtr_stats_node *> edges; /* ptrs to other nodes */
  vector <int> weights;                  /* weights for the edges to
                                            other nodes */
  ocin_router * rt_ptr;         /* ptr to router (used to get the
                                   weight from the cost_reg) */
  
  void update();                /* grab value of cost reg and put it
                                   in weight */

  int distance;                     /* current best sum through this
                                       node */

  inline void reset() { distance = 0x0fffffff; rt_est.clear();}; /* set node up for route
                                                algo */

  deque <int> rt_est;          /* current route estimate to this
                                   node */


};

//bool comp_node (ocin_prtr_stats_node * a, ocin_prtr_stats_node * b);


#endif /* OCIN_PRTR_STATS_NODE_H */
