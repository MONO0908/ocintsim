#include "ocin_mon_node.h"

void ocin_mon_node::clear() {
  rtr_inj_flits.clear();
  chpt_rtr_inj_flits.clear();
  
  rtr_comp_flits.clear();
  chpt_rtr_comp_flits.clear();
  
  rtr_end_cycle.clear();
}


// this populates the stats maps with nodes that all have 0s
void ocin_mon_node::init(vector<string> &node_names, string _name) {
  
  name = _name;
  
  vector<string>::iterator name_it;
  for(name_it = node_names.begin(); 
      name_it != node_names.end(); name_it++) {
      
    rtr_inj_flits[*name_it] = 0;
    chpt_rtr_inj_flits[*name_it] = 0;
    
    rtr_comp_flits[*name_it] = 0;
    chpt_rtr_comp_flits[*name_it] = 0;
    
    rtr_end_cycle[*name_it] = 0;
    
  }
}

void ocin_mon_node::checkpoint() {
  
  chpt_rtr_inj_flits = rtr_inj_flits;
  chpt_rtr_comp_flits = rtr_comp_flits;
}

void ocin_mon_node::print(vector<string> &node_names, bool header, 
                          bool acc_off, bool totals, 
                          tsim_u64 inj_end, tsim_u64 chpt_inj_end) {
  vector<string>::iterator name_it;

  // print header if needed...
  if(header) {
    cout <<"node ,";
    for(name_it = node_names.begin(); 
        name_it != node_names.end(); name_it++) {
      
      printf("%5s,",name_it->c_str());
    }
    cout << " mean\n";
  }

  tsim_u64 cycle, flits;
  
  printf("%5s,",name.c_str());

  float sum = 0;
  float sumsq = 0;
  for(name_it = node_names.begin(); 
      name_it != node_names.end(); name_it++) {

    if(acc_off){                // doing the accepted BW

      if(totals) {

        // we have not reached the comp flit for this node
        if(rtr_end_cycle[*name_it] != 0) {
          cycle = rtr_end_cycle[*name_it] - param_warmup_cycles;
        } else {
          cycle = ocin_cycle_count - param_warmup_cycles;
        }
        
        flits = rtr_comp_flits[*name_it];

      } else {
        if(rtr_end_cycle[*name_it] != 0) {
          if (rtr_end_cycle[*name_it] + 
              param_stats_interval >ocin_cycle_count) {
            cycle = rtr_end_cycle[*name_it] % param_stats_interval;
          } else {
            cycle = 0;
          }
        } else {
          cycle = param_stats_interval;
        }
        
        flits = rtr_comp_flits[*name_it] - chpt_rtr_comp_flits[*name_it];

      }
    } else {                    // doing offered BW
      if (totals) {
        cycle = inj_end - param_warmup_cycles;
        flits = rtr_inj_flits[*name_it];
      } else {
        cycle = inj_end - chpt_inj_end;
        flits = rtr_inj_flits[*name_it] - chpt_rtr_inj_flits[*name_it];
      }
    }

    // now do the actual print...
    float temp = (float)flits / (float)cycle; 
    if (cycle == 0) {
      temp = 0;
    }
    sum += temp;
    printf("%5.3f,",temp);
    //    printf("%5d,",cycle);
  }

  printf("%5.3f\n",sum/(float)(node_names.size()));
  
}

    

  
  
