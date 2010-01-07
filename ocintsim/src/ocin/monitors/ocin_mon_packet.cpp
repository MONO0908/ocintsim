#include "ocin_mon_packet.h"

void ocin_mon_packet::init(string n, int _node_cnt) {
  name = n;
  clear();
  node_cnt = (float) _node_cnt;

}

// clear all the vars:
void ocin_mon_packet::clear() {
  // reset everything
  flit_inj = 0;            
  msg_inj = 0;             
  inj_end = 0;             
  msg_comp = 0;            
  flit_comp = 0;           
  end_cycle = 0;       
  end_flit = 0;            
  hop_sum = 0;
  hop_sum_actual = 0;             
  msg_lat_sum = 0;         
  msg_pre_sum = 0;         
  msg_dst_sum = 0;         
  msg_ej_sum = 0;  
  flit_lat_sum = 0;        
  flit_pre_sum = 0;        
  flit_dst_sum = 0;        
  flit_network_sum = 0;    
  first_comp_cycle = 0;

  chpt_inj_end = 0;
  chpt_flit_inj = 0;       
  chpt_msg_inj = 0;        
  chpt_msg_comp = 0;       
  chpt_flit_comp = 0;      
  chpt_hop_sum = 0; 
  chpt_hop_sum_actual = 0;       
  chpt_msg_lat_sum = 0;    
  chpt_msg_pre_sum = 0;    
  chpt_msg_dst_sum = 0;    
  chpt_msg_ej_sum = 0; 
  chpt_flit_lat_sum = 0;   
  chpt_flit_pre_sum = 0;   
  chpt_flit_dst_sum = 0;   
  chpt_flit_network_sum = 0;

}

// checkpoint all chpt vars
void ocin_mon_packet::checkpoint() {
  chpt_inj_end = inj_end;
  chpt_flit_inj = flit_inj;       
  chpt_msg_inj = msg_inj;        
  chpt_msg_comp = msg_comp;       
  chpt_flit_comp = flit_comp;      
  chpt_hop_sum = hop_sum; 
  chpt_hop_sum_actual = hop_sum_actual;       
  chpt_msg_lat_sum = msg_lat_sum;    
  chpt_msg_pre_sum = msg_pre_sum;    
  chpt_msg_dst_sum = msg_dst_sum;    
  chpt_msg_ej_sum = msg_ej_sum; 
  chpt_flit_lat_sum = flit_lat_sum;   
  chpt_flit_pre_sum = flit_pre_sum;   
  chpt_flit_dst_sum = flit_dst_sum;   
  chpt_flit_network_sum = flit_network_sum;
}




/* operator+=()
 *  - Adds an allocator monitor to this monitor.
 *  - Returns a reference to this mon.
 */
ocin_mon_packet& ocin_mon_packet::operator+=(ocin_mon_packet &m) {

  //some get copied
  node_cnt = m.node_cnt;

  // some go with the max value seen
  if(m.inj_end > inj_end) {
    inj_end = m.inj_end;
  }
  if(m.end_cycle > end_cycle) {
    end_cycle = m.end_cycle;
  }

  if((m.first_comp_cycle >0) && (m.first_comp_cycle < first_comp_cycle)) {
    first_comp_cycle = m.first_comp_cycle;
  }

  // the rest get summed
  flit_inj += m.flit_inj;            
  msg_inj += m.msg_inj;             
  msg_comp += m.msg_comp;            
  flit_comp += m.flit_comp;           
  end_flit += m.end_flit;            
  hop_sum += m.hop_sum;
  hop_sum_actual += m.hop_sum_actual;             
  msg_lat_sum += m.msg_lat_sum;         
  msg_pre_sum += m.msg_pre_sum;         
  msg_dst_sum += m.msg_dst_sum;         
  msg_ej_sum += m.msg_ej_sum;  
  flit_lat_sum += m.flit_lat_sum;        
  flit_pre_sum += m.flit_pre_sum;        
  flit_dst_sum += m.flit_dst_sum;        
  flit_network_sum += m.flit_network_sum;    

  chpt_inj_end += m.chpt_inj_end;
  chpt_flit_inj += m.chpt_flit_inj;       
  chpt_msg_inj += m.chpt_msg_inj;        
  chpt_msg_comp += m.chpt_msg_comp;       
  chpt_flit_comp += m.chpt_flit_comp;      
  chpt_hop_sum += m.chpt_hop_sum;        
  chpt_hop_sum_actual += m.chpt_hop_sum_actual;        
  chpt_msg_lat_sum += m.chpt_msg_lat_sum;    
  chpt_msg_pre_sum += m.chpt_msg_pre_sum;    
  chpt_msg_dst_sum += m.chpt_msg_dst_sum;    
  chpt_msg_ej_sum += m.chpt_msg_ej_sum; 
  chpt_flit_lat_sum += m.chpt_flit_lat_sum;   
  chpt_flit_pre_sum += m.chpt_flit_pre_sum;   
  chpt_flit_dst_sum += m.chpt_flit_dst_sum;   
  chpt_flit_network_sum += m.chpt_flit_network_sum;


  return *this;
}

