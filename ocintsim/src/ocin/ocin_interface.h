#ifndef OCIN_INTERFACE_H_
#define OCIN_INTERFACE_H_

#include "tsim.h"
#include "ocin_defs.h"
#include <vector>
#include <set>

class ocin_mon_packet;

/*
 * directions: {N, S, E, W, Local, Invalid}
 * Note: might not be a good idea to use cardinal directions 
 * in case we ever go to higher-order topologies.
 */ 
enum dir_t {
  INC = 0,
  DEC = 1
};


/* The following classes define flit & message structures.
 * In general, a message contains control (ie, routing info, error protection,
 * etc) and data, which is segmented into flits.
 */ 
class ocin_msg;
 
/* Flit class
 */ 
class ocin_flit {
 public: 
  ocin_msg *msg;
  int idx;                      // flit index in the msg.flits vector.
  int p_out;                    // output port at current hop
  int p_in;                     // input port at next hop (to support
                                // path sets & early eject)
  int vc_idx;                   // VC at port p_in
 	
  bool is_header;               // Is first flit in msg
  bool is_tail;                 // Is last flit in msg

  int flit_vect_idx;            // index into grand flit vector, saves
                                // time on deallocation.
  tsim_u64 arrival_time;        // Used by receiver on the wire. Models the wire delay

  // some stats
  tsim_u64 gen_time;            // cycle the flit was generated

  tsim_u64 inj_time;            // cycle the flit was injected to net

  tsim_u64 dst_time;            // cycle the flit arrived at the
                                // destination router

  vector <int> curr_addr; /* current resident location of this flit */


  ocin_flit() {};
  ocin_flit(ocin_msg *m) : msg(m) {};
  ocin_flit(ocin_msg *m, int index) : msg(m), idx(index) {};
};

/* Message class:
 * header info, pointers to flits that make up the msg
 */ 
class ocin_msg {
 public:
  // fields used for routing
  vector <int> src_addr;
  vector <int> dest_addr;

  string src;
  string dst;
  
  // terminal info (local port under concentration)
  string src_t;
  string dst_t;
  int t_out;     // terminal port at destination
  
  Quadrant quadrant;            // Quadrant in which the msg is routed
  bool yFirst;                  // Is this packet routed YX (instead of XY)?
                                //   Used for O1Turn 

  bool is_counted;              /* This flag is true for measured
                                   packets */
  bool dor;                     /* this packet is now in the escape
                                   channels */

  bool post_max;                /* This flag is true packets injected
                                   after max_packet count reached */

  int rt_dimension;             // dimension in which routing was last
                                // performed

  int rt_dir;                   // direct (inc or dec) within the
                                // dimension

  int p_out;                    // output port, determined as a
                                // function of route dimension &
                                // direction

  int vc_out_index;             /* reserved vc for this message
                                   (acquired in vc allocate) used for
                                   cycle determination */

  int reroutes;                 /* number of times this message has
                                   been re-routed due to delays in
                                   routers */

  bool rrt_msg;                 /* set by iu when reroutes indicates
                                   the message needs to be
                                   re-routed. Used to keep track of
                                   cases where we need to charge a
                                   cycle */

  bool add_rrt_delay;           /* tells vc_fifo/vcalloc to add a
                                   cycle of delay to this packet to
                                   account for the latency of a
                                   re-route due to non-full preroutes
                                   deque*/

  map <int, set <int> > routes;   // key = p_out, value = set of valid
                                  // VC's @ this p_out

  deque <int> preroutes;       /* vector of output ports used in
                                   pre-routed packets */

  int hops;

  ocin_mon_packet *mon;         /* packet monitor from the generator
                                   that produced this packet*/
  
  long long pid;                // number of request

  // some stats
  tsim_u64 gen_time;            // cycle the msg was generated

  tsim_u64 inj_time;            // cycle the msg was injected to net

  tsim_u64 dst_time;            // cycle the msg arrived at the
                                // destination router
 	  
  vector <ocin_flit *> flits;
  // TODO: need to have a way of indicating VC restrictions (ie, msg type or QOS)
 	
  ocin_msg() {};
};

#endif /*OCIN_INTERFACE_H_*/
