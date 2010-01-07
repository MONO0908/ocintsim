#ifndef OCIN_PRTR_XY_H_
#define OCIN_PRTR_XY_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_top.h"
#include "ocin_helper.h"
#include "ocin_prtr.h"
#include <vector>
#include <map>

class ocin_prtr_xy : public ocin_prtr {
public:
  
  ocin_prtr_xy( map<string, ocin_node_info> &_node_inst_data, 
                map<string, ocin_router> &_nodes)
    : ocin_prtr(_node_inst_data, _nodes) {}; 
  
  void init (tsim_object *parent, string _name);

  void preroute_msg(ocin_msg * msg, string src, string dest);

};

#endif /* OCIN_PRTR_XY_H */
