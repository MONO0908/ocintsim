#ifndef OCIN_EJ_NOBLOCK_H_
#define OCIN_EJ_NOBLOCK_H_

#include "ocin_defs.h"
#include "ocin_ej.h"
#include "ocin_router.h"
#include "ocin_interface.h"
#include "ocin_mon_packet.h"
#include "ocin_top.h"
#include "ocin_helper.h"
#include "ocin_gen.h"
#include <vector>
#include <map>

class ocin_ej_noblock : public ocin_ej {
 public:

  ocin_ej_noblock(map<string, ocin_router> &_nodes,          
                  map<int, ocin_msg*> & _msg_map,
                  map<string, ocin_node_info> &_node_inst_data) 
    : ocin_ej(_nodes, _msg_map, _node_inst_data) {};
  
  
  void init (tsim_object *parent, string _name);

  /* nothing in evaluate */
  void evaluate();

  void update();

  /* nothing in finalize */
  void finalize();

};

#endif /* OCIN_EJ_NOBLOCK_H */
