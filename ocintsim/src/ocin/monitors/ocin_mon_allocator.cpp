#include "ocin_mon_allocator.h"

ocin_mon_allocator::ocin_mon_allocator() {
	// Allocate the checkpoint fifos
	chpt_stalls      = new unsigned int[MON_HISTORY_SIZE];
    chpt_local_reqs  = new unsigned int[MON_HISTORY_SIZE];
    chpt_local_gnts  = new unsigned int[MON_HISTORY_SIZE];
    chpt_global_reqs = new unsigned int[MON_HISTORY_SIZE];
    chpt_global_gnts = new unsigned int[MON_HISTORY_SIZE];
	
	// Clear the checkpoint fifos
	clear();
}


/* init()
 */
void ocin_mon_allocator::init(string n) {
	name = n;
}


/* checkpoint()
 *  - checkpoints state
 */
void ocin_mon_allocator::checkpoint() {
	// first, roll the history over
    for (int i=1; i<MON_HISTORY_SIZE; i++) {
    	chpt_stalls[i]      = chpt_stalls[i-1];
		chpt_local_reqs[i]  = chpt_local_reqs[i-1];
		chpt_local_gnts[i]  = chpt_local_gnts[i-1];
		chpt_global_reqs[i] = chpt_global_reqs[i-1];
		chpt_global_gnts[i] = chpt_global_gnts[i-1];
    }
    
    // set current totals as the checkpoint
	chpt_stalls[0]      = stalls;
	chpt_local_reqs[0]  = local_reqs;
	chpt_local_gnts[0]  = local_gnts;
	chpt_global_reqs[0] = global_reqs;
	chpt_global_gnts[0] = global_gnts;
}


/* clear()
 *  - clears all stats
 */
void ocin_mon_allocator::clear() {
	stalls      = 0;
	local_reqs  = 0;
	local_gnts  = 0;
	global_reqs = 0;
	global_gnts = 0;
	
	for (int i=0; i<MON_HISTORY_SIZE; i++) {
		chpt_stalls[i]      = 0;
		chpt_local_reqs[i]  = 0;
		chpt_local_gnts[i]  = 0;
		chpt_global_reqs[i] = 0;
		chpt_global_gnts[i] = 0;
	}
}


/* operator+=()
 *  - Adds an allocator monitor to this monitor.
 *  - Returns a reference to this mon.
 */
ocin_mon_allocator& ocin_mon_allocator::operator+=(ocin_mon_allocator &m) {
	stalls += m.stalls;
	local_reqs   += m.local_reqs;
	local_gnts   += m.local_gnts;
	global_reqs  += m.global_reqs;
	global_gnts  += m.global_gnts;
	
	for (int i=0; i<MON_HISTORY_SIZE; i++) {
    	chpt_stalls[i]      += m.chpt_stalls[i];
		chpt_local_reqs[i]  += m.chpt_local_reqs[i];
		chpt_local_gnts[i]  += m.chpt_local_gnts[i];
		chpt_global_reqs[i] += m.chpt_global_reqs[i];
		chpt_global_gnts[i] += m.chpt_global_gnts[i];
    }

	return *this;
}

/* demand (local & global)
 *  - Demand = Stalls + Requests
 */
int ocin_mon_allocator::local_demand(bool totals) { 
  if (totals) {
    return (get_all_stalls() + get_all_local_reqs()); 
  } else {
    return (get_curr_stalls() + get_curr_local_reqs());
  } 
}

int ocin_mon_allocator::global_demand(bool totals) { 
  if (totals) {
    return (get_all_stalls() + get_all_global_reqs()); 
  } else {
    return (get_curr_stalls() + get_curr_global_reqs());
  } 
}

/* local efficiency()
 *  grants / (reqs + stalls)
 *  - low efficiency: few grants, many stalls and/or reqs
 *    high efficiency: most/all reqs are granted w/ few stalls
 *  - do a sanity check while we are here
 */
inline float ocin_mon_allocator::local_efficiency(bool totals) { 
	sanity_check();
        float efficiency;
        if(totals) {
          efficiency = (local_demand(totals) == 0) ? 1 : ((float)get_all_local_gnts() / (float)local_demand(totals) );
        } else {
          efficiency = (local_demand(totals) == 0) ? 1 : ((float)get_curr_local_gnts() / (float)local_demand(totals) );
        }

	return efficiency; 
}



/* global efficiency()
 *  grants / (reqs + stalls)
 *  - low efficiency: few grants, many stalls and/or reqs
 *    high efficiency: most/all reqs are granted w/ few stalls
 *  - do a sanity check while we are here
 */
inline float ocin_mon_allocator::global_efficiency(bool totals) { 
	sanity_check();

    float efficiency;
    if (totals) {
      efficiency = (global_demand(totals) == 0) ? 1 : ((float)get_all_global_gnts() / (float)global_demand(totals) );
    }else{
      efficiency = (global_demand(totals) == 0) ? 1 : ((float)get_curr_global_gnts() / (float)global_demand(totals) );
    }
    
	return efficiency; 
}


/* sanity_check()
 *  - ensure that grants don't exceed requests
 */
inline void ocin_mon_allocator::sanity_check() {
  if ((get_all_local_gnts() > get_all_local_reqs()) || 
      (get_all_global_gnts() > get_all_global_reqs())) {
    stringstream s;
    s << "# of reqs exceeded # of grants!!! \n" 
      << print(true) << endl;
    ocin_name_fatal(name, s.str() ); 
    exit(0);
  }
}


/* print()
 *  - returns a string w/ stats
 */
string ocin_mon_allocator::print(bool totals) {
	stringstream s;
	if (name != "") {
		s << name << ": ";
	}
        if (totals) {
          s << "stalls: "   << get_all_stalls();
          s << ", L reqs: " << get_all_local_reqs();
          s << ", L gnts: " << get_all_local_gnts();
          s << ", G reqs: " << get_all_global_reqs();
          s << ", G gnts: " << get_all_global_gnts();
        }
        else {
          s << "stalls: "   << get_curr_stalls();
          s << ", L reqs: " << get_curr_local_reqs();
          s << ", L gnts: " << get_curr_local_gnts();
          s << ", G reqs: " << get_curr_global_reqs();
          s << ", G gnts: " << get_curr_global_gnts();
        }
        s << ", L efficiency = "  << ocin_ftos_3(local_efficiency(totals) );
        s << ", G efficiency = " << ocin_ftos_3(global_efficiency(totals));
	
	return s.str();
}


/* smart_print()
 *  - print stats to the screen 
 *  - perform a sanity check: (# of reqs) > (# of grants)
 *  - selectively, can skip printing if # of reqs = 0
 *  - selectively, can indent the line
 */
void ocin_mon_allocator::smart_print(bool print_all, bool indent, bool totals) {
  sanity_check();
  
  // only print if # of reqs is non-zero
  if (print_all || (global_reqs > 0)) {
    string s = indent ? "\t" : "";
    cout << s << print(totals) << endl;
  }
}


/* operator+()
 *  - Adds 2 allocator monitors and returns the result 
 *    as a new allocator monitor object
 */
ocin_mon_allocator operator+(ocin_mon_allocator &m, ocin_mon_allocator &n) {
	ocin_mon_allocator new_mon = m;
	new_mon += n;

	return new_mon;
}
