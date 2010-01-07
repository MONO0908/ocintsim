#ifndef OCIN_EJ_H_
#define OCIN_EJ_H_

#include "ocin_defs.h"
#include "ocin_router.h"
#include "ocin_interface.h"
#include "ocin_mon_packet.h"
#include "ocin_top.h"
#include "ocin_helper.h"
#include "ocin_gen.h"
#include "ocin_node_info.h"
#include <vector>
#include <map>

class ocin_ej : public tsim_module {
 public:

  /* pointer to ocin_top */
  ocin_top *top_p;

  int msg_cnt;  // track the # of received messages


  /* This is a reference to the top level node map */
  map<string, ocin_router> &nodes;
  map<string, ocin_router>::iterator n_it; /* iterator for
                                              nodes */
  
  map<string, ocin_node_info> & node_inst_data; /* Ref of map of info
                                                   read from netcfg
                                                   file.  Key is the
                                                   name of the node. */

  /* This is a ref to the top level map of all the outstanding
     messages, indexed by pid */
  map<int, ocin_msg*> & msg_map;


  /* We will construct a vector of pointers to the ejector units in
     the nodes to reduce the overhead of walking the map every
     cycle */
  vector<ocin_router_ej *> rt_ej_vec;
  vector<ocin_router_ej *>::iterator re_it;

  string name;

  ocin_ej(map<string, ocin_router> &_nodes, 
          map<int, ocin_msg*> & _msg_map,
          map<string, ocin_node_info> &_node_inst_data) 
    : nodes(_nodes), msg_map(_msg_map), node_inst_data(_node_inst_data) {};
  
  virtual void init (tsim_object *parent, string _name)=0;

};

#endif /* OCIN_EJ_H */
