#ifndef OCIN_ROUTER_VC_FIFO_H_
#define OCIN_ROUTER_VC_FIFO_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_router.h"
#include "ocin_mon_vc_fifo.h"
#include <deque>

class ocin_router;
class ocin_router_ou;
class ocin_mon_vc_fifo;

/* Individual Virtual Channel FIFO */
class ocin_router_vc_fifo {
  public:
    /* VC info */
	string name;
	ocin_router *parent_router;
	
	/* VC identifier: class, index, ID */
	int vc_id;                 // unique VC id on this router
	int vc_port;               // physical input port this VC belongs to
	int vc_index;              // index of this VC on the given port & class
	//int vc_class;              // priority class
	/* Logical VC identifier (for supporting BECs) */
	int vc_lport;              // logical input port this VC belongs to
	int vc_lindex;             // index of this VC on the given lport & class

	/* Data & control */
	deque <ocin_flit *> data;
	int credits;               // equival to the number of free flit buffers
	bool empty;                // no flits in fifo
	bool needs_vc;             // header flit at the front of the FIFO w/o an allocated VC
	bool has_vc;               // has output VC been allocated?
	bool needs_xb;             // flit at the front of the fifo needs the x-bar 
	bool avail;                // true if a *new pkt* can be accepted.
	                           //   this can mean 1 of 2 things:
	                           //   a) VC is idle (no flits buffered)
	                           //   b) EOM has been received, but not forwarded
	                           // Whether (b) is supported depends on the implementation.
	                           // Note, however, that in case (b), VC can be "available"
	                           // by virtue of having received its EOM, but not have credits.
	
	/* Routing info for this msg */
	int p_out;                 // output port for this msg
	int vc_out_index;		   // dest VC at p_out
	
	/* Misc */
	ocin_router_ou *credit_port;  // port on which upstream-bound credits for this VC will depart
	 
	/* Monitor */
	ocin_mon_vc_fifo *mon;
	  
	ocin_router_vc_fifo() {};
	void init(ocin_router *parent, 
	          const string &_name,
	          int buffers, 
	          int vc_id,
	          bool is_injector,
	          bool is_ejector);
	void enque(ocin_flit *flin_in);
	ocin_flit* deque();
        void update_fifo_head();
	// helper functions
	inline bool is_full()  { return (data.size()==_buf_size); };
	inline bool is_empty() { return (data.size()==0); };
	inline bool is_avail() { return (avail && (!is_full()) ); };
	inline ocin_msg  * front_msg()  { return data.front()->msg; };  // called must ensure data is not empty
	inline ocin_flit * front_flit() { return data.front(); };      // called must ensure data is not empty
    string print();
    // TODO: decouple xbar request from VC allocation for speculative X-bar arbiters
    inline bool needs_xbar()        { return (has_vc && needs_xb); }; 

  private:
    int _buf_size;             // equal to the max # of credits
    int _local_port;           // port number of the ejector 
    bool _is_ejector;          // true if this FIFO is in the ejector
    bool _is_injector;         // true if this FIFO is an injector
    
};

#endif /*OCIN_ROUTER_VC_FIFO_H_*/
