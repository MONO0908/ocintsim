#include "ocin_mon_port.h"

void ocin_mon_port::init(string n) {
	name = n;
	 
	vmon.init("VC");
	xmon.init("XB");
}


/* operator+=()
 *  - Adds a port monitor to this monitor.
 *  - Returns a reference to this mon.
 */
ocin_mon_port& ocin_mon_port::operator+=(ocin_mon_port &m) {
	vmon += m.vmon;
	xmon += m.xmon;
	
	return *this;
}


/* net_vc_stalls()
 *  - VC stall cycles: stalls + reqs - gnts
 */
inline int ocin_mon_port::net_vc_stalls(bool totals) {
  int vmon_stalls;
  
  if (totals) {
    vmon_stalls = vmon.global_demand(totals) - vmon.get_all_global_gnts();
  } 
  else {  
    vmon_stalls = vmon.global_demand(totals) - vmon.get_curr_global_gnts();     
  }
  
  return vmon_stalls;
}


/* net_efficiency()
 *  - attempts to capture contention @ this port
 *  - takes into account net VC stall & XB demand.
 *    grants / (net_vc_stalls + reqs + stalls)
 *  - low efficiency: few grants, many stalls and/or reqs
 *    high efficiency: most/all reqs are granted w/ few stalls
 *  - NOTE: global requests & global grants are used.
 */
inline float ocin_mon_port::efficiency(bool totals) {
  int total_demand = vmon.global_demand(totals) + xmon.global_demand(totals);
  int net_demand, net_grants;
  float efficiency;
  
  if (totals) {
    net_demand = total_demand - vmon.get_all_global_gnts();
    net_grants = xmon.get_all_global_gnts();
    efficiency = (total_demand == 0) ? 1 : ((float)net_grants / (float)net_demand );
  }
  else{
    net_demand = total_demand - (vmon.get_curr_global_gnts());
    net_grants = xmon.get_curr_global_gnts();
    efficiency = (total_demand == 0) ? 1 : ((float)net_grants / (float)net_demand );
  }
  
  return efficiency; 
}


/* print()
 *  - returns a string w/ stats
 */
string ocin_mon_port::print(bool verbose, bool totals) {
  stringstream s;
  int ggrants = totals ? xmon.get_all_global_gnts() : xmon.get_curr_global_gnts();  
  
  s << name;
  s << ": Net VC stall = " << net_vc_stalls(totals);
  s << ", XB demand = " << xmon.global_demand(totals);
  s << ", grants = " << ggrants;
  s << ", efficiency = " << ocin_ftos_3(efficiency(totals) );
  s << "\n";
  if (verbose) {
    s << "\t" << vmon.print(totals) << "\n";
    s << "\t" << xmon.print(totals) << "\n";
  }
  
  return s.str();
}


/* smart_print()
 *  - print stats to the screen
 *  - selectively, can skip printing if both VC & XB demand is 0 (print_all)
 *  - selectively, can print detailed stats for VC & XB monitors (verbose)
 *  - selectively, can indent the line (indent)
 */
void ocin_mon_port::smart_print(bool print_all, bool verbose, bool totals) {
  int total_demand = vmon.global_demand(totals) + xmon.global_demand(totals);
  // only print if combined VC & XB demand is non-zero
  if (print_all || (total_demand > 0)) {
    cout << print(verbose,totals);
  }
}


/* operator+()
 *  - Adds 2 port monitors and returns the result 
 *    as a new port monitor object
 */
ocin_mon_port operator+(ocin_mon_port &m, ocin_mon_port &n) {
	ocin_mon_port new_mon = m;
	new_mon += n; 

	return new_mon;
}
