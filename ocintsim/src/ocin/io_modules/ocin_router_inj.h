#ifndef OCIN_ROUTER_INJ_H_
#define OCIN_ROUTER_INJ_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_router.h"

class ocin_router;
class ocin_router_vc_fifo;
class ocin_router_ru;

class ocin_router_inj {
 public:
  string name;
  ocin_router *parent_router;
  deque <ocin_flit *> data;
  

  ocin_router_inj(vector <ocin_router_vc_fifo> &inj_fifos) : _inj_fifos(inj_fifos) {};

  void init(ocin_router *parent, 
            const string &_name,
            /*gen ptr,*/
            ocin_router_ru *ru);
  void receive();
  int find_free_vc();
  void repreroute();

  /* Adding method to inject flit to data queue - PG 3/28/07 */
  void push_flit(ocin_flit * flit);

  /* need to get rid of this method, wont be used, saving it for now
     becuase of the debugging stuff in there */
  void req_pkt();

 private:
  ocin_router_ru *_ru;
  vector <ocin_router_vc_fifo> &_inj_fifos;    // VC fifos associated w/ this input port
};

#endif /*OCIN_ROUTER_INJ_H_*/
