#ifndef OCIN_ROUTER_IU_H_
#define OCIN_ROUTER_IU_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_router.h"

class ocin_router;
class ocin_router_vc_fifo;
class ocin_router_ru;
class ocin_router_cost_mgr;

/* Input Unit
 * See the cpp file for a description.
 */
 
 class ocin_router_iu {
   public: 
     string name;
     ocin_router *parent_router;
     ocin_channel *channel_in;
     vector <int> local_addr;

     int _port_num;
     vector <ocin_router_vc_fifo> &_input_fifos;     // VC fifos associated w/ this input port
   
     ocin_router_iu(vector <ocin_vc_status> &vc_stats, 
                    deque <int> &free_vc_list,
     	            vector <ocin_router_vc_fifo> &input_fifos,
     	            ocin_router_cost_mgr *cost_mgr)
                                      : _downstream_vc_stats(vc_stats),
                                        _free_vc_list(free_vc_list), 
                                        _input_fifos(input_fifos),
                                        _cost_mgr(cost_mgr)       {};
     void init(ocin_router *parent, 
               const string &_name, 
               int port_num,
               ocin_channel *c_in,
               ocin_router_ru *ru );
     void receive();
     void repreroute();

   private:
     ocin_router_ru *_ru;
     vector <ocin_vc_status> &_downstream_vc_stats;  // VC credits @ downstream node
     deque <int> &_free_vc_list;                     // idx of downstream VCs that can be alloc'ed  
     ocin_router_cost_mgr *_cost_mgr;                // cost manager
 };

#endif /*OCIN_ROUTER_IU_H_*/
