#ifndef OCIN_GEN_FILE_H_
#define OCIN_GEN_FILE_H_

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
#include <zlib.h>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

class ocin_top;
class ocin_router_inj;

/* here we assume that the L1 has 64 byte cache lines and the L2 has
   128 byte cache lines */
#define L1OFFSET 6
#define L2OFFSET 7

class ocin_gen_file : public ocin_gen {
 public:

  /* Some local vars associated w/ file reading */
  bool is_compressed;
  bool is_multipart;
  bool ocn_proper;
  bool splash;
  int filepart;
  bool opn;                     /* opn file type */
  tsim_u64 last_inj_cycle;      /* used to detect rollovers */
  tsim_u64 high_inj_cycle;      /* used to fix rollovers */
  tsim_u64 cycle_offset;        /* used to throttle by offsetting
                                   injection cycle */

  int max_x, max_y;             /* used when spreading src/dst
                                   pairs */
  int srcymask, dstymask, dstxmask, dstxshift;

  int filegen_num;

  FILE *patt_file;
  gzFile gzpatt_file;

  /* pointer to ocin_top */
  ocin_top *top_p;

  /* vector of pointers to local_iu's: */
  vector<ocin_router_inj *> local_iu_ptrs;
  map <string, ocin_router_inj *> local_iu_ptrs_map; 

  /* convert term id to coordinates */
  vector <vector <int> > termid2coords;

  /* initialize the refs on instantiation */
  ocin_gen_file(map<string, ocin_router> &_nodes, 
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

  /* functions for file reading and record list maintenance*/
  void openfile();
  void closefile();
  ocin_gen_filerec * get_record();
  ocin_gen_filerec * readfile();
  void pull_records();
  void ff_file();


  /* list of records to be injected */
  list <ocin_gen_filerec *> curr_recs;
  list <ocin_gen_filerec *>::iterator cr_it;

  /* this is a list of the MT replies, pre-created at req gen
     time. pull_records combines the two lists together as needed. */
  vector <ocin_gen_filerec *> curr_reps;
  vector <ocin_gen_filerec *>::iterator cp_it;
  

  bool filecomplete;            /* flag to keep track of when the
                                   current file is done */

  /* our own version of the gen_packet so we can handle the specifics
     of the record's packet */
  void gen_packet(ocin_gen_filerec * filerec_ptr);

  void get_dst_translation(ocin_gen_filerec * filerec_ptr); 
  /* translate from addr to dst (OCN packets) */

  void spread_translation(ocin_gen_filerec * filerec_ptr);


  unsigned int trans_table[64]; /* used to translate OCN reqs */


};

#endif /* OCIN_GEN_FILE_H */