void ocin_mon_packet::print(bool totals) {

  tsim_u64 tmp_gen_cycles = 0;
  tsim_u64 tmp_ej_cycles = 0;
  tsim_u64 tmp_flit_inj = 0;
  tsim_u64 tmp_msg_inj = 0; 
  tsim_u64 tmp_msg_comp = 0;
  tsim_u64 tmp_flit_comp = 0;
  tsim_u64 tmp_end_flit = 0;
  tsim_u64 tmp_hop_sum = 0;  
  tsim_u64 tmp_hop_sum_actual = 0;
  tsim_u64 tmp_msg_lat_sum = 0;
  tsim_u64 tmp_msg_pre_sum = 0;
  tsim_u64 tmp_msg_dst_sum = 0;
  tsim_u64 tmp_msg_ej_sum = 0;
  tsim_u64 tmp_flit_lat_sum = 0;
  tsim_u64 tmp_flit_pre_sum = 0;
  tsim_u64 tmp_flit_dst_sum = 0;
  tsim_u64 tmp_flit_network_sum = 0; 

  if (totals) {
    tmp_gen_cycles = inj_end - param_warmup_cycles;

    
    if(end_cycle !=0) {
      tmp_ej_cycles = end_cycle - first_comp_cycle;
      tmp_end_flit = end_flit;
    } else {
      tmp_ej_cycles = ocin_cycle_count - first_comp_cycle;
      tmp_end_flit = flit_comp;
    }
    tmp_flit_inj = flit_inj;
    tmp_msg_inj = msg_inj; 
    tmp_msg_comp = msg_comp;
    tmp_flit_comp = flit_comp;
    tmp_hop_sum = hop_sum;  
    tmp_hop_sum_actual = hop_sum_actual;
    tmp_msg_lat_sum = msg_lat_sum;
    tmp_msg_pre_sum = msg_pre_sum;
    tmp_msg_dst_sum = msg_dst_sum;
    tmp_msg_ej_sum = msg_ej_sum;
    tmp_flit_lat_sum = flit_lat_sum;
    tmp_flit_pre_sum = flit_pre_sum;
    tmp_flit_dst_sum = flit_dst_sum;
    tmp_flit_network_sum = flit_network_sum; 
  } else {

    tmp_gen_cycles = inj_end - chpt_inj_end;

    if(end_cycle !=0) {
      if (end_cycle + param_stats_interval > ocin_cycle_count) {
        tmp_ej_cycles = end_cycle % param_stats_interval; // want the cycles from the last chpt to the end cycle
        tmp_end_flit = end_flit -  chpt_flit_comp;
      } else {
        tmp_end_flit = 0;           // in this case we are more than one
                                // interval from end_cycle
        tmp_ej_cycles = 0;
      }
    } else {
      tmp_ej_cycles = param_stats_interval;
      tmp_end_flit = flit_comp -  chpt_flit_comp;
    }

    // calc the difference since last checkpoint
    tmp_flit_inj = flit_inj - chpt_flit_inj;
    tmp_msg_inj = msg_inj - chpt_msg_inj; 
    tmp_msg_comp = msg_comp - chpt_msg_comp;
    tmp_flit_comp = flit_comp - chpt_flit_comp;
    tmp_hop_sum = hop_sum - chpt_hop_sum;
    tmp_hop_sum_actual = hop_sum_actual - chpt_hop_sum_actual;
    tmp_msg_lat_sum = msg_lat_sum - chpt_msg_lat_sum;
    tmp_msg_pre_sum = msg_pre_sum - chpt_msg_pre_sum;
    tmp_msg_dst_sum = msg_dst_sum - chpt_msg_dst_sum;
    tmp_msg_ej_sum = msg_ej_sum - chpt_msg_ej_sum;
    tmp_flit_lat_sum = flit_lat_sum - chpt_flit_lat_sum;
    tmp_flit_pre_sum = flit_pre_sum - chpt_flit_pre_sum;
    tmp_flit_dst_sum = flit_dst_sum - chpt_flit_dst_sum;
    tmp_flit_network_sum = flit_network_sum - chpt_flit_network_sum; 
  }



  //  cout << "tmp_end_flit:" << tmp_end_flit << " tmp_ej_cycles:" << tmp_ej_cycles<< " node_cnt:" << node_cnt << "\n";

  stringstream *ss = new stringstream();
  *ss << "Data for Packet Monitor:"<< name <<"\n";
  *ss << "==================================================\n";
  *ss << "total offered packets                : " << tsim_u64tos(tmp_msg_inj) << "\n";
  *ss << "total offered flits                  : " << tsim_u64tos(tmp_flit_inj) << "\n";
  *ss << "total accepted packets               : " << tsim_u64tos(tmp_msg_comp) << "\n";
  *ss << "total accepted flits                 : " << tsim_u64tos(tmp_flit_comp) << "\n";
  *ss << "offered rate (% of max)              : " << ocin_ftos_5( ((float) (tmp_flit_inj) / (float) tmp_gen_cycles) / node_cnt) << "\n";
  *ss << "acceptance rate (% of max)           : " << ocin_ftos_5( ((float) (tmp_end_flit) / (float) tmp_ej_cycles) / node_cnt) << "\n";
  *ss << "ave hop count - mesh (hops/msg)      : " << ocin_ftos_5( (float) (tmp_hop_sum) / (float)(tmp_msg_comp) ) << "\n";
  *ss << "ave hop count - actual (hops/msg)    : " << ocin_ftos_5( (float) (tmp_hop_sum_actual) / (float)(tmp_msg_comp) ) << "\n";
  *ss << "ave msg latency (cycle/msg)          : " << ocin_ftos_5( (float) (tmp_msg_lat_sum) / (float)(tmp_msg_comp) ) << "\n";
  *ss << "ave msg pre-inj wait (cycle/msg)     : " << ocin_ftos_5( (float) (tmp_msg_pre_sum) / (float)(tmp_msg_comp) ) << "\n";
  *ss << "ave msg post-dst wait (cycle/msg)    : " << ocin_ftos_5( (float) (tmp_msg_dst_sum) / (float)(tmp_msg_comp) ) << "\n";
  *ss << "avg wait for ejection VC (cycle/msg) : " << ocin_ftos_5( (float) (tmp_msg_ej_sum) / (float)(tmp_msg_comp) ) << "\n";
  *ss << "ave flit latency (cycle/flit)        : " << ocin_ftos_5( (float) (tmp_flit_lat_sum) / (float)(tmp_flit_comp) ) << "\n";
  *ss << "ave flit pre-inj wait (cycle/flit)   : " << ocin_ftos_5( (float) (tmp_flit_pre_sum) / (float)(tmp_flit_comp) ) << "\n";
  *ss << "ave flit post-dst wait (cycle/flit)  : " << ocin_ftos_5( (float) (tmp_flit_dst_sum) / (float)(tmp_flit_comp) ) << "\n";
  *ss << "ave time in the network (cycle/flit) : " << ocin_ftos_5( (float) (tmp_flit_network_sum) / (float)(tmp_flit_comp) ) << "\n";
  // repeat, but very convenient
  *ss << "** ave msg latency                   : " << ocin_ftos_5( (float) (tmp_msg_lat_sum) / (float)(tmp_msg_comp) ) << "\n";
  *ss << "==================================================\n";
//  cerr << ss->str();
  cout <<ss->str();
  delete ss;
}
