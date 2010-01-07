#ifndef OCIN_ROUTER_EJ_H_
#define OCIN_ROUTER_EJ_H_

#include "ocin_router.h"

class ocin_router;
class ocin_router_vc_fifo;

/* Output unit.
 * See .cpp file for details 
 */
class ocin_router_ej {
  public:
	string name;
	ocin_router *parent_router;
	ocin_channel *channel_out;
	vector <ocin_vc_status> &ej_vc_stats;  // VC credits @ ejection port
	deque <int> &free_vc_list;                     // idx of VCs @ eject port that can be alloc'd 
	
  	vector <ocin_router_vc_fifo> &ej_fifos;    // VC fifos associated w/ this input port
	
	ocin_router_ej(vector <ocin_vc_status> &downstream_vc_stats, 
	               deque <int> &free_vc_list,
	               vector <ocin_router_vc_fifo> &eject_fifos) 
	                  : ej_vc_stats(downstream_vc_stats),
	                    free_vc_list(free_vc_list), 
	                    ej_fifos(eject_fifos)              {};
   	void init(ocin_router *parent, const string &_name);
   	void enque(ocin_flit *flit);
//   	void enque(ocin_credit *credit);
   	ocin_flit * deque(int vc_idx);
   	// transmit() is used by "real" output ports - not by the ejection port
   	//void transmit();
  
  private:
};

#endif /*OCIN_ROUTER_EJ_H_*/
