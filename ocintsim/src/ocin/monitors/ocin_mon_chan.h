#ifndef OCIN_MON_CHAN_H_
#define OCIN_MON_CHAN_H_

#include "ocin_defs.h"
#include "ocin_helper.h"

class ocin_mon_chan {
public:
  string name;
  // chan statitical varaibles:

  tsim_u64 flit_count;          /* running count of flits through this
                                   node */

  tsim_u64 chpt_flit_count;      /* checkpoint of flit count */

  void init(string n); 
  void clear();             // clear stats
  void checkpoint();          /* checkpoint values */
  ocin_mon_chan& operator+=(ocin_mon_chan &m);
  
  inline tsim_u64 get_all_flit_count()   { return flit_count; };
  inline tsim_u64 get_chpt_flit_count()  { return (flit_count - chpt_flit_count); };



};


#endif /*OCIN_MON_CHAN_H_*/
