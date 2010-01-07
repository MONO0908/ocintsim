#ifndef OCIN_MON_NODE_H
#define OCIN_MON_NODE_H

#include "ocin_defs.h"
#include <map>
#include <vector>
#include <string>

class ocin_node_info;

class ocin_mon_node {

 public:
  
  string name;                  /* this node */
  
  // This map keeps track of how many flits have been injected from
  // this router to each other router.  It is used to figure out node
  // level BW numbers
  map<string, tsim_u64> rtr_inj_flits;
  map<string, tsim_u64> chpt_rtr_inj_flits;

  // This map keeps track of how many flits have been accepted at the
  // destination from this router.  Used to calc accepted rate per node
  map<string, tsim_u64> rtr_comp_flits;
  map<string, tsim_u64> chpt_rtr_comp_flits;

  // Used to keep track of the cycle that each router receives its
  // first non-monitored packet from this router.
  map<string, tsim_u64> rtr_end_cycle;
  
  void clear();
  ocin_mon_node() { clear(); };
 
  void init(vector<string> &node_names, string _name);
  void checkpoint();

  void print(vector<string> &node_names, bool header, 
             bool acc_off, bool totals, tsim_u64 inj_end, 
             tsim_u64 chpt_inj_end);


};

#endif

