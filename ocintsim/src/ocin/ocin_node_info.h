#ifndef OCIN_NODE_INFO_H
#define OCIN_NODE_INFO_H

#include <string>
#include <vector>
#include "ocin_mon_node.h"

class ocin_channel;
class ocin_router_iu;

class ocin_node_info {

 public:

  // Instantiation information
  string name;                  /* name of router, used as key to
                                   node_info map */

  vector <int> coord;           /* router's multi-dimensional coordinates */

  bool is_source;               /* Does this node inject packets */

  bool is_destination;          /* Does this node receive packets */

  tsim_u64 clock_period;              /* clock period */

  vector <string> src_type;     /* type of sources this has attached */

  string dst_type;              /* type of dest this is */

  string router_type;           /* router shell model (abstraction
                                   level and number of pipe stages) */
                                   
  // Terminals: src/dest entities connected to this network node.
  //            Used to model concentration.
  int terminals;
  vector <string> terminal_names;             // terminal names
  vector <vector <int> > terminal_coords;     // terminal coords

  // Used by router init: 
  string xbar_type;             /* Xbar design */
  string vc_alloc;              /* VC allocator type */
  string xb_alloc;              /* Xbar allocator type */
  string preroute_type;         /* Preroute algorithm */
  string rt_algo_type;          /* Routing algorithm */
  string rt_sel_type;           /* Selector function for choosing the
                                   output port (used in adaptive routing) */
  string rt_cost_fn;            /* cost function used in computing link cost.
                                   eg: local, omniscient */
  string rt_cost_reg;           /* local monitor used in computing the
                                   cost of an output port */
  string rt_cost_mgr;           /* manages aggregation & propagation of 
                                   congestion info */
  int vc_count;                 /* number of VCs per port */
  int vc_classes;               /* number of priority classes to
                                   subdivide VCs up amongst */
  int que_depth;                /* FIFO queue depth per VC (we can
                                   extend this to an array if we want
                                   different depths per VC) */

  // Connection information:
  int port_count;               /* number of ports */
  vector<string> port_dest;     /* name of the entity the port is
                                   connected to. 
                                */

  vector<string>::iterator pt_it; /* Iteratetor for port_dest */

  vector<ocin_channel*> input_links; /* pointers to input ports */
  vector<ocin_channel*> output_links; /* pointers to output ports */

  /* Boris's requested link to next hop's input ports */
  //  vector <ocin_router_iu *> next_hop_input_ports; (not in use anymore)
  
  // Connectivity for EC's and Bypassed ECs
  vector <int> phys_ports; 		// physical ports 
  vector <int> logical_ports;	// same as physical for ECs.
  								//  For BECs, group multiple channels 
  								//  under one output port.


  //  vector<string> wire_type;   may be used to identify the type of
                                /*wire for each port */


  // Router level statistical counts: 
  ocin_mon_node stats;

  ocin_node_info() {};

  /* copy constructor */
  ocin_node_info(const ocin_node_info &org) {
    name = org.name;
    /* dont copy coord */
    is_source = org.is_source;
    is_destination = org.is_destination;
    src_type = org.src_type;
    router_type = org.router_type;
    xbar_type = org.xbar_type;
    vc_alloc = org.vc_alloc;
    xb_alloc = org.xb_alloc;
    preroute_type = org.preroute_type;
    rt_algo_type = org.rt_algo_type;
    rt_sel_type = org.rt_sel_type; 
    rt_cost_fn = org.rt_cost_fn;
    vc_count = org.vc_count;
    vc_classes = org.vc_classes;
    que_depth = org.que_depth;
    port_count = org.port_count;
    /* dont copy port_dest, pt_it or links they must be defined for
       each individually */

  }

};

#endif
