#ifndef OCIN_GEN_H_
#define OCIN_GEN_H_

#include "ocin_defs.h"
#include "ocin_router.h"
#include "ocin_interface.h"
#include "ocin_mon_packet.h"
//#include "ocin_top.h"
#include "ocin_prtr.h"
#include "ocin_helper.h"
#include <vector>
#include <map>

class ocin_node_info;

class ocin_gen : public tsim_module {
 public:

  static bool gen_done;                // true when all packets inserted
  // note: currently using a static for this to save sim performance
  // in ocin_top.  If we might have more than one file gen then we
  // probably want this to be non-static and iterate over generators
  // to determine gen_done state...

  bool throttling;              /* flag for when packet injection is
                                   too high for simulation */

  /* This is a reference to the top level node map */
  map<string, ocin_router> & nodes;

  /* This is a ref to the top level map of all the outstanding
     messages, indexed by pid */
  map<int, ocin_msg*> & msg_map;

  map<string, ocin_node_info> & node_inst_data; /* Ref of map of info
                                                  read from netcfg
                                                  file.  Key is the
                                                  name of the node. */

  map<string, ocin_prtr *> & prertrs; /* map of pointers to the
                                       prerouters keyed on prerouter
                                       location node */

  map<string, string> terminal2node_map;  /* map a terminal to a network node */
  map<string, int> terminal2port_map;     /* map a terminal to a local port at a network node */

  /* Packet stats monitor */
  ocin_mon_packet * mon;

  /* Vector of legal source nodes */
  vector<string> srcs;

  /* Vector of legal desitnation nodes */
  vector<string> dsts;

  /* current pid */
  static long long pid;
  static long long temp_pid;  // for the warm-up and cool-down periods

  string name;

  /* on initialization set up refs */
  ocin_gen(map<string, ocin_router> &_nodes, 
           map<int, ocin_msg*> &_msg_map, 
           map<string, ocin_node_info> &_node_inst_data, 
           map<string, ocin_prtr *> &_prertrs)
    : nodes(_nodes), msg_map(_msg_map), node_inst_data(_node_inst_data), prertrs(_prertrs) 
  {pid = 0; temp_pid = -1; rolls = taken = 0;};

  virtual void init (tsim_object *parent, string _name)=0;


  /* Various functions to be used in a decendants */
  void checkpoint();  /* function to checkpoint vars */

  /* function generates actual random packet */
  ocin_msg * gen_packet(string src_term, string dst_term);

  /* for generating list of legal source and dest nodes */
  void push_src(string src);
  void push_dst(string dst);

  /* These are for self sim and random inj process */
  double gen_rand(int flag);
  void openfile(int fileno);
  ifstream selfsim_file1;
  ifstream selfsim_file2;

  int rolls;
  int taken;

  void update_gen_done();       /* used to determine if we've hit the
                                   max_packets */


};

#endif /* OCIN_GEN_H */
