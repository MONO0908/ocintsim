#include "ocin_mon_vc_fifo.h"

ocin_mon_vc_fifo::ocin_mon_vc_fifo() {
	// Allocate the checkpoint fifos
	chpt_cycles_empty =  new unsigned int[MON_HISTORY_SIZE];
	chpt_cycles_full = new unsigned int[MON_HISTORY_SIZE];
	chpt_cycles_allocated = new unsigned int[MON_HISTORY_SIZE];
	chpt_buffers_occupied = new unsigned int[MON_HISTORY_SIZE];
	chpt_buffers_free = new unsigned int[MON_HISTORY_SIZE];
	chpt_flit_stall_cycles = new unsigned int[MON_HISTORY_SIZE];

	
	// Clear the checkpoint fifos
	clear();
}

/* init()
 */
void ocin_mon_vc_fifo::init(string n, int fifo_size) {
	name = n;
	_fifo_size = fifo_size;
	last_deque_cycle = 0;
}


/* update()
 *  - update fifo stats
 */
void ocin_mon_vc_fifo::update() {
	int occupancy = _fifo->data.size();
	buffers_occupied += occupancy;
	buffers_free += _fifo->credits;
	cycles_empty += (occupancy == 0);
	cycles_full += _fifo->is_full();
	cycles_allocated += !(_fifo->is_avail());
	flit_stall_cycles += (_fifo->is_empty() ? 0 : (ocin_cycle_count - last_deque_cycle));
}


/* checkpoint(tsim_u64 cycle)
 *  - checkpoint's the data for incremental stats
 */
void ocin_mon_vc_fifo::checkpoint(tsim_u64 cycle) {
  // first, roll the history over
  for (int i=1; i<MON_HISTORY_SIZE; i++) {
		chpt_cycles_empty[i]     = chpt_cycles_empty[i-1]; 
		chpt_cycles_full[i]      = chpt_cycles_full[i-1]; 
		chpt_cycles_allocated[i] = chpt_cycles_allocated[i-1];
		chpt_buffers_occupied[i] = chpt_buffers_occupied[i-1];	
		chpt_buffers_free[i]     = chpt_buffers_free[i-1];
		chpt_flit_stall_cycles[i] = chpt_flit_stall_cycles[i-1];
  }
  
  // set current totals as the checkpoint
  chpt_cycles_empty[0] = cycles_empty;
  chpt_cycles_full[0] = cycles_full;
  chpt_cycles_allocated[0] = cycles_allocated;
  chpt_buffers_occupied[0] = buffers_occupied;
  chpt_buffers_free[0] = buffers_free;
  chpt_flit_stall_cycles[0] = flit_stall_cycles;
  
  last_chpt_cycle = cycle;
}


/* clear()
 *  - clears all stats
 */
void ocin_mon_vc_fifo::clear() {
	cycles_empty = 0;
	cycles_full = 0;
	cycles_allocated = 0;
	buffers_occupied = 0;
	buffers_free = 0;
    flit_stall_cycles = 0;
    
    for (int i=0; i<MON_HISTORY_SIZE; i++) {
	    chpt_cycles_empty[i] = 0;
	    chpt_cycles_full[i] = 0;
		chpt_cycles_allocated[i] = 0;
		chpt_buffers_occupied[i] = 0;
		chpt_buffers_free[i] = 0;
		chpt_flit_stall_cycles[i] = 0;
    }
	
    last_chpt_cycle = 0;
    last_deque_cycle = 0;
}


/* operator+=()
 *  - Adds a vc fifo monitor to this monitor object 
 *    and returns a reference to this.
 */
ocin_mon_vc_fifo& ocin_mon_vc_fifo::operator+=(ocin_mon_vc_fifo &m) {
	cycles_empty     += m.cycles_empty; 
	cycles_full     += m.cycles_full;
	cycles_allocated += m.cycles_allocated;
	buffers_occupied += m.buffers_occupied;
	buffers_free += m.buffers_free;
	flit_stall_cycles += m.flit_stall_cycles;		

	for (int i=0; i<MON_HISTORY_SIZE; i++) {
		chpt_cycles_empty[i]     += m.chpt_cycles_empty[i]; 
		chpt_cycles_full[i]     += m.chpt_cycles_full[i]; 
		chpt_cycles_allocated[i] += m.chpt_cycles_allocated[i];
		chpt_buffers_occupied[i] += m.chpt_buffers_occupied[i];	
		chpt_buffers_free[i] += m.chpt_buffers_free[i];	
		chpt_flit_stall_cycles[i] += m.chpt_flit_stall_cycles[i];	
	}
	
	last_chpt_cycle = m.last_chpt_cycle;
	last_deque_cycle = m.last_deque_cycle;

	return *this;
}


/* scale_stats()
 *  - Scales FIFO stats to help in composing fifo monitors.
 *  - Composed (aggregated) monitors are not always useful
 *    without scaling the resulting monitor by the number
 *    of aggregated fifos. 
 */
void ocin_mon_vc_fifo::scale_stats(int scale_factor) {
	cycles_empty /= scale_factor;
    cycles_full /= scale_factor;
    cycles_allocated /= scale_factor;
    buffers_occupied /= scale_factor;
    buffers_free /= scale_factor;
    flit_stall_cycles /= scale_factor;
    
    for (int i=0; i<MON_HISTORY_SIZE; i++) {
	    chpt_cycles_empty[i] /= scale_factor;
	    chpt_cycles_full[i] /= scale_factor;
		chpt_cycles_allocated[i] /= scale_factor;
		chpt_buffers_occupied[i] /= scale_factor;
		chpt_buffers_free[i] /= scale_factor;
		chpt_flit_stall_cycles[i] /= scale_factor;
    } 
}


