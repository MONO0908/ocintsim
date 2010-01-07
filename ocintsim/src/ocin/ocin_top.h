#ifndef OCIN_TOP_H
#define OCIN_TOP_H

#include "ocin_router.h"
#include "ocin_defs.h"
#include "ocin_channel.h"
#include "ocin_node_info.h"
#include "ocin_gen.h"
#include "ocin_gen_rand.h"
#include "ocin_gen_modeled.h"
#include "ocin_gen_bitcomp.h"
#include "ocin_gen_bitrev.h"
#include "ocin_gen_randpair.h"
#include "ocin_gen_hotspot.h"
#include "ocin_gen_transpose.h"
#include "ocin_gen_selfsim.h"
#include "ocin_gen_file.h"
#include "ocin_gen_filetm.h"
#include "ocin_ej.h"
#include "ocin_ej_noblock.h"
#include "ocin_interface.h"
//#include "ocin_vis.h"
#include "ocin_prtr.h"
#include "ocin_prtr_xy.h"
#include "ocin_prtr_omni.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <string>

class ocin_ej;

class ocin_top : public tsim_module {
public:	
  string modname;
	
   ocin_top() {};
   void init (tsim_object *parent, const string &name);	
   void get_params ();
   void parse_netcfg();

   void create_generator();
   void create_tiles();
   void create_internal_wires ();
   void create_ejector();
   void create_vis();

   void evaluate ();
   void update ();
   void finalize();

   void print_config();
   void print_stats(bool totals);
   
   void checkpoint_monitors();
   void deadlock_check();
   
   void set_network_size();
   void set_bandwidth();
   int bisection_channel_count();  // compute the bisection channel count for a given topology
   void set_concentration();       // setup concentration

   map<string, ocin_router> nodes; /* map of network nodes (a node may
                                    be a router or an injection-
                                    ejection port).  Key is the name
                                    of the node*/

   map<string, ocin_router>::iterator nodes_it; /* iterator for
                                                nodes */

   map<string, ocin_node_info> node_inst_data; /* Map of info read
                                                  from netcfg file.
                                                  Key is the name of
                                                  the node. */

   map<string, ocin_node_info>::iterator nid_it; /* iterator for
                                                    node_isnt_data */
   
   map<string, string> terminal2node_map;  /* map a terminal to a network node */
   map<string, int> terminal2port_map;     /* map a terminal to a local port at a network node */

   vector<ocin_channel*> wires; /* vector of wires to connect nodes*/
   vector<ocin_channel*>::iterator w_it; /* wire iterator*/

   map<string, ocin_prtr *> prertrs; /* map of pointers to the
                                        prerouters keyed on prerouter
                                        location node */

   map<string, ocin_gen *> gens;
   map<string, ocin_gen *>::iterator gen_it;
   set<string> gen_types;

   vector<ocin_ej *> ejectors;
   set<string> ejc_types;

   map<int, ocin_msg*> msg_map; /* map of the outstanding
                                   transactions */

   vector<string> node_names;   /* used to access various maps in the
                                   same order each time */

   //ocin_vis * vis_ptr;		/* pointer to the visualizer object */
   bool vis_all;		/* flag that means no start and
				   stop */
};

#endif


