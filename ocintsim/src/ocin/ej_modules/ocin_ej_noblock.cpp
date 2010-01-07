#include "ocin_ej_noblock.h"

void ocin_ej_noblock::init (tsim_object *parent, string _name) {
  
  name = _name;
  tsim_module::init (parent, name);

  top_p = (ocin_top *) parent;

#ifdef DEBUG
  {
    stringstream tmp;
    tmp  << "Initializing the ejector ";
    ocin_name_debug(name,tmp.str());
  }
#endif

  // to speed things up we put pointers to the nodes in a vector

  for (n_it = nodes.begin(); n_it != nodes.end(); n_it++) {
    for (int i=0; i < n_it->second.local_ou.size(); i++) {
      ocin_router_ej * rt_ou = &(n_it->second.local_ou[i]);
      rt_ej_vec.push_back(rt_ou);
    }
  }
  
}




void ocin_ej_noblock::evaluate() {
}

void ocin_ej_noblock::update() {
  vector <ocin_router_vc_fifo> *ej_fifos;    
  vector <ocin_router_vc_fifo>::iterator ej_it;

  // iterate over all the nodes in the vector
  for (re_it = rt_ej_vec.begin(); re_it != rt_ej_vec.end(); re_it++) {

    // grab a ref to the ej_fifos
    ej_fifos = &((*re_it)->ej_fifos);

    // iterate over all the fifos and see if they are non-empty
    for (ej_it = ej_fifos->begin(); ej_it != ej_fifos->end(); ej_it++) {
      //brg
      // Record when the header was received @ dest router.
      // This is used to determine the wait experienced by the msg
      // in allocating an ejection VC
      tsim_u64 header_dst_time = 0;

      if (!ej_it->is_empty() ) {
        
#ifdef DEBUG
        {
          stringstream tmp;
          tmp  << "Found non empty fifo";
          ocin_name_debug(name,tmp.str());
        }
        
#endif
        // if non-empty then deque the flit and do the accounting
        int vc_idx = ej_it->vc_index;
        ocin_flit * ej_flit = (*re_it)->deque(vc_idx);
        
#ifdef PARANOID
        // brg: Make sure actual # of hops matches the expected hop count
        if (ej_flit->is_header) {
          ocin_msg *msg = ej_flit->msg;
          int expected_hops = ocin_count_hop(msg);
          if (msg->hops != expected_hops) {
            stringstream tmp;
            tmp << "Actual hop count for a msg doesn't match the expected hop count!\n" 
                << "\t Actual = " << msg->hops << endl
                << "\t Expected = " << expected_hops;
            ocin_name_fatal(name,tmp.str());
            exit (0);
          }
        }
#endif
        
        int pid = ej_flit->msg->pid;
        bool is_counted = ej_flit->msg->is_counted;
        bool post_max = ej_flit->msg->post_max;
        ocin_mon_packet * mon = ej_flit->msg->mon;
        string src = ej_flit->msg->src;
        string dst = ej_flit->msg->dst;
        bool is_tail = ej_flit->is_tail;
        
        // Check to make sure this message arrived at the right
        // destination
        string ejected_from = ej_it->parent_router->node_info->name;
        if (ejected_from.compare(dst) != 0) {
          stringstream tmp;
          tmp  << "Error: Flit ejected from wrong port: "<<ejected_from <<" \n";     
          tmp << flit2str(ej_flit);
          tmp << "As part of this msg: \n";
          tmp << msg2str(ej_flit->msg, true /*print flits*/);
          ocin_name_fatal(name,tmp.str());
          exit (0);
        }

        // first deque from the msg, thrown assert if it does not
        // match
        
        if (ej_flit != ej_flit->msg->flits.front()) {
          
          stringstream tmp;
          tmp  << "Error: flits received out of order!, got flit: \n";     
          tmp << flit2str(ej_flit);
          tmp << "As part of this msg: \n";
          tmp << msg2str(ej_flit->msg, true /*print flits*/);
          ocin_name_fatal(name,tmp.str());
          exit (0);
        }
        
        // do flit related stats
        if(is_counted) {
          mon->flit_comp++;            // increment flit completion count

          if(param_node_bw_stats == 1) {          
            node_inst_data[src].stats.rtr_comp_flits[dst]++;
          }

          mon->flit_lat_sum += (ocin_cycle_count - ej_flit->gen_time); 
          
          mon->flit_pre_sum += (ej_flit->inj_time - ej_flit->gen_time); 
          
          mon->flit_dst_sum += (ocin_cycle_count - ej_flit->dst_time); 
          
          //brg
          mon->flit_network_sum += (ej_flit->dst_time - ej_flit->inj_time);

          if(mon->first_comp_cycle == 0) {
            mon->first_comp_cycle = ocin_cycle_count; // grab cycle of
                                                      // first
                                                      // completed
                                                      // packet
          }
          
          mon->end_cycle = ocin_cycle_count; // end_cycle and end flit
                                             // are always of the
                                             // latest retired flit
          mon->end_flit = mon->flit_comp;
          if(param_node_bw_stats == 1) {
            if (node_inst_data[src].stats.rtr_end_cycle[dst] == 0) {
              node_inst_data[src].stats.rtr_end_cycle[dst] = 
                ocin_cycle_count;
            }
          }
        }
            


#ifdef DEBUG
        {
          stringstream tmp;
          tmp  << "Erasing flit from msg #" <<pid ;
          ocin_name_debug(name,tmp.str());
        }

#endif

                                // remove from flits in msg
        ej_flit->msg->flits.erase( ej_flit->msg->flits.begin()); 


        if (ej_flit->is_header) {

          if(is_counted) {
            mon->msg_ej_sum += (ej_flit->dst_time - ej_flit->msg->dst_time);
            
            //cout << "flit dst_time: "<<ej_flit->dst_time <<"  msg dst_time: "<< ej_flit->msg->dst_time<<endl;
            // stringstream tmp;
            // tmp  <<"msg:\n";
            // tmp  << msg2str(ej_flit->msg, true /*print flits*/);
            // ocin_name_warn(name,tmp.str());

            // count the # of cycles spent waiting for an ejection
            // VC. This is the diff b/w the time the header flit is
            // recv'd at the ejector VC and the time it became head of
            // the input VC @ dest rtr.
          }

        } 
        
        if (is_tail) {          // also remove msg from msg_map

          ocin_msg * msg = ej_flit->msg; // grab a ptr to reduce
                                         // derefs
          
#ifdef DEBUG
          {
            stringstream tmp;
            tmp  <<"Erasing msg:\n";
            tmp  << msg2str(msg, true /*print flits*/);
            ocin_name_debug(name,tmp.str());
          }
#endif

          // do the msg related statistics:
          if(is_counted) {
            mon->msg_comp++;         // increment msg completion count

            //            cout << "Ejecting: " << ocin_cycle_count << ":" << msg->gen_time <<endl;

            mon->msg_lat_sum += (ocin_cycle_count - msg->gen_time); 
                                // increment the cycls/msg sum
            mon->msg_pre_sum += (msg->inj_time - msg->gen_time); 
                                // increment the cycls/msg sum

            mon->msg_dst_sum += (ocin_cycle_count - msg->dst_time); 
                                // increment the cycls/msg sum

            mon->hop_sum += ocin_count_hop(msg); // ideal hop count
                                // sum
            mon->hop_sum_actual += msg->hops;  // actual # of hops taken

            // Remove msg from map and delete the msg itself
            msg_map.erase(pid);
          }
          delete msg;
          
        }

        // deleting the flit
        delete ej_flit;
      }
    }
  }
}
        
        

        
  

void ocin_ej_noblock::finalize() {
}

