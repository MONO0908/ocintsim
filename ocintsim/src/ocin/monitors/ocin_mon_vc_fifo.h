#ifndef OCIN_MON_VC_FIFO_H_
#define OCIN_MON_VC_FIFO_H_

#include "ocin_defs.h"
#include "ocin_helper.h"
//#include "ocin_router.h"
#include "ocin_router_vc_fifo.h"

//class ocin_router_vc_fifo;


class ocin_mon_vc_fifo {
  public:
    string name;
    /* Accumulated values:
     * Store the aggregate values from the beginning of the sim.
     */
    // TODO: these should be 64-bit long long data types
    unsigned int cycles_empty;      // # of cycles w/ no data in buffer
    unsigned int cycles_full;       // # of cycles fifo was full
    unsigned int cycles_allocated;  // # of cycles VC is allocated 
                                    // (though it may be empty due to upstream stalls)
    unsigned int buffers_occupied;  // rolling sum of buffers occupied
    unsigned int buffers_free;      // rolling sum of free buffers
    unsigned int flit_stall_cycles; // rolling sum of flit stall cycles (cycles in fifo w/o deque)

    /* Checkpoint fifos:
     * Fifos store N-deep history of checkpoint values. Note that each 
     * checkpoint is an aggregate value from the beginning of the sim.  
     * The minimum array depth is 2, which is sufficient to reconstruct the   
     * current interval's value as (total - history[0]) and the last checkpoint 
     * value as (history[0] - history[1]).
     */ 
    unsigned int *chpt_cycles_empty;      // # of cycles w/ no data in buffer
    unsigned int *chpt_cycles_full;       // # of cycles fifo was full
    unsigned int *chpt_cycles_allocated;  // # of cycles VC is allocated 
                                    // (though it may be empty due to upstream stalls)
    unsigned int *chpt_buffers_occupied;  // rolling sum of buffers occupied
    unsigned int *chpt_buffers_free;      // rolling sum of free buffers
    unsigned int *chpt_flit_stall_cycles; // rolling sum of flit stall cycles (cycles in fifo w/o deque)
    
    tsim_u64 last_chpt_cycle;                 // cycle of the last checkpoint
    tsim_u64 last_deque_cycle;            // cycle of last deque from this fifo 
    
    // should be private, but is used in composing monitors, etc..
    ocin_router_vc_fifo *_fifo;
    
    ocin_mon_vc_fifo();
    void init(string n, int fifo_size);
    void update();
    void checkpoint(tsim_u64 cycle);
    void clear();             				// clear stats
    ocin_mon_vc_fifo& operator+=(ocin_mon_vc_fifo &m);
    void scale_stats(int scale_factor);     // used in composing multiple monitors
    bool is_starved();
    inline float buffer_utilization(tsim_u64 t, bool totals);
    string print(tsim_u64 t, bool totals);
    void smart_print(tsim_u64 t, bool print_all, bool indent, bool totals);
    
    /** Helpers **/
    void set_fifo_ptr(ocin_router_vc_fifo *fifo)       {_fifo = fifo; };
    inline void clear_running() {cycles_empty = cycles_full = cycles_allocated = 
    	                          buffers_occupied = buffers_free = 0; };
    // track deques from the fifo for deadlock monitoring
    inline void reset_deque_cycle_counter() { last_deque_cycle = ocin_cycle_count; }; 
 	// getter & setter for managing the _fifo_size variable (used in composing monitors)
 	inline int get_fifo_size()             { return _fifo_size; };
 	inline void set_fifo_size(int size)    { _fifo_size = size; };
 	// getters for the entire simulation's values
 	inline unsigned int get_all_cycles_empty()      { return cycles_empty; };
    inline unsigned int get_all_cycles_full()       { return cycles_full; };
    inline unsigned int get_all_cycles_allocated()  { return cycles_allocated; };
    inline unsigned int get_all_buffers_occupied()  { return buffers_occupied; };
    inline unsigned int get_all_buffers_free()      { return buffers_free; };
    inline unsigned int get_all_flit_stall_cycles() { return flit_stall_cycles; }; 
    // getters for current interval's values
    inline unsigned int get_curr_cycles_empty()     { return(cycles_empty - chpt_cycles_empty[0]); };
    inline unsigned int get_curr_cycles_full()      { return(cycles_full - chpt_cycles_full[0]); };
    inline unsigned int get_curr_cycles_allocated() { return(cycles_allocated - chpt_cycles_allocated[0]); };
    inline unsigned int get_curr_buffers_occupied() { return(buffers_occupied - chpt_buffers_occupied[0]); };
    inline unsigned int get_curr_buffers_free()     { return(buffers_free - chpt_buffers_free[0]); };
    inline unsigned int get_curr_flit_stall_cycles() { return (flit_stall_cycles - chpt_flit_stall_cycles[0]); };
    // getters for checkpointed values
   	inline unsigned int get_chpt_cycles_empty()     { return(chpt_cycles_empty[0] - chpt_cycles_empty[1]); };
    inline unsigned int get_chpt_cycles_full()      { return(chpt_cycles_full[0] - chpt_cycles_full[1]); };
    inline unsigned int get_chpt_cycles_allocated() { return(chpt_cycles_allocated[0] - chpt_cycles_allocated[1]); };
    inline unsigned int get_chpt_buffers_occupied() { return(chpt_buffers_occupied[0] - chpt_buffers_occupied[1]); };
    inline unsigned int get_chpt_buffers_free()     { return(chpt_buffers_free[0] - chpt_buffers_free[1]); };
    inline unsigned int get_chpt_flit_stall_cycles() { return (chpt_flit_stall_cycles[0] - chpt_flit_stall_cycles[1]); };
    
  private:
	int _fifo_size;  
};

    ocin_mon_vc_fifo  operator+ (ocin_mon_vc_fifo &m, ocin_mon_vc_fifo &n);


#endif /*OCIN_MON_VC_FIFO_H_*/
