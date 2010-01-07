#include "ocin_mon_chan.h"

void ocin_mon_chan::init(string n) {
  name = n;
  clear();

}

// clear all the vars:
void ocin_mon_chan::clear() {
  // reset everything
  flit_count = 0;            
  chpt_flit_count = 0;

}

// checkpoint all chpt vars
void ocin_mon_chan::checkpoint() {
  chpt_flit_count = flit_count;       

}




/* operator+=()
 *  - Adds an allocator monitor to this monitor.
 *  - Returns a reference to this mon.
 */
ocin_mon_chan& ocin_mon_chan::operator+=(ocin_mon_chan &m) {
  // the rest get summed
  flit_count += m.flit_count;            

  chpt_flit_count += m.chpt_flit_count;       


  return *this;
}
