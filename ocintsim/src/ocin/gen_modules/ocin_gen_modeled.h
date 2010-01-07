#ifndef OCIN_GEN_MODELED_H_
#define OCIN_GEN_MODELED_H_

#include "ocin_defs.h"
#include "ocin_router.h"
#include "ocin_interface.h"
#include "ocin_mon_packet.h"
#include "ocin_helper.h"
#include "ocin_gen.h"
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

class ocin_top;
class ocin_router_inj;

class ocin_gen_modeled : public ocin_gen {
 public:

  /* Stuff for gsl random distribution */
  const gsl_rng_type * T;
  gsl_rng * r;

  /* pointer to ocin_top */
  ocin_top *top_p;

  /* vector of pointers to local_iu's: */
  vector<ocin_router_inj *> local_iu_ptrs;

  map<string, double> src_rate;
  map<string, double> dst_rate;

  double dst_cum_tot;

  /* initialize the refs on instantiation */
  ocin_gen_modeled(map<string, ocin_router> &_nodes, 
                   map<int, ocin_msg*> &_msg_map, 
                   map<string, ocin_node_info> &_node_inst_data,
                   map<string, ocin_prtr *> &_prertrs)
    : ocin_gen(_nodes, _msg_map, _node_inst_data, _prertrs) {}; 

  void init (tsim_object *parent, string _name);

  /* number of packets to be generated chosen here, generated w/ calls
     to gen_packet and pushed onto the appropriate node's injector */
  void update();

  void set_probabilities();     /* sets values in per-source and
                                   per-dest hashes of injection
                                   probabilities */


  /* nothing in finalize */
  void finalize();

};

#endif /* OCIN_GEN_MODELED_H */
