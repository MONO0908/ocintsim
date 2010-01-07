#ifndef OCIN_GEN_FILETM_H_
#define OCIN_GEN_FILETM_H_

#include "ocin_defs.h"
#include "ocin_router.h"
#include "ocin_mon_packet.h"
#include "ocin_interface.h"
#include "ocin_top.h"
#include "ocin_helper.h"
#include "ocin_gen.h"
#include "ocin_gen_filerec.h"
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

class ocin_top;
class ocin_router_inj;


class ocin_gen_filetm : public ocin_gen {
 public:
 
  /* pointer to ocin_top */
  ocin_top *top_p;

  /* vector of pointers to local_iu's: */
  vector<ocin_router_inj *> local_iu_ptrs;
  map <string, ocin_router_inj *> local_iu_ptrs_map; 

  /* initialize the refs on instantiation */
  ocin_gen_filetm(map<string, ocin_router> &_nodes, 
                  map<int, ocin_msg*> &_msg_map, 
                  map<string, ocin_node_info> &_node_inst_data,
                  map<string, ocin_prtr *> &_prertrs)
    : ocin_gen(_nodes, _msg_map, _node_inst_data, _prertrs) {}; 

  /* vector of the max in each coord */
  vector<int> max_coords;

  /* map of src to dest pairs */
  map<string, string> src_to_dst;
  
  
  ifstream traceFile;    // file pointer
  TraceFormat next_rec;  // next record to be processed
  bool eof_reached;      // EOF reached?

  void init (tsim_object *parent, string _name);

  /* functions for file reading and record list maintenance*/
  void openfile();
  void closefile();
  void inject_packet(TraceFormat *rec);
  
  void update();

  /* nothing in finalize */
  void finalize();
};


#endif /*OCIN_GEN_FILETM_H_*/
