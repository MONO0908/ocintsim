#ifndef OCIN_PRTR_H_
#define OCIN_PRTR_H_

#include "ocin_defs.h"
#include "ocin_router.h"
#include "ocin_interface.h"
#include "ocin_mon_packet.h"
//#include "ocin_top.h"
#include "ocin_helper.h"
#include "ocin_node_info.h"
#include <vector>
#include <map>

class ocin_top;

class ocin_prtr : public tsim_module {
 public:

  /* pointer to ocin_top */
  ocin_top *top_p;

  string name;

  map<string, ocin_node_info> & node_inst_data; /* Map of info read
                                                 from netcfg file.
                                                 Key is the name of
                                                 the node. */

  map<string, ocin_router> & nodes; /* map of network nodes (a node
                                       may be a router or an
                                       injection- ejection port).  Key
                                       is the name of the node*/


  /* on initialization set up refs */
  ocin_prtr(map<string, ocin_node_info> &_node_inst_data, 
            map<string, ocin_router> &_nodes)
    : node_inst_data(_node_inst_data), nodes(_nodes){};


  virtual void init (tsim_object *parent, string _name)=0;

  virtual void preroute_msg(ocin_msg * msg, string src, string dest)=0;


};

#endif /* OCIN_PRTR_H */
