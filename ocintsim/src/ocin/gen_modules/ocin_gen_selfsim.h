#ifndef OCIN_GEN_SELFSIM_H_
#define OCIN_GEN_SELFSIM_H_

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

class ocin_top;
class ocin_router_inj;

class ocin_gen_selfsim : public ocin_gen {
 public:

  /* pointer to ocin_top */
  ocin_top *top_p;

  /* vector of pointers to local_iu's: */
  vector<ocin_router_inj *> local_iu_ptrs;

  /* initialize the refs on instantiation */
  ocin_gen_selfsim(map<string, ocin_router> &_nodes, 
                   map<int, ocin_msg*> &_msg_map, 
                   map<string, ocin_node_info> &_node_inst_data,
                   map<string, ocin_prtr *> &_prertrs)
    : ocin_gen(_nodes, _msg_map, _node_inst_data, _prertrs) {}; 

  void init (tsim_object *parent, string _name);

  /* number of packets to be generated chosen here, generated w/ calls
     to gen_packet and pushed onto the appropriate node's injector */
  void update();

  /* nothing in finalize */
  void finalize();

};

#endif /* OCIN_GEN_SELFSIM_H */