/* buffer_utilization()
 *  - utilization (%) = (buffer occupancy / fifo size) / time
 *  - if (totals==true) prints the aggregate utilization (from t=0)
 *  - if (totals==false) prints utilization for the *current*
 *    interval (aggregate - last_checkpoint) 
 */
inline float ocin_mon_vc_fifo::buffer_utilization(tsim_u64 t, bool totals) {
  if (t == 0) {
    stringstream tmp;
    tmp << "Can't compute utilization w/ iterval = 0" <<endl;
    ocin_name_fatal(name,tmp.str());
    exit(0);
  }
  
  if (totals) {
    return ( ((float)get_all_buffers_occupied() / (float)_fifo_size ) / t);
  } else {
    return ( ((float)get_curr_buffers_occupied() / (float)_fifo_size ) / (t - last_chpt_cycle));
  }
}


/* is_starved()
 *  - Check if the FIFO is non-empty and has not been pulled from in a while.
 *  - Compares current cycle count with that of last pull. If it exceeds
 *    10,000 cycles and fifo is non-empty, return true.
 *  - This is a simple check and is not a definite indication of true
 *    starvation or deadlock
 */
bool ocin_mon_vc_fifo::is_starved() {
	if (!_fifo->is_empty()) {
		tsim_u64 cycles_since_last_deque = ocin_cycle_count - last_deque_cycle;
		if (cycles_since_last_deque > 2000) 
		  return true;
	}
	
	return false;
}



/* print()
 *  - returns a string w/ stats
 *  - if (totals==true) prints the aggregate stats (from t=0)
 *  - if (totals==false) prints the stats for the *current*
 *    interval (aggregate - last_checkpoint)
 */
string ocin_mon_vc_fifo::print(tsim_u64 t, bool totals) {
  stringstream s;
  
  if (totals) {
    tsim_u64 all_cycles_nonempty = t - get_all_cycles_empty(); 

    s << name;
    s << ": cycles non-empty: " << all_cycles_nonempty
      << " (" << ocin_ftos_3(100.0 * ((float)all_cycles_nonempty) / t) << "%)";
    s << ", full: " << get_all_cycles_full()
      << " (" << ocin_ftos_3(100.0 * ((float)get_all_cycles_full()) / t) << "%)";
    s << ", alloc'd: " << get_all_cycles_allocated()
      << " (" << ocin_ftos_3(100.0 * ((float)get_all_cycles_allocated()) / t) << "%)";
    s << ", occupancy = " << get_all_buffers_occupied()
      << " (util = " << ocin_ftos_3(buffer_utilization(t, true) * 100) << "%)";
    s << ", flit stalls: " << get_all_flit_stall_cycles();
  } 
  else {  
  	// stats for the *current* interval
    tsim_u64 t_delta = t - last_chpt_cycle;
    tsim_u64 curr_cycles_nonempty = t_delta - get_curr_cycles_empty();
    //    cout << name << " : t :"<< t << " : chpt_cycle : " << chpt_cycle << " : cycles_empty : " << cycles_empty << " : chpt_cycles_empty : " << chpt_cycles_empty <<endl;

    s << name;
    s << ": cycles non-empty: " << curr_cycles_nonempty
      << " (" << ocin_ftos_3(100.0 * ((float)curr_cycles_nonempty) / t_delta) << "%)";
    s << ": full: " << get_curr_cycles_full()
      << " (" << ocin_ftos_3(100.0 * (float)get_curr_cycles_full() / t_delta) << "%)";
    s << ", alloc'd: " << get_curr_cycles_allocated()
      << " (" << ocin_ftos_3(100.0 * (float)get_curr_cycles_allocated() / t_delta) << "%)";
    s << ", occupancy = " << get_curr_buffers_occupied()
      << " (util = " << ocin_ftos_3(buffer_utilization(t, false) * 100) << "%)";
    s << ", flit stalls: " << get_curr_flit_stall_cycles();
  }

    return s.str();
}


/* smart_print()
 *  - print stats to the screen 
 *  - perform a sanity check: buffer utilization should not exceed 1.0
 *  - selectively, can skip printing if utilization = 0
 *  - selectively, can indent the line
 */
void ocin_mon_vc_fifo::smart_print(tsim_u64 t, bool print_all, bool indent, bool totals) {
	// sanity check: ensure that buffer utilization does not exceed 1.0
	if (buffer_utilization(t, totals) > 1.0) {
		stringstream s;
		s << "Buffer utilization exceeded 1.0!!! \n" 
		  << print(t, totals) << endl;
		ocin_name_fatal(name, s.str() ); 
		exit(0);
	}
	
	if (print_all || (get_all_buffers_occupied() > 0)) {
		string s = indent ? "\t" : "";
      	cout << s << print(t, totals) << endl;
	}
}


/* operator+()
 *  - Adds 2 vc fifo stat objects and 
 *    returns the result as a new fifo stat object
 */
ocin_mon_vc_fifo operator+(ocin_mon_vc_fifo &m, ocin_mon_vc_fifo &n) {
	ocin_mon_vc_fifo new_mon = m;
	
	return (new_mon+=n);
}
