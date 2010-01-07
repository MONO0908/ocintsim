#ifndef OCIN_CREDIT_H_
#define OCIN_CREDIT_H_

#include "tsim.h"
#include "ocin_defs.h"
#include "ocin_helper.h"
#include "ocin_interface.h"
#include <vector>

/* Credit:
 * Used to signal buffer availability to upstream routers
 */
class ocin_credit {
  public:
	int credits;              // increment amount (*NOT* an absolute number)
	int vc;                   // VC fifo which generated the credit update               
    tsim_u64 arrival_time;    // Used by receiver on the wire. Models the wire delay 
	
	ocin_credit() {};
	ocin_credit(int cr, int v) : credits(cr), vc(v) {}; 
};

/* VC status:
 * Used to keep track of credits & VC availability at downstream nodes
 */
class ocin_vc_status {
  public:
    string name;
    int credits;
    int vc_idx;
    int vc_class;
    bool avail;
    deque <int> *free_vc_list;
    int max_credits;  // buffer size
    
    int cost;  // used by the predictive adaptive routing fn.
               // reflects availability AND # of flits remaining for tx.
    
    ocin_vc_status() {};
    ocin_vc_status(int cr, int vc, bool av, deque <int> *free_list) :
                                 credits(cr), vc_idx(vc), avail(av), free_vc_list(free_list) {};
                              //   { credits = _cr; vc = _vc; avail = _av; }; 
    void init(const string &_name);
    void inc_credits();
    void dec_credits(bool eom);  // eom = end of message
    void allocate(ocin_msg  *msg); 
    // This is a dangerous function, since its meaning is ambiguous
    //inline bool is_avail() { return ((avail && (credits > 0)) ? true : false); };
    inline bool is_avail() { return (avail && 
		                                ( (!param_aggressive_vc_alloc && (credits==max_credits)) ||
		                                  ( param_aggressive_vc_alloc && (credits > 0)))); };

  private:
    
};

#endif /*OCIN_CREDIT_H_*/
