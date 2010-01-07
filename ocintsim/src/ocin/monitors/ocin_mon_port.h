#ifndef OCIN_MON_PORT_H_
#define OCIN_MON_PORT_H_

#include "ocin_mon_allocator.h"

class ocin_mon_port {
  public:
    string name;
    ocin_mon_allocator vmon;    // monitors VC contention
    ocin_mon_allocator xmon;    // monitors x-bar contention
  
    ocin_mon_port() { clear(); };
    void init(string n);
    inline void checkpoint() { vmon.checkpoint(); xmon.checkpoint(); }; 
    inline void clear()      { vmon.clear(); xmon.clear(); };
    ocin_mon_port& operator+=(ocin_mon_port &m);
    inline int net_vc_stalls(bool totals);
    inline float efficiency(bool totals);
    string print(bool verbose, bool totals);
    void smart_print(bool print_all, bool verbose, bool totals);
    
  private:
};

    ocin_mon_port  operator+ (ocin_mon_port &m, ocin_mon_port &n);

#endif /*OCIN_MON_PORT_H_*/
