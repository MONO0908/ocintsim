#ifndef OCIN_ROUTER_H
#define OCIN_ROUTER_H

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_credit.h"
#include "ocin_cost_msg.h"
#include "ocin_channel.h"
#include "ocin_router_inj.h"
#include "ocin_router_ej.h"
#include "ocin_router_iu.h"
#include "ocin_router_ou.h"
#include "ocin_router_vc_fifo.h"
#include "ocin_router_ru.h"
#include "ocin_router_ru_xydor.h"
#include "ocin_router_ru_preroute.h"
#include "ocin_router_ru_yxdor.h"
#include "ocin_router_ru_adaptive_xy.h"
#include "ocin_router_ru_xydor_bec.h"
#include "ocin_router_ru_o1turn_bec.h"
#include "ocin_router_sel.h"
#include "ocin_router_sel_1match.h"
#include "ocin_router_sel_lastmatch.h"
#include "ocin_router_sel_bec.h"
#include "ocin_router_sel_o1turn.h"
#include "ocin_router_sel_1avail.h"
#include "ocin_router_sel_noturn.h"
#include "ocin_router_sel_local.h"
#include "ocin_router_sel_stat.h"
#include "ocin_router_sel_random.h"
#include "ocin_router_cost_reg.h"
#include "ocin_router_cost_mgr.h"
#include "ocin_router_cost_mgr_local.h"
#include "ocin_router_cost_mgr_1D.h"
#include "ocin_router_cost_mgr_fanin.h"
#include "ocin_router_cost_mgr_quadrant.h"
#include "ocin_router_cost_mgr_1domni.h"
#include "ocin_router_cost.h"
#include "ocin_router_vcalloc.h"
#include "ocin_router_vcalloc_2level.h"
#include "ocin_router_xballoc.h"
#include "ocin_router_xballoc_2level.h"
#include "ocin_router_xballoc_speculative_2level.h"
#include "ocin_router_xballoc_bec_2level.h"
#include "ocin_router_xballoc_bec_power_2level.h"
#include "ocin_router_xballoc_bec_spec_2level.h"
#include "ocin_router_xbar.h"
#include "ocin_mon_port.h"
#include "ocin_mon_vc_fifo.h"
#include "ocin_node_info.h"
#include "ocin_helper.h"
#include "ocin_prtr.h"


using namespace std;

class ocin_top;
class ocin_router_inj;
class ocin_router_ej;
class ocin_router_iu;
class ocin_router_ou;
class ocin_router_vc_fifo;
class ocin_router_ru;
class ocin_router_ru_xydor;
class ocin_router_ru_preroute;
class ocin_router_ru_yxdor;
class ocin_router_ru_adaptive_xy;
class ocin_router_ru_xydor_bec;
class ocin_router_ru_o1turn_bec;
class ocin_router_sel;
class ocin_router_sel_1match;
class ocin_router_sel_lastmatch;
class ocin_router_sel_bec;
class ocin_router_sel_o1turn;
class ocin_router_sel_1avail;
class ocin_router_sel_noturn;
class ocin_router_sel_local;
class ocin_router_sel_stat;
class ocin_router_sel_random;
class ocin_router_cost_reg;
class ocin_router_cost_mgr;
class ocin_router_cost_mgr_local;
class ocin_router_cost_mgr_1D;
class ocin_router_cost_mgr_fanin;
class ocin_router_cost_mgr_quadrant;
class ocin_router_cost;
class ocin_router_vcalloc;
class ocin_router_vcalloc_2level;
class ocin_router_xballoc;
class ocin_router_xballoc_2level;
class ocin_router_xballoc_speculative_2level;
class ocin_router_xballoc_bec_spec_2level;
class ocin_router_xballoc_bec_power_2level;
class ocin_router_xballoc_bec_2level; 
class ocin_router_xbar;
class ocin_mon_port;
class ocin_mon_vc_fifo;

class ocin_router : public tsim_module {

public: 


  map<string, ocin_router> *nodes; /* used by const_mgr */

  ocin_node_info *node_info;    // node descriptor
  int ocin_router_id;
  ocin_top    *top_p; 

  ocin_prtr *prtr_ptr;          /* prerouter */
  
  /* injection unit */
  deque <ocin_router_inj> local_iu;
  /* ejection unit */
  deque <ocin_router_ej> local_ou;
  
  /* input & output units */
  // vector would do here, but its constructor barfs when I pass a list in
  deque <ocin_router_iu> input_units;
  deque <ocin_router_ou> output_units; 
  
  /* routing units */
  ocin_router_ru *routing_units;  // @ each input port
  ocin_router_ru *inj_ru;         // @ inj port
  
  /* route selection, congestion management */
  vector <vector <ocin_router_cost_reg> > cost_regs;
  ocin_router_cost_mgr *cost_mgr;
  ocin_router_sel *route_sel;
  ocin_router_cost *route_cost;
  
  /* VC fifos */
  // vc_fifos[port][vc_idx]
  vector <vector <ocin_router_vc_fifo> > vc_fifos;
  // injection fifos - appended to network fifos (vc_fifos)
  int first_inj_fifo_idx;  // index into vc_fifos corresponding to 1st inj fifo
  // ejection fifos - not part of vc_fifos
  vector <vector <ocin_router_vc_fifo> > ej_fifos;
  
  // Logical ports
  vector <vector <ocin_router_vc_fifo *> > lvc_fifos;  // pointers to VCs via logical ports
  
  // mappings from vc_id to input port & priority class
  vector <int> vcid2port;
  vector <int> vcid2class;
  vector <int> vcid2vcidx;
  vector <int> vcid2lport;
  vector <int> vcid2lvcidx;
  vector <ocin_router_vc_fifo *> vcid2vc;   // pointers to fifos sorted by vc_id
  vector <int> port2lport;
  int first_ej_idx;      // index of the first ejector fifo in the vectors above
  
  
  /* VC credits downstream */
  vector <vector <ocin_vc_status> > downstream_vc_stats;
  vector <deque <int> > free_vc_list;   // index of downstream VCs that can be allocated
  
  /* VC allocator */
  ocin_router_vcalloc *vc_allocator;
  
  /* X-bar Allocator */
  ocin_router_xballoc *xb_allocator;
  
  /* X-bar */
  ocin_router_xbar *xbar;
  
  /* Port monitors */
  vector <vector <ocin_mon_port> > pin_mon;
  vector <vector <ocin_mon_port> > pout_mon;

  void init (tsim_object *parent, ocin_node_info *node_dsc);
  void number_vcs();
  void create_lports(); // group VCs into logical ports 
  void select_output_ports();
  void spec_vcxb_alloc_finalize();
  
  void stop() { this->sim()->stop(); };
  
  void evaluate();
  void update();
  void finalize();
  

  int _terminals;         // # of local ports
  int _ports;             // # of network ports
  bool spec_xb_alloc;     // speculative XB alloc?
  bool sequential_alloc;  // sequential VC-XB allocation (1 cycle each)
  

};

#endif
