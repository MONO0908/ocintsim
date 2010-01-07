#ifndef OCIN_MON_ALLOCATOR_H_
#define OCIN_MON_ALLOCATOR_H_

#include "ocin_defs.h"
#include "ocin_helper.h"

class ocin_mon_allocator {
  public:
    string name;
    // accumulated values
    unsigned int stalls;         // resource unavailable
    unsigned int local_reqs;     // only if resource is avail
    unsigned int local_gnts;     // resource avail & selected by arb
    unsigned int global_reqs;
    unsigned int global_gnts;
	// checkpointed values
    unsigned int *chpt_stalls;        // resource unavailable
    unsigned int *chpt_local_reqs;    // only if resource is avail
    unsigned int *chpt_local_gnts;    // resource avail & selected by arb
    unsigned int *chpt_global_reqs;
    unsigned int *chpt_global_gnts;
  
    ocin_mon_allocator();
    void init(string n); 
    void clear();             // clear stats
    void checkpoint();          /* checkpoint values */
    ocin_mon_allocator& operator+=(ocin_mon_allocator &m);
    int local_demand(bool totals);
    int global_demand(bool totals);
    inline float local_efficiency(bool totals);
    inline float global_efficiency(bool totals);
    inline void sanity_check();
    string print(bool totals);
    void smart_print(bool print_all, bool indent, bool totals);
    
    /** Helpers **/
    // increment both local & global reqs
    inline void inc_reqs() {local_reqs++; global_reqs++; };
    // increment both local & global grants
    inline void inc_gnts() {local_gnts++; global_gnts++; };
    // getters for the entire simulation's values
    inline unsigned int get_all_stalls()       { return stalls; };
    inline unsigned int get_all_local_reqs()   { return local_reqs; };
    inline unsigned int get_all_local_gnts()   { return local_gnts; };
    inline unsigned int get_all_global_reqs()  { return global_reqs; };
    inline unsigned int get_all_global_gnts()  { return global_gnts; };
    // getters for current interval's values
	inline unsigned int get_curr_stalls()      { return (stalls - chpt_stalls[0]); };
    inline unsigned int get_curr_local_reqs()  { return (local_reqs - chpt_local_reqs[0]); };
    inline unsigned int get_curr_local_gnts()  { return (local_gnts - chpt_local_gnts[0]); };
    inline unsigned int get_curr_global_reqs() { return (global_reqs - chpt_global_reqs[0]); };
    inline unsigned int get_curr_global_gnts() { return (global_gnts - chpt_global_gnts[0]); };
    // getters for checkpointed values
    inline unsigned int get_chpt_stalls()      { return (chpt_stalls[0] - chpt_stalls[1]); };
    inline unsigned int get_chpt_local_reqs()  { return (chpt_local_reqs[0] - chpt_local_reqs[1]); };
    inline unsigned int get_chpt_local_gnts()  { return (chpt_local_gnts[0] - chpt_local_gnts[1]); };
    inline unsigned int get_chpt_global_reqs() { return (chpt_global_reqs[0] - chpt_global_reqs[1]); };
    inline unsigned int get_chpt_global_gnts() { return (chpt_global_gnts[0] - chpt_global_gnts[1]); };
    
  private:
  
};

    ocin_mon_allocator  operator+ (ocin_mon_allocator &m, ocin_mon_allocator &n);

#endif /*OCIN_MON_ALLOCATOR_H_*/
