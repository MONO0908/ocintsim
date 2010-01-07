#ifndef OCIN_MON_PACKET_H_
#define OCIN_MON_PACKET_H_

#include "ocin_defs.h"
#include "ocin_helper.h"

class ocin_mon_packet {
public:
  string name;
  float node_cnt;

  // packet statitical varaibles:

  tsim_u64 flit_inj;            /* running count of injectd flits */
  tsim_u64 msg_inj;             /* running count of injected msgs */
  tsim_u64 chpt_flit_inj;       /* checkpoint of flit_inj */
  tsim_u64 chpt_msg_inj;        /* checkpoint of msg_inj */
  
  tsim_u64 inj_end;             /* cycle of the last msg injection */
  tsim_u64 chpt_inj_end;

  tsim_u64 first_comp_cycle;    /* cycle of the first completed
                                   packet */

  tsim_u64 msg_comp;            /* running count of completed msgs */
  tsim_u64 flit_comp;           /* running count of completed flits */
  tsim_u64 end_cycle;       /* cycle of the first non-counted flit  */
  tsim_u64 end_flit;            /* count of flits at end_cycle */
  tsim_u64 hop_sum;             /* sum of min path hops from src to dest (2D mesh) */
  tsim_u64 hop_sum_actual;      /* actual # of hops */
  tsim_u64 msg_lat_sum;         /* sum of cycles from gen to ejc */
  tsim_u64 msg_pre_sum;         /* sum of cycles between gen and
                                   inj */
  tsim_u64 msg_dst_sum;         /* sum of cycles between arriving at
                                   dest and ejc  */
  tsim_u64 msg_ej_sum;  /* sum of cycles spent waiting for eject VC */
  
  tsim_u64 flit_lat_sum;        /* sum of cycles from gen to ejc */
  tsim_u64 flit_pre_sum;        /* sum of cycles between gen and
                                   inj */
  tsim_u64 flit_dst_sum;        /* sum of cycles between arriving at
                                   dest and ejc  */
  tsim_u64 flit_network_sum;    /* # of cycles spent in the network
                                   from inj at source to dropping off
                                   at the ejection fifo @ dest */

  tsim_u64 chpt_msg_comp;       /* running count of completed msgs */
  tsim_u64 chpt_flit_comp;      /* running count of completed flits */
  tsim_u64 chpt_hop_sum;        /* sum of min path hops from src to dest */
  tsim_u64 chpt_hop_sum_actual; /* actual # of hops */
  tsim_u64 chpt_msg_lat_sum;    /* sum of cycles from gen to ejc */
  tsim_u64 chpt_msg_pre_sum;    /* sum of cycles between gen and
                                   inj */
  tsim_u64 chpt_msg_dst_sum;    /* sum of cycles between arriving at
                                   dest and ejc  */
  tsim_u64 chpt_msg_ej_sum; /* sum of cycles spent waiting for eject VC */
  
  tsim_u64 chpt_flit_lat_sum;   /* sum of cycles from gen to ejc */
  tsim_u64 chpt_flit_pre_sum;   /* sum of cycles between gen and
                                   inj */
  tsim_u64 chpt_flit_dst_sum;   /* sum of cycles between arriving at
                                   dest and ejc  */
  tsim_u64 chpt_flit_network_sum; /* # of cycles spent in the network
                                     from inj at source to dropping off
                                     at the ejection fifo @ dest */



  void init(string n, int _node_count); 
  void clear();             // clear stats
  void checkpoint();          /* checkpoint values */
  ocin_mon_packet& operator+=(ocin_mon_packet &m);
  


  void print(bool totals);

};


#endif /*OCIN_MON_PACKET_H_*/
