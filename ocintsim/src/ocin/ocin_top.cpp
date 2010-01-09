#include "ocin_top.h"

void ocin_top::init (tsim_object* parent, const string& _name) {


  tsim_module::init( parent, _name );

  modname = _name;

  logstring = new stringstream;

  ocin_cycle_count = 0;

  get_params();
  set_network_size();

  
  set_bandwidth();
  create_generator();

  create_ejector();

  // TODO: new code goes here!
  create_internal_wires();
  create_tiles();


  create_vis();
  
  // Print config options to disambiguate outputs
  print_config();

}

void ocin_top::get_params() {
  // some baseline params, mostly temporary

  param_src_sigma_str = "1.0";
  define_parameter( "src_sigma_str", param_src_sigma_str);
  char *tail;
  param_src_sigma = strtod (param_src_sigma_str.c_str(), &tail);

  param_dst_sigma_str = "1.0";
  define_parameter( "dst_sigma_str", param_dst_sigma_str);
  param_dst_sigma = strtod (param_dst_sigma_str.c_str(), &tail);

  param_extra_rca_delay = 0;
  define_parameter( "extra_rca_delay", param_extra_rca_delay);

  param_pkt_throttle = 5000;
  define_parameter( "pkt_throttle", param_pkt_throttle);

  param_preroute_scale = 100;   // note this is a percentage
  define_parameter( "preroute_scale", param_preroute_scale);

  param_repreroute_delay = 0;
  define_parameter( "repreroute_delay", param_repreroute_delay);

  param_reroute_limit = 2;
  define_parameter( "reroute_limit", param_reroute_limit);

  param_dim_ave = 0;
  define_parameter( "dim_ave", param_dim_ave);

  param_preroute_baseline = 5;
  define_parameter( "preroute_baseline", param_preroute_baseline);

  param_repreroute_hops = 0;
  define_parameter( "repreroute_hops", param_repreroute_hops);

  param_repreroute_cycles = 0;
  define_parameter( "repreroute_cycles", param_repreroute_cycles);

  param_preroute_pkts = 0;
  define_parameter( "preroute_pkts", param_preroute_pkts);

  param_ocn_4proc = 0;
  define_parameter( "ocn_4proc", param_ocn_4proc);

  param_midpoint_file = 0;
  define_parameter( "midpoint_file", param_midpoint_file);

  param_seed = 1;
  define_parameter("seed", param_seed);
  // first set up the seed
  
  cout <<"Using random seed: "<<param_seed<<endl;

  srandom((unsigned)param_seed);

  param_network_terminals = 0;
  define_parameter( "network_terminals", param_network_terminals);

  param_num_x_tiles = 0;
  define_parameter( "num_x_tiles", param_num_x_tiles);

  param_num_y_tiles = 0;
  define_parameter( "num_y_tiles", param_num_y_tiles);

  param_fgen_rate = 1000;
  define_parameter( "fgen_rate", param_fgen_rate);

  param_random_req_size = 0;
  define_parameter( "random_req_size", param_random_req_size);

  param_bimodal_req_size = 0;
  define_parameter( "bimodal_req_size", param_bimodal_req_size);
  
  param_bimodal_size1 = 64;
  define_parameter( "bimodal_size1", param_bimodal_size1);
  
  param_bimodal_size2 = 512;
  define_parameter( "bimodal_size2", param_bimodal_size2);
  
  param_baseline_channel_width = 128;
  define_parameter( "baseline_channel_width", param_baseline_channel_width);  

  param_show_body_flit_log = 0;
  define_parameter( "show_body_flit_log", param_show_body_flit_log);

  param_wire_delay = 0;
  define_parameter("wire_delay", param_wire_delay);
  
  param_hops_per_cycle = 0; 
  define_parameter("hops_per_cycle", param_hops_per_cycle);

  param_router_pipeline_latency = 0;
  define_parameter("router_pipeline_lat", param_router_pipeline_latency);  

  param_netcfg_file = "";
  define_parameter( "netcfg_file", param_netcfg_file);

  param_tracefile_name = "";
  define_parameter( "tracefile_name", param_tracefile_name);

  // Self-similar params
  param_selfsim_inj = 0;
  define_parameter( "selfsim_inj", param_selfsim_inj);
  
  param_selfsim_trace1 = "";
  define_parameter( "selfsim_trace1", param_selfsim_trace1);

  param_selfsim_trace2 = "";
  define_parameter( "selfsim_trace2", param_selfsim_trace2);


  param_vis_fifo_type = "free_buff";
  define_parameter( "vis_fifo_type", param_vis_fifo_type);

  param_vis_link_type = "xbar_gnts";
  define_parameter( "vis_link_type", param_vis_link_type);

  param_max_expr_hop = 1;
  define_parameter( "max_expr_hop", param_max_expr_hop);

  param_flit_max = 1;
  define_parameter( "flit_max", param_flit_max);

  param_inj_vc_count = 1;
  define_parameter( "inj_vc_count", param_inj_vc_count);

  param_ej_vc_count = 1;
  define_parameter( "ej_vc_count", param_ej_vc_count);

  param_percent_inj_bw = 20;
  define_parameter( "percent_inj_bw", param_percent_inj_bw);


  // Indicate whether to checkpoint monitors & chkpt frequency
  param_incr_chkpt = 0;
  define_parameter( "incr_chkpt", param_incr_chkpt);

  param_chkpt_interval = 1;
  define_parameter( "chkpt_interval", param_chkpt_interval);
  
  // Print stats periodically?
  param_incr_stats =0;
  define_parameter( "incr_stats", param_incr_stats);
    
  param_stats_interval = 1000;
  define_parameter( "stats_interval", param_stats_interval);
  
  // If set, monitors for each node will be dumped along with incr stats 
  param_node_stats = 0;
  define_parameter( "node_stats", param_node_stats);


  param_vis_on = 0;
  define_parameter( "vis_on", param_vis_on);

  param_vis_start = 0xffffffffffffffffull; // setting it to max value as a flag
  define_parameter( "vis_start", param_vis_start);

  param_vis_stop = 0xffffffffffffffffull; // setting it to max value as a flag
  define_parameter( "vis_stop", param_vis_stop);
  
  param_cost_reg_history = 1;
  define_parameter( "cost_reg_history", param_cost_reg_history);

  param_warmup_cycles = 0;
  define_parameter( "warmup_cycles", param_warmup_cycles);

  param_max_packets = 0;
  define_parameter( "max_packets", param_max_packets);

  param_node_bw_stats =0;
  define_parameter( "node_bw_stats", param_node_bw_stats);
  
  param_aggressive_vc_alloc = 0;
  define_parameter( "aggressive_vc_alloc", param_aggressive_vc_alloc);

  param_0delay_cost_msg_update = 0;
  define_parameter( "0delay_cost_msg_update", param_0delay_cost_msg_update);
  
  param_same_cycle_local_cost = 0;
  define_parameter( "same_cycle_local_cost", param_same_cycle_local_cost);
    
    
  param_cost_multiplier_local = 1;
  define_parameter( "cost_multiplier_local", param_cost_multiplier_local);
  
  param_cost_multiplier_remote = 1;
  define_parameter( "cost_multiplier_remote", param_cost_multiplier_remote);
  
  param_use_max_quadrant_cost = 0;
  define_parameter( "use_max_quadrant_cost", param_use_max_quadrant_cost);

  param_adaptive_1avail = 0;
  define_parameter( "adaptive_1avail", param_adaptive_1avail);

  param_cost_precision = 0;
  define_parameter( "cost_precision", param_cost_precision);
  
  param_edge_cost_fn = "max";
  define_parameter( "edge_cost_fn", param_edge_cost_fn);
  
  param_ignore_escape_vc = 0;
  define_parameter( "ignore_escape_vc", param_ignore_escape_vc);
  
  param_ignore_escape_vc_Ydir = 0;
  define_parameter( "ignore_escape_vc_Ydir", param_ignore_escape_vc_Ydir);
  
  param_ignore_escape_vc_Ydir_heuristic = 0;
  define_parameter( "ignore_escape_vc_Ydir_heuristic", param_ignore_escape_vc_Ydir_heuristic);
  
  param_low_bw_rca = 0;
  define_parameter( "low_bw_rca", param_low_bw_rca);
  
  param_low_bw_rca_latency = 0;
  define_parameter( "low_bw_rca_latency", param_low_bw_rca_latency);


  parse_netcfg();

  // Here setup vis start and stop if they are used
  if (param_vis_start == 0xffffffffffffffffull) {
    param_vis_start = param_warmup_cycles;
  }
  
  // speed things up if we are not really checking extents)
  if ((param_vis_start ==0) &&
      (param_vis_stop == 0xffffffffffffffffull)) {
      	
    vis_all = true;
  } else {
    vis_all = false;
  }

    
}

void ocin_top::create_generator() {

  // we will have a switch here eventually to pick the proper
  // generator...
  set<string>::iterator gentp_it;

  for(gentp_it = gen_types.begin(); gentp_it != gen_types.end(); gentp_it++) {

    ocin_gen * gen_ptr;

    if (gentp_it->compare("rand") == 0) {
      gen_ptr = new ocin_gen_rand(nodes, msg_map, node_inst_data, prertrs);
    } else if (gentp_it->compare("bitcomp") ==0) {
      gen_ptr = new ocin_gen_bitcomp(nodes, msg_map, node_inst_data, prertrs);
    } else if (gentp_it->compare("bitrev") ==0) {
      gen_ptr = new ocin_gen_bitrev(nodes, msg_map, node_inst_data, prertrs);
    } else if (gentp_it->compare("randpair") ==0) {
      gen_ptr = new ocin_gen_randpair(nodes, msg_map, node_inst_data, prertrs);
    } else if (gentp_it->compare("hotspot") ==0) {
      gen_ptr = new ocin_gen_hotspot(nodes, msg_map, node_inst_data, prertrs);
    } else if (gentp_it->compare("transpose") ==0) {
      gen_ptr = new ocin_gen_transpose(nodes, msg_map, node_inst_data, prertrs);
    } else if ((gentp_it->compare("file") ==0) || 
    
               (gentp_it->compare("file2") ==0) ||
               (gentp_it->compare("file3") ==0) ||
               (gentp_it->compare("file4") ==0)) {
      gen_ptr = new ocin_gen_file(nodes, msg_map, node_inst_data, prertrs);
    } else if (gentp_it->compare("filetm") ==0) {
      gen_ptr = new ocin_gen_filetm(nodes, msg_map, node_inst_data, prertrs); 
    } else if (gentp_it->compare("selfsim") ==0) {
      gen_ptr = new ocin_gen_selfsim(nodes, msg_map, node_inst_data, prertrs);
    } else {
      stringstream tmp;
      tmp << "Error unknown type of generator: " << gentp_it->c_str() << " (This will be fatal soon)";
  
      ocin_name_fatal(modname,tmp.str());
      exit(1);
    }

#ifdef DEBUG
    {
      stringstream tmp;
      tmp << "Creating generator of type: " << gentp_it->c_str() ;
      
      ocin_name_debug(modname,tmp.str());
    }
#endif


    gens[gentp_it->c_str()] = gen_ptr;

    // note: moved initialization to the creat_tiles method so we init
    // the generator after the tiles are made...
  }


  // When creating the tiles we will also place the sources and dests
  // in the gen.
}


void ocin_top::parse_netcfg() {
  // This method parses the netcfg file and populates the node_info
  // map with data found there. If no netcfg_file is specified a
  // generic OCN like configuration is generated on the fly.

  if (param_netcfg_file == "") {

#ifdef DEBUG
    {
      stringstream tmp;
      tmp << "Using default two node network";
      
      ocin_name_debug(modname,tmp.str());
    }
#endif


    // creating a very simple two node config

    // This is a one dimensional mesh...
    // RT0:
    node_inst_data["rt0"].name = "rt0";
    node_inst_data["rt0"].coord.push_back(0); // x = 0
    node_inst_data["rt0"].is_source = true;
    node_inst_data["rt0"].is_destination = true;
    node_inst_data["rt0"].src_type.push_back("rand");
    node_inst_data["rt0"].dst_type = "noblock";
    node_inst_data["rt0"].router_type = "basic";
    node_inst_data["rt0"].xbar_type = "fullcon";
    node_inst_data["rt0"].vc_alloc = "2level";
    node_inst_data["rt0"].xb_alloc = "2level";
    node_inst_data["rt0"].rt_algo_type = "xydor";
    node_inst_data["rt0"].rt_sel_type = "first_match";
    node_inst_data["rt0"].rt_cost_fn = "local";
    node_inst_data["rt0"].rt_cost_reg = "free_vc_nohist";  
    node_inst_data["rt0"].rt_cost_mgr = "local";
    node_inst_data["rt0"].vc_count = 4;
    node_inst_data["rt0"].vc_classes = 1;
    node_inst_data["rt0"].que_depth = 2;
    node_inst_data["rt0"].port_count = 2;
    node_inst_data["rt0"].port_dest.push_back("rt1");  // +x
    // also put it in set of gen_types
    gen_types.insert(node_inst_data["rt0"].src_type[0]);
    ejc_types.insert(node_inst_data["rt0"].dst_type);
        
    // RT1:
    node_inst_data["rt1"].name = "rt1";
    node_inst_data["rt1"].coord.push_back(1); // x = 1;
    node_inst_data["rt1"].is_source = true;
    node_inst_data["rt1"].is_destination = true;
    node_inst_data["rt1"].src_type.push_back("rand");
    node_inst_data["rt1"].dst_type = "noblock";
    node_inst_data["rt1"].router_type = "basic";
    node_inst_data["rt1"].xbar_type = "fullcon";
    node_inst_data["rt1"].vc_alloc = "2level";
    node_inst_data["rt1"].xb_alloc = "2level";
    node_inst_data["rt1"].rt_algo_type = "xydor";
    node_inst_data["rt1"].rt_sel_type = "first_match";
    node_inst_data["rt1"].rt_cost_fn = "local";
    node_inst_data["rt1"].rt_cost_reg = "free_vc_nohist";
    node_inst_data["rt1"].rt_cost_mgr = "local"; 
    node_inst_data["rt1"].vc_count = 4;
    node_inst_data["rt1"].vc_classes = 1;
    node_inst_data["rt1"].que_depth = 2;
    node_inst_data["rt1"].port_count = 2;
    node_inst_data["rt1"].port_dest.push_back("rt0");  // -x 
    // also put it in set of gen_types
    gen_types.insert(node_inst_data["rt1"].src_type[0]);
    ejc_types.insert(node_inst_data["rt1"].dst_type);

  } else {
    
#ifdef DEBUG
    {
      stringstream tmp;
      tmp << "Parsing network config file:" << param_netcfg_file;
      
      ocin_name_debug(modname,tmp.str());
    }
#endif

    ifstream myfile(param_netcfg_file.c_str());
    string line;

    myfile.is_open();
        
    if (myfile.fail()) {
      {
        stringstream tmp;
        tmp  << "Error: Unable to open file:" << param_netcfg_file ;     
        ocin_name_fatal(modname,tmp.str());
      }

      exit(0);
    }
        
    string curr_node = "_defaults";  // special node name for defaults

    while (!myfile.eof()) {
      getline(myfile, line);
      //      cout <<line <<endl;
      /* Skip comments */
      if (line[0] == '#') {
        continue;
      }

      istringstream working_line(line);


      // deal with "node:" lines
      if (line.find("node:",0) != string::npos) {
        string trash, old_node;
        old_node = curr_node;
        working_line >> trash >>curr_node;
#ifdef DEBUG
        {
          stringstream tmp;
          tmp << "Found node in file: " <<curr_node ;
      
          ocin_name_debug(modname,tmp.str());
        }
#endif

        // re-using defaults is a special case..
        if (curr_node != "_defaults") {

          // first double check that node is not already defined
          // anywhere...
          if (node_inst_data.find(curr_node.c_str()) !=  node_inst_data.end()) {
            stringstream tmp;
            tmp  << "Error: node: " << curr_node << " found more than once in file: " << param_netcfg_file ;     
            ocin_name_fatal(modname,tmp.str());
          

            exit(0);
          }

          // if not then instantiate it with a copy the default stuff
          node_inst_data[curr_node.c_str()] = node_inst_data["_defaults"];

          // and set its name:
          node_inst_data[curr_node.c_str()].name = curr_node;

        } // _defaults reuse needs to count ports on previous node too

        // go back and calc the number of ports in the old node
        node_inst_data[old_node.c_str()].port_count = 
          node_inst_data[old_node.c_str()].port_dest.size();


        continue;               // goto next line
      }

      // deal with clock lines, note: will error out if we do those
      // for the default node
      if  (line.find("clock",0) != string::npos) {
        string trash;
        tsim_u64 clock_period;
        working_line >> trash >> clock_period;

#ifdef DEBUG
        {
          stringstream tmp;
          tmp <<"Found clock: "<< clock_period<< " for node: "  <<curr_node ;
      
          ocin_name_debug(modname,tmp.str());
        }
#endif

        // first double check that we are not defining coords for the
        // default node.
        if (curr_node == "_defaults") {
          {
            stringstream tmp;
            tmp  << "Error: attempt to define clock for default node";     
            ocin_name_fatal(modname,tmp.str());
          }
          
          exit(0);
        }

        // if not then pushback the clock
        node_inst_data[curr_node.c_str()].clock_period = clock_period;
        continue;
      }

      // deal with coord lines, note: will error out if we do those
      // for the default node
      if  (line.find("coord",0) != string::npos) {
        string trash;
        int coord;
        working_line >> trash >> coord;

#ifdef DEBUG
        {
          stringstream tmp;
          tmp <<"Found coord: "<< coord<< " for node: "  <<curr_node ;
      
          ocin_name_debug(modname,tmp.str());
        }
#endif

        // first double check that we are not defining coords for the
        // default node.
        if (curr_node == "_defaults") {
          {
            stringstream tmp;
            tmp  << "Error: attempt to define coords for default node";     
            ocin_name_fatal(modname,tmp.str());
          }
          
          exit(0);
        }

        // if not then pushback the coord
        node_inst_data[curr_node.c_str()].coord.push_back(coord);
        continue;
      }

      // deal with port lines, note: will error out if we do those for
      // the default node
      if  ( (line.find("port",0) != string::npos) &&
      	    (line.find("_port",0) == string::npos)) {  // exclude phys_ports and logical_ports
        string trash, port;
        
        working_line >> trash >> port;

        if (trash == "lport") {
          continue;             // skip local port for now...
        }

#ifdef DEBUG
        {
          stringstream tmp;
          tmp  <<"Found port: "<< port<< " for node: "  <<curr_node ;
      
          ocin_name_debug(modname,tmp.str());
        }
#endif

        // first double check that we are not defining coords for the
        // default node.
        if (curr_node == "_defaults") {
          {
            stringstream tmp;
            tmp  << "Error: attempt to define ports for default node";     
            ocin_name_fatal(modname,tmp.str());
          }

          exit(0);
        }

        // if not then pushback the coord
        node_inst_data[curr_node.c_str()].port_dest.push_back(port.c_str());
        continue;
      }

      if  (line.find("is_not_source",0) != string::npos) {

        node_inst_data[curr_node.c_str()].is_source = false;

        continue;
      }

      // deal with is_source lines
      if  (line.find("is_not_destination",0) != string::npos) {

        node_inst_data[curr_node.c_str()].is_destination = false;

        continue;
      }

      // removed in favor of the src_type vector
      //       // deal with is_source lines
      //       if  (line.find("is_source2",0) != string::npos) {
      //         string trash, type;
        
      //         working_line >> trash >> type;

      //         // if not then pushback the coord
      //         node_inst_data[curr_node.c_str()].is_source2 = true;

      //         node_inst_data[curr_node.c_str()].src_type2 = type;

      //         // also put it in set of gen_types
      //         gen_types.insert(type);

      // #ifdef DEBUG
      //         {
      //           stringstream tmp;
      //           tmp <<"Found src_type: "<< type<< " for node: "  
      //               <<curr_node ;      
      //           ocin_name_debug(modname,tmp.str());
      //         }
      // #endif
      //         continue;
      //       }

      // deal with is_source lines
      if  (line.find("is_source ",0) != string::npos) {
        string trash, type;
        
        working_line >> trash >> type;

        // if not then pushback the coord
        node_inst_data[curr_node.c_str()].is_source = true;

        node_inst_data[curr_node.c_str()].src_type.push_back(type);

        // also put it in set of gen_types
        gen_types.insert(type);

#ifdef DEBUG
        {
          stringstream tmp;
          tmp <<"Found src_type: "<< type<< " for node: "  
              <<curr_node ;      
          ocin_name_debug(modname,tmp.str());
        }
#endif
        continue;
      }

      // deal with is_destination lines
      if  (line.find("is_destination",0) != string::npos) {
        string trash, type;
        
        working_line >> trash >> type;

        // if not then pushback the coord
        node_inst_data[curr_node.c_str()].is_destination = true;
        node_inst_data[curr_node.c_str()].dst_type = type;

        // also put it in set of ejc_types
        ejc_types.insert(type);

#ifdef DEBUG
        {
          stringstream tmp;
          tmp <<"Found dest_type: "<< type<< " for node: "  
              <<curr_node ;      
          ocin_name_debug(modname,tmp.str());
        }
#endif

        continue;
      }

      // deal with router_type lines
      if  (line.find("router_type",0) != string::npos) {
        string trash, type;
        
        working_line >> trash >> type;

#ifdef DEBUG
        {
          stringstream tmp;
          tmp <<"Found router_type: "<< type<< " for node: "  
              <<curr_node ;      
          ocin_name_debug(modname,tmp.str());
        }
#endif

        // set it in data
        node_inst_data[curr_node.c_str()].router_type = type;
        continue;
      }

      // deal with preroute_type lines
      if  (line.find("preroute_type",0) != string::npos) {
        string trash, type;
        
        working_line >> trash >> type;

#ifdef DEBUG
        {
          stringstream tmp;
          tmp <<"Found preroute_type: "<< type<< " for node: "  
              <<curr_node ;      
          ocin_name_debug(modname,tmp.str());
        }
#endif

        // set it in data
        node_inst_data[curr_node.c_str()].preroute_type = type;
        continue;
      }

      // deal with xbar_type lines
      if  (line.find("xbar_type",0) != string::npos) {
        string trash, type;
        
        working_line >> trash >> type;

#ifdef DEBUG
        {
          stringstream tmp;
          tmp  <<"Found xbar_type: "<< type<< " for node: "  
               <<curr_node ;
      
          ocin_name_debug(modname,tmp.str());
        }
#endif

        // set it in data
        node_inst_data[curr_node.c_str()].xbar_type = type;
        continue;
      }

      // deal with vc_alloc lines
      if  (line.find("vc_alloc",0) != string::npos) {
        string trash, type;
        
        working_line >> trash >> type;

#ifdef DEBUG
        {
          stringstream tmp;
          tmp <<"Found vc_alloc: "<< type<< " for node: "  
              <<curr_node ;
      
          ocin_name_debug(modname,tmp.str());
        }
#endif

        // set it in data
        node_inst_data[curr_node.c_str()].vc_alloc = type;
        continue;
      }

      // deal with xb_alloc lines
      if  (line.find("xb_alloc",0) != string::npos) {
        string trash, type;
        
        working_line >> trash >> type;

#ifdef DEBUG
        {
          stringstream tmp;
          tmp  <<"Found xb_alloc: "<< type<< " for node: " <<curr_node ;
          ocin_name_debug(modname,tmp.str());
        }
#endif


        // set it in data
        node_inst_data[curr_node.c_str()].xb_alloc = type;
        continue;
      }

      // deal with rt_algo_type lines
      if  (line.find("rt_algo_type",0) != string::npos) {
        string trash, type;
        
        working_line >> trash >> type;

#ifdef DEBUG
        {
          stringstream tmp;
          tmp  <<"Found rt_algo_type: "<< type<< " for node: " <<curr_node ;
          ocin_name_debug(modname,tmp.str());
        }
#endif
        
        // ensure that the VC reuse policy is compatible with the routing algo
        if (param_aggressive_vc_alloc) {
          if ((type != "xydor") && (type != "yxdor") && 
              (type != "xydor_bec") && (type != "o1turn_bec")) {
            stringstream t;
            t << "The selected routing function is "
              << "incompatible with aggressive VC allocation "
              << "(resulting network is deadlock prone) \n";
            t << "Only deterministic routing functions (eg: DOR) "
              << "are safe to use w/ aggressive VC allocation. \n";
            t << "Selected routing algo = " << type << endl; 
            
            ocin_name_fatal(modname, t.str() );
            exit(0);
          }
        } 
        
        // set it in data
        node_inst_data[curr_node.c_str()].rt_algo_type = type;
        continue;
      }
      
      // deal with rt_sel_type lines
      if  (line.find("rt_sel_type",0) != string::npos) {
        string trash, type;
        
        working_line >> trash >> type;

#ifdef DEBUG
        {
          stringstream tmp;
          tmp  <<"Found rt_sel_type: "<< type<< " for node: " <<curr_node ;
          ocin_name_debug(modname,tmp.str());
        }
#endif
        // set it in data
        node_inst_data[curr_node.c_str()].rt_sel_type = type;
        continue;
      }
      
      // deal with rt_cost_fn lines
      if  (line.find("rt_cost_fn",0) != string::npos) {
        string trash, type;
        
        working_line >> trash >> type;

#ifdef DEBUG
        {
          stringstream tmp;
          tmp  <<"Found rt_cost_fn: "<< type<< " for node: " <<curr_node ;
          ocin_name_debug(modname,tmp.str());
        }
#endif
        // set it in data
        node_inst_data[curr_node.c_str()].rt_cost_fn = type;
        continue;
      } 
    
    
      // deal with rt_cost_reg lines
      if  (line.find("rt_cost_reg",0) != string::npos) {
        string trash, type;
        
        working_line >> trash >> type;

#ifdef DEBUG
        {
          stringstream tmp;
          tmp  <<"Found rt_cost_reg: "<< type<< " for node: " <<curr_node ;
          ocin_name_debug(modname,tmp.str());
        }
#endif
        // set it in data
        node_inst_data[curr_node.c_str()].rt_cost_reg = type;
        continue;
      } 


      // deal with rt_cost_mgr lines
      if  (line.find("rt_cost_mgr",0) != string::npos) {
        string trash, type;
        
        working_line >> trash >> type;

#ifdef DEBUG
        {
          stringstream tmp;
          tmp  <<"Found rt_cost_mgr: "<< type<< " for node: " <<curr_node ;
          ocin_name_debug(modname,tmp.str());
        }
#endif
        // set it in data
        node_inst_data[curr_node.c_str()].rt_cost_mgr = type;
        continue;
      } 
            
          
      // deal with vc_count lines
      if  (line.find("vc_count",0) != string::npos) {
        string trash;
        int count;
        
        working_line >> trash >> count;
        
#ifdef DEBUG
        {
          stringstream tmp;
          tmp  <<"Found vc_count: "<< count<< " for node: "  <<curr_node ;
          ocin_name_debug(modname,tmp.str());
        }
#endif

        // set it in data
        node_inst_data[curr_node.c_str()].vc_count = count;
        continue;
      }

      // deal with vc_classes lines
      if  (line.find("vc_classes",0) != string::npos) {
        string trash;
        int count;
        
        working_line >> trash >> count;
        
#ifdef DEBUG
        {
          stringstream tmp;
          tmp  <<"Found vc_classes: "<< count<< " for node: " <<curr_node ;
          ocin_name_debug(modname,tmp.str());
        }
#endif
        
             

        // set it in data
        node_inst_data[curr_node.c_str()].vc_classes = count;
        continue;
      }

      // deal with que_depth lines
      if  (line.find("que_depth",0) != string::npos) {
        string trash;
        int count;
        
        working_line >> trash >> count;

#ifdef DEBUG
        {
          stringstream tmp;
          tmp  <<"Found que_depth: "<< count<< " for node: " <<curr_node ;
          ocin_name_debug(modname,tmp.str());
        }
#endif
        
        // set it in data
        node_inst_data[curr_node.c_str()].que_depth = count;
        continue;
      }

      /* Concentration */
      if  (line.find("concentration",0) != string::npos) {
        string trash;
        int concentration_degree;
        
        working_line >> trash >> concentration_degree;

#ifdef DEBUG
        {
          stringstream tmp;
          tmp  <<"Concentration: "<< concentration_degree << " for node: " << curr_node ;
          ocin_name_debug(modname,tmp.str());
        }
#endif
        
        // set it in data
        node_inst_data[curr_node.c_str()].terminals = concentration_degree;
        continue;
      }
      
      
      /* Connectivity */
      
      // Physical
      if (line.find("phys_ports",0) != string::npos) {
      	string trash;
      	vector <int> ports;
      	
      	// get rid of the config tag
      	working_line >> trash;
      	
      	int hops = 0;
      	while (!working_line.eof()) {
          int x;
          working_line >> x;
          ports.push_back(x);
          hops++;	
      	}

        // Ensure that ports are not duplicated.
        // This is also the simplest way to check for a space at the end of the config line,
        // which causes port duplication.  Stoooopid C++
        {
          stringstream tmp;
          int last_port = -1;
          for (int i=0; i<hops; i++)  { 
            tmp << ports[i] << " ";
          	
            if (ports[i] < last_port) {  // Redundant port found?
              stringstream tmp2;
              tmp2 << "Decreasing phys_port found for node " << curr_node << endl;
              tmp2 << "phys_ports: " << tmp.str();
          		
              ocin_name_fatal(modname,tmp2.str());
              exit(1);
            }
            last_port = ports[i];
          }
          
#ifdef DEBUG
          stringstream tmp2; 
          tmp2 << "Found phys ports for node: " << curr_node;
          tmp2 << tmp.str();
          ocin_name_debug(modname,tmp2.str());
#endif
        
          // DELETE
          cout << "Phys: " << tmp.str() << endl;
        }
		      	
        // set it in data
        node_inst_data[curr_node.c_str()].phys_ports = ports;
        continue;
      }
      
      // Logical
      if (line.find("logical_ports",0) != string::npos) {
      	string trash;
      	vector <int> ports;
      	
      	// get rid of the config tag
      	working_line >> trash;
      	
      	int hops = 0;
      	while (!working_line.eof()) {
          int x;
          working_line >> x;
          ports.push_back(x);
          hops++;	
      	}

        // Logical port count should perfectly match phys port count
        if (ports.size() != node_inst_data[curr_node.c_str()].phys_ports.size()) {
          stringstream tmp2;
          tmp2 << "Number of logical ports doesn't match # of physical ports. " << curr_node << endl;
          tmp2 << "Possible cause is an extra space in the config file on logical_ports line. ";
          ocin_name_fatal(modname,tmp2.str());
          exit(1);
        }
          
        {
          stringstream tmp;
          for (int i=0; i<hops; i++)  { 
            tmp << ports[i] << " ";
          }
          
#ifdef DEBUG
          stringstream tmp2; 
          tmp2 << "Found logical ports for node: " << curr_node;
          tmp2 << tmp.str();
          ocin_name_debug(modname,tmp2.str());
#endif
        
          // DELETE
          cout << "Logical: " << tmp.str() << endl;
        }
		      	
        // set it in data
        node_inst_data[curr_node.c_str()].logical_ports = ports;
        continue;
      }
      
      // Combined physical + logical via pre-specified configurations     
      if (line.find("pl_config",0) != string::npos) {
      	string trash, conf_str;
      	vector <int> pports, lports;
      	
      	// get rid of the config tag
      	working_line >> trash;
      	
      	// get the string corresponding to the config;
      	working_line >> conf_str;
      	// extract components of the config
      	// The basic format is:  pAlB[xC[byD]], where:
      	// p - # (A) of physical channels in the sim
      	// l - # (B) of logical channels
      	// x - multiplier (C) of the logical channels
      	// by - partitioning of destinations among logical channel.
      	//      Default is by1, so that each destination (up to max hop length)
      	//      is accessible from any logical channel. 
      	//      byN means that destinations are partitioned among the N (actually, xC) 
      	//      logical channels.
      	string chan_str;	// channel (pAlB) configuration
      	int chan_mult = 1;	// channel multiplier (xC)
      	int chan_by = 1; 	// byN
      	
      	string::size_type x_idx =  conf_str.find("x");
        string::size_type by_idx = conf_str.find("by");
      	if (x_idx != string::npos) {
          // extract channel multiplier (xC)
          chan_str = conf_str.substr(0, x_idx);
          istringstream buff(conf_str.substr(x_idx+1));
          buff >> chan_mult;
  			
          // make sure 'byN' is not set
          if (by_idx != string::npos) {
            stringstream tmp;
            tmp << "Invalid pl_config: both xC and byD cannot be specified. Config: " << conf_str;
            ocin_name_fatal(modname,tmp.str());
            exit(1);
          }
      	}
        else if (by_idx != string::npos) {	
          // extract destination partioning (byN)
          chan_str = conf_str.substr(0, by_idx);		
          istringstream buff(conf_str.substr(by_idx+2));
          buff >> chan_by;
        }
        else {
          chan_str = conf_str; 
        }
  		
      	int pport_cnt = 0;
      	int lport_cnt = 1;
      	if (chan_str.compare("p1l1") == 0)
          pport_cnt = lport_cnt = 1;
      	else if (chan_str.compare("p2l2") == 0)
          pport_cnt = lport_cnt = 2;
      	else if (chan_str.compare("p3l3") == 0)
          pport_cnt = lport_cnt = 3;
      	else if (chan_str.compare("p4l4") == 0)
          pport_cnt = lport_cnt = 4;
      	else if (chan_str.compare("p6l6") == 0) 
          pport_cnt = lport_cnt = 6;
      	else if (chan_str.compare("p8l8") == 0) 
          pport_cnt = lport_cnt = 8;
      	else if (chan_str.compare("p16l16") == 0) 
          pport_cnt = lport_cnt = 16;
      	else if (chan_str.compare("p2l1") == 0)
          pport_cnt = 2;
      	else if (chan_str.compare("p3l1") == 0)
          pport_cnt = 3;
        else if (chan_str.compare("p4l1") == 0)
          pport_cnt = 4;
      	else if (chan_str.compare("p6l1") == 0)
          pport_cnt = 6;
      	else if (chan_str.compare("p7l1") == 0)
          pport_cnt = 7;
      	else if (chan_str.compare("p8l1") == 0)
          pport_cnt = 8;
      	else if (chan_str.compare("p16l1") == 0)
          pport_cnt = 16;
      	else {
          stringstream tmp;
          tmp << "Invalid pl_config: " << conf_str;
          ocin_name_fatal(modname,tmp.str());
          exit(1);
      	}
      	
        cout << "pports: " << pport_cnt << ", lports: " << lport_cnt << ", mult: " << chan_mult << ", by: " << chan_by << endl;      	      	
      	// configure physical & logical ports
      	if ((chan_mult == 1) && (chan_by == 1)) {
          // simple config (mesh, EC, basic BEC)
          // eg:  phys:  1 2 3 4
          //      log:   1 1 2 2
          int lports_per_pport = pport_cnt / lport_cnt;
          for (int i=1; i<=pport_cnt; i++) {
            pports.push_back(i);
            int lport_num = ((i-1) / lports_per_pport) + 1;
            lports.push_back(lport_num);
          }
      	}
      	else if (chan_mult > 1) {
          // multiple BECs, all connecting same set of destinations
			
          // only 1 logical channel per x1 is supported
          if (lport_cnt > 1) {
            stringstream tmp;
            tmp << "Only 1 logical port is permitted in this config! Config: " << conf_str;
            ocin_name_fatal(modname,tmp.str());
            exit(1);
          }

          // configure physical & logical channels
          for (int i=1; i<=pport_cnt; i++) {
            for (int j=1; j<=chan_mult; j++) {
              pports.push_back(i);
              lports.push_back(j);
            }
          }
      	}
      	else {  // (chan_by>1)
      		// multiple BECs, each connecting a unique set of destinations

          // only 1 logical channel per by1 is supported
          if (lport_cnt > 1) {
            stringstream tmp;
            tmp << "Only 1 logical port is permitted in this config! Config: " << conf_str;
            ocin_name_fatal(modname,tmp.str());
            exit(1);
          }
          // configure physical & logical channels
          int by = 0;
          int pport_div = (pport_cnt - 1) / chan_by;
          int pport_max = (pport_div + 1) * chan_by;  
          for (int i=1; i<=pport_max; i++) {
            pports.push_back(i);
            lports.push_back(by+1);
            by = (by + 1) % chan_by;
          }
      	}
      	
      	// configure logical ports
      	// This supports multiple (but not all) phys ports per logical port
      	// eg:  phys:  1 2 3 4
      	//      log:   1 1 2 2
      	// Note that only strided combinations are supported.
        /*
          int lports_per_pport = pport_cnt / lport_cnt;
          for (int i=0; i<pport_cnt; i++) {
          int lport_num = (i / lports_per_pport) + 1;
          lports.push_back(lport_num); 
          } 
        */
      	// Sanity checks
      	if (pports.size() != lports.size()) {
          stringstream tmp;
          tmp << "P & L vectors are unequal size! " << endl;
          tmp << "  pports.size(): " << pports.size() << endl;
          tmp << "  lports.size(): " << lports.size() << endl;
          ocin_name_fatal(modname,tmp.str());
          exit(1);
      	}
      	
      	stringstream pstr, lstr;
        for (int i=0; i<pports.size(); i++)  { 
          pstr << pports[i] << " ";
          lstr << lports[i] << " ";
        }
        // DELETE
        cout << "Phys:    " << pstr.str() << endl;
      	cout << "Logical: " << lstr.str() << endl;
      	
      	// set it in data
        node_inst_data[curr_node.c_str()].phys_ports = pports;
        node_inst_data[curr_node.c_str()].logical_ports = lports;
        continue;
      }
    }

    // finally erase the defaults entry so we don't instantiate it.
    node_inst_data.erase("_defaults");
  }


  // setup a list of all node names & accomodate 2nd source (for hotspot)
  for (nid_it=node_inst_data.begin(); 
       nid_it != node_inst_data.end(); nid_it++) {
    
    string name = nid_it->first;

    // constructing the node_names
    // brg 6/4/08: this is now done in set_network_size()
//    node_names.push_back(name);

    // Also go through and set source2 to rand if we have a hotspot in
    // source 1 and source2 is not set.
    if (nid_it->second.is_source) {
      if (nid_it->second.src_type[0].compare("hotspot") ==0) {

        if (nid_it->second.src_type.size() == 1) { // only hotspot
                                                   // defined
          nid_it->second.src_type.push_back("rand");
          // also put it in set of gen_types
          gen_types.insert("rand");
        }
      }
     
      // If ocn_4proc is set then we want to create all the different
      // sources.
      if (param_ocn_4proc == 1) {
        //         nid_it->second.src_type.push_back("file2");
        //         gen_types.insert("file2");
        //         nid_it->second.src_type.push_back("file3");
        //         gen_types.insert("file3");
        //         nid_it->second.src_type.push_back("file4");
        //         gen_types.insert("file4");
      }
    }
    
    
  }  
  
}

void ocin_top::create_tiles () {

  // Here we create instances of the routers that form the network.
  // This is where we will parse the network connection files.  For now
  // I'm just making a square of x by y.


  int count = 0;
  stringstream *ss;

#ifdef DEBUG
  ocin_name_debug (modname, "Creating tiles...");
#endif

  ocin_prtr * prtr_ptr = NULL;

	  
  // configure concentration
  set_concentration();
  
  for (nid_it=node_inst_data.begin(); 
       nid_it != node_inst_data.end(); nid_it++) {
    
    ocin_node_info * info = &nid_it->second;
    string name = nid_it->first;
    
    // double check that the config file is right
    if (nodes.find(name) != nodes.end()) {
      {
        stringstream tmp;
        tmp  << "Error more than one "<< name << " in net config file";     
        ocin_name_fatal(modname,tmp.str());
      }

      exit(0);
    }
    
    // instantiate router and initialize it
    nodes[name].init(this, info);

    nodes[name].nodes = &(nodes);

    //    cout << "Nodes in top "<< nodes[name].nodes<<endl;

#ifdef DEBUG
    ss = new stringstream;
    *ss << name << " - xcoord: "<< info->coord[0] << " ycoord: " << info->coord[1] ; 
    ocin_name_debug (modname,ss->str());
    delete ss;
#endif

    // If this is a source or a dest tell the generator
    if (info->is_source) {
      for(int x=0; x < info->src_type.size(); x++) {
      	for (int t=0; t < info->terminals; t++) {
          string tname = info->terminal_names[t];
          gens[info->src_type[x]]->push_src(tname);
      	}
      }
    }

    if (info->is_destination) {
      for(gen_it = gens.begin(); gen_it != gens.end(); gen_it++) {
      	for (int t=0; t < info->terminals; t++) {
          string tname = info->terminal_names[t];
          gen_it->second->push_dst(tname);
      	}
      }
    }

    // Now setup the statistics in the node info
    info->stats.init(node_names, name);
    
    // Add switch for all types of prerouters as needed
    if (param_preroute_pkts == 1) {
      if (info->preroute_type.empty()) {
        {
          stringstream tmp;
          tmp  << "Error no prerouter specified for node "<< name;     
          ocin_name_fatal(modname,tmp.str());
        }
        exit(1);
      }

      // if tree for each prerouter type to create objects
      if (info->preroute_type.compare("xy") == 0) {
        if (prtr_ptr == NULL) { // we only want one of this type
          prtr_ptr  = new ocin_prtr_xy(node_inst_data, nodes);
        }
      } else if (info->preroute_type.compare("omni") == 0) {
        if (prtr_ptr == NULL) { // we only want one of this type
          prtr_ptr  = new ocin_prtr_omni(node_inst_data, nodes);
        }
      } else {
        stringstream tmp;
        tmp << "Error unknown type of prerouter: " << info->preroute_type.c_str();
  
        ocin_name_fatal(modname,tmp.str());
        exit(1);
      }

      prertrs[name] = prtr_ptr; // place prerouter ptr in prertrs

      nodes[name].prtr_ptr = prtr_ptr; // give a pointer to the node.

      // the gens already have a pointer to the prertrs structure...


    }

  }

  // Init the prerouters after all the nodes have been created:
  map<string, ocin_prtr *>::iterator prtr_it;
  prtr_ptr = NULL;
  ocin_prtr * last_ptr = NULL;
  for(prtr_it = prertrs.begin(); prtr_it != prertrs.end(); prtr_it++) {
    prtr_ptr = prtr_it->second;
    if (prtr_ptr != last_ptr) {
      stringstream prtr_name;
      prtr_name << "prerouter_"<< "_" <<count;
      prtr_it->second->init(this, prtr_name.str());

      count++;
      last_ptr = prtr_ptr;      // this is to make sure we only
                                // initialize each prerouter once
                                // because more than one node may have
                                // the same prerouter.
    }

  }



  // We have to initialize the generator and ejector after tiles so
  // that all the local_*u units are instatiated first...

  // first the generators
  set<string>::iterator gentp_it;
  count = 0;
  for(gentp_it = gen_types.begin(); gentp_it != gen_types.end(); gentp_it++) {

    ocin_gen * gen_ptr;
    gen_ptr = gens[gentp_it->c_str()];

    stringstream genname;

    genname << "generator_"<<gentp_it->c_str() << "_" << count;
	
    // set the terminal2node translation maps
    gen_ptr->terminal2node_map = terminal2node_map;
    gen_ptr->terminal2port_map = terminal2port_map;

    

    gen_ptr->init(this, genname.str());
    count++; 
  }

  // now the ejectors:
  set<string>::iterator ejc_it;
  count = 0;
  for(ejc_it = ejc_types.begin(); ejc_it != ejc_types.end(); ejc_it++) {

    ocin_ej * ejc_ptr;
    ejc_ptr = ejectors[count];

    stringstream ejname;
    ejname <<"ejector_"<< ejc_it->c_str() << "_" <<count;
    ejc_ptr->init(this, ejname.str());
    count++;
  }



  // ********* Next_hop_input_ports are not used anymore
  // // now go through and construct the set of links to neighbor's iu's,
  // // can't do this in the main loop in internal wires because the iu's
  // // have not be instantiated yet there.

  // for (nid_it=node_inst_data.begin(); 
  //      nid_it != node_inst_data.end(); nid_it++) {
  //   ocin_node_info *info;     
  //   info = &nid_it->second;
  //   string name = nid_it->first;

  //   int dimensions = info->coord.size();
  //   int current_idx = 0;
    
  //   // pre-init the iu array with nulls
  //   int port_cnt = DIRECTIONS * DIMENSIONS * info->phys_ports.size();
  //   ocin_router_iu * null_ptr =  NULL;
  //   node_inst_data[name].next_hop_input_ports.assign(port_cnt, null_ptr); 

  // }

}		


// This function just sets up the visualization stuff
void ocin_top::create_vis () {

  // needs a pointer to the inst data structure at instatiation time
  //  vis_ptr = new ocin_vis(node_inst_data,nodes);

  // vis_ptr->init();


}
  

void ocin_top::create_internal_wires () {


  /* Initialize all input and output links to NULL */
  for (nid_it=node_inst_data.begin(); 
       nid_it != node_inst_data.end(); nid_it++) {
    // grab a pointer to this node's node_info struct
    ocin_node_info * info = &nid_it->second;
    
    // Total ports = (# of dims) * (# of dirs/dim) * (# of phys ports/dir)
    //             = 2 * 2 * phys_ports
    int port_cnt = DIRECTIONS * DIMENSIONS * info->phys_ports.size();
       	    
    // DELETE
    if (nid_it == node_inst_data.begin())
      cout << "Network port count = " << port_cnt << endl;

    ocin_channel * null_ptr =  NULL;
    info->output_links.assign(port_cnt, null_ptr); 
    info->input_links.assign(port_cnt, null_ptr);
            
#ifdef DEBUG
    {
      stringstream tmp;
      tmp  << "initing input & output links to a total of " << port_cnt << " wires";
      ocin_name_debug(modname,tmp.str());
    }
#endif
   	
  }
  
  /* Connect outputs to inputs */
  for (nid_it=node_inst_data.begin(); 
       nid_it != node_inst_data.end(); nid_it++) {

    // grab a pointer to this node's node_info struct
    ocin_node_info * info = &nid_it->second;
    string name = info->name;
    int i_coord = info->coord[0];
    int j_coord = info->coord[1];
    
    // Total ports = (# of dims) * (# of dirs/dim) * (# of phys ports/dir)
    //             = 2 * 2 * phys_ports
    int port_cnt = DIRECTIONS * DIMENSIONS * info->phys_ports.size();    
    
    int current_idx = 0;  // index into the output_links vector of size port_cnt
    int dir = 1;          // determines the offset polarity (+-) relative to current node
    
    for (int i = 0; i<DIMENSIONS; i++) {
      for (int j = 0; j<DIRECTIONS; j++) {
      	for (int k = 0; k<info->phys_ports.size(); k++) {
          int hop_dist = info->phys_ports[k];  // next hop distance
          int hop_offset = dir * hop_dist;     // hop offset takes dir (+-) into account
          // determine the node this offset corresponds to
          stringstream d;
          if (current_idx < (port_cnt / 2)) 
            d << "" << (i_coord+hop_offset) << "." << j_coord;
          else 
            d << "" << i_coord << "." << (j_coord+hop_offset);
          // destination (downstream port) node name
          string dest = d.str();
          // destination port index
          int dest_idx = current_idx + (info->phys_ports.size() * dir);
 
          // DELETE
          //cout << name << ": Next node = " << dest << ". Curr idx = " << current_idx << ", dest idx = " << dest_idx << endl;

          // check if such node exists
          if (node_inst_data.find(dest) == node_inst_data.end()) {
            // offset falls outside the mesh - set input & output links to NULL
            info->output_links[current_idx] = NULL;
            info->input_links[current_idx] = NULL;	
          }
          else {
            // create a new channel
            ocin_channel * new_wire = new ocin_channel;
            wires.push_back(new_wire);
              	
            // Sanity check: both output port at this node and input port downstream 
            //               should not be connected.
            if (info->output_links[current_idx] != NULL) {
              stringstream tmp;
              tmp  << name << ". Error: output port already connected! Index: " << current_idx ;     
                  
              ocin_name_fatal(modname,tmp.str());
              exit(0);
            }
            if (node_inst_data[dest].input_links[dest_idx] != NULL) {
              stringstream tmp;
              tmp  << name << ". Error: downstream input port already connected at node: " << dest << ", index: " << current_idx ;     
                  
              ocin_name_fatal(modname,tmp.str());
              exit(0);
            }

            // connect output port at this node w/ input port downstream
            info->output_links[current_idx] = new_wire;  
            node_inst_data[dest].input_links[dest_idx] = new_wire;
              	
            // Compute the wire delay, taking # of hops spanned into account
            int wire_delay;
            if (param_hops_per_cycle > 0) {
              int cycles = (int)(ceil( (double)hop_dist / param_hops_per_cycle ));
              wire_delay = cycles * param_wire_delay;
            }
            else {
              wire_delay = param_wire_delay;
            } 
              	
            // Initialize the wire
            stringstream *ss = new stringstream();
            *ss << name << "->" << dest;
            new_wire->init(ss->str(), wire_delay);

            // DELETE
            //cout << name << ": Connecting output " << k << " (idx " << current_idx << ") to node " << dest << " (idx " << dest_idx << ")" << endl;
            //cout << "  " << ss->str() << ": " << wire_delay << ". Hops = " << hop_dist << endl;;

#ifdef DEBUG
            {
              stringstream tmp;
              tmp  << "Connecting output " << k << " to node " << dest;
              ocin_name_debug(modname,tmp.str());
            }
#endif 
            // Finally recreate the port_dest structure here
            info->port_dest.push_back(dest);


          }

          current_idx++;
      	}
      	dir *= -1;
      }       
    }

    // sanity check: make sure output and input link count is equal to port count
    if ( (info->output_links.size() != port_cnt) ||
         (info->input_links.size() != port_cnt)) 
      {
        stringstream tmp;
        tmp << "Port count not equal to link count!" << endl;
        tmp << "  port count = " << port_cnt << endl;
        tmp << "  output links = " << info->output_links.size() << endl;
        tmp << "  input links = " << info->input_links.size() << endl;
		
      	ocin_name_fatal(modname,tmp.str());
      	exit(1); 
      }
	
    // now we set the port count correctly in the info
    info->port_count = port_cnt;
        
#ifdef DEBUG
    {
      stringstream tmp;
      tmp  << "Setting node " << name << " port count = " << port_cnt;
      ocin_name_debug(modname,tmp.str());
    }
#endif

  }
}


void ocin_top::create_ejector() {

  set<string>::iterator ejc_it;


  for(ejc_it = ejc_types.begin(); ejc_it != ejc_types.end(); ejc_it++) {

    ocin_ej * ejc_ptr;

    if (ejc_it->compare("noblock") == 0) {
      ejc_ptr =  new ocin_ej_noblock(nodes, msg_map, node_inst_data);

    } else {
      stringstream tmp;
      tmp << "Error unknown type of ejector: " << ejc_it->c_str();
  
      ocin_name_fatal(modname,tmp.str());
      continue;
    }

#ifdef DEBUG
    {
      stringstream tmp;
      tmp << "Creating ejector of type: " << ejc_it->c_str() ;
      
      ocin_name_debug(modname,tmp.str());
    }
#endif

    ejectors.push_back(ejc_ptr);

  }

  
}


void ocin_top::evaluate() {
  // empty in top level
}

void ocin_top::update() {
  ocin_cycle_count = this->sim()->cycles() + 1;

  // kill the simulation if all the messages we are watching have been
  // injected and completed
  if((ocin_gen::gen_done) && (msg_map.empty())) {
    this->sim()->stop();
  }
  
  
  // visualization
  if (ocin_cycle_count % param_stats_interval == 0) {      
//     if(param_vis_on ==1) {
//       if (vis_all) {
//         vis_ptr->print(false);
//       } 
//       else {
//         if ((ocin_cycle_count >= param_vis_start) &&
//             (ocin_cycle_count <= param_vis_stop)) {
//           vis_ptr->print(false);
//         }
//       }
//     }
    
    // Temporarily moving this out from under the exclusvity w/
    // checkpointed monitors.  This should be fine as long as we are
    // only printing packet stats...
    if (param_incr_stats==1) 
      print_stats(true);        // temporarily non-cumulative
    
  }

  // checkpointing for cost vars
  if ((param_incr_chkpt==1) && (ocin_cycle_count % param_chkpt_interval == 0)) { 
    checkpoint_monitors();
  }

  
}

void ocin_top::finalize() {

  if(param_vis_on ==1) {
    // if vis is enabled, always print final pic
//    vis_ptr->print(true);
    /*
      if (vis_all) {

      } else {
      if ((ocin_cycle_count >= param_vis_start) &&
      (ocin_cycle_count <= param_vis_stop)) {
      vis_ptr->print(true);
      }
      }
    */    
  }      

  print_stats(true);            // print the final totals

  
  deadlock_check();             // any suspected deadlock??
}


void ocin_top::deadlock_check() {
  bool is_starved = false;
  bool kill_flag = false;
  
  // iterate over all nodes
  for(nodes_it = nodes.begin(); nodes_it != nodes.end(); nodes_it++) {
    ocin_router * nodept = &(nodes_it->second);
    
    for (int i=0; i<nodept->vc_fifos.size(); i++) {       // for all ports
      for (int j=0; j<nodept->vc_fifos[i].size(); j++) {   
        ocin_mon_vc_fifo *fmon = nodept->vc_fifos[i][j].mon;
        
#ifdef DEBUG
        // { Alittle to verbose...
        //   stringstream tmp;
        //   tmp << fcns(fmon->name) << "Last deque: " << (ocin_cycle_count - fmon->last_deque_cycle);
        //   tmp << ", empty = " << fmon->_fifo->is_empty();
        //   tmp << ", is_starved() = " << fmon->is_starved() << endl;
          
        //   ocin_name_debug(modname,tmp.str());
        // }
#endif
        
        if (fmon->is_starved()) {
          stringstream ss;
          ss << endl;
          ss << fcns(fmon->name) << "Possible starvation/deadlock detected!!! \n";
          ss << "\t Last deque cycle = " << fmon->last_deque_cycle;
          ss << " (" << (ocin_cycle_count - fmon->last_deque_cycle) << " cycles ago) \n";
          cout << ss.str();
          
          kill_flag = true;
        }
      } 
    }	// for all ports
  } // for all nodes in the network
  
  // if (kill_flag)
  //   exit(0);                    // replaced assert to avoid dumpfiles
  kill_flag = false;
  if (kill_flag) {
    map<int, ocin_msg*>::iterator msg_it; 
    for(msg_it = msg_map.begin(); msg_it != msg_map.end(); msg_it++) {
      // only print the "counted" packets
      if (msg_it->second->pid >= 0) {

        cout << "Hung packet: ";
        cout << msg2str(msg_it->second, true) << endl;

      }

    }
  }

}



void ocin_top::checkpoint_monitors() {
  // Walk all nodes
  for(nodes_it = nodes.begin(); nodes_it != nodes.end(); nodes_it++) {
    ocin_router * nodept = &(nodes_it->second);
    ocin_node_info *node_info = nodept->node_info;

    // Checkpoint VC FIFOs
    for (int i=0; i<nodept->vc_fifos.size(); i++) {
      for (int j=0; j<nodept->vc_fifos[i].size(); j++) {
        nodept->vc_fifos[i][j].mon->checkpoint(ocin_cycle_count);
        //nodept->vc_fifos[i][j].mon->clear_running();  
      }
    }
    
    // Checkpoint port monitors
    int total_ports = node_info->port_count + 1;  // inc. local port
  
    // Input ports
    for (int i=0; i<total_ports; i++) {     // iterate over input ports
      for (int j=0; j<total_ports; j++) {   // iterate over out ports @
        nodept->pin_mon[i][j].checkpoint();
      }
    }
  
    // Output ports
    for (int i=0; i<total_ports; i++) {     // iterate over input ports
      for (int j=0; j<total_ports; j++) { // iterate over out ports @
        nodept->pout_mon[i][j].checkpoint();
      }
    }
  }

  // TODO: checkpoint node BW stats
}


/* Set the size of the network based on either the number of nodes
 * in the config file or the specified size
 */
void ocin_top::set_network_size() {
  map<string, ocin_node_info>::reverse_iterator rit = node_inst_data.rbegin();
  ocin_node_info *info = &rit->second;
  int concentration_factor = info->terminals;

  // Infer mesh size from the total number of terminals & concentration factor
  if (param_network_terminals != 0) {
    int terminals_per_dim = (int)(sqrt(param_network_terminals));
    int concentration_per_dim = (int)sqrt(concentration_factor);
    int nodes_per_dim = terminals_per_dim / concentration_per_dim;  
  	
    // Mesh must be square for this config method to work
    if (param_network_terminals % terminals_per_dim != 0) {
      stringstream tmp;
      tmp << "The use of 'network_terminals' param requires a square mesh. param_network_terminals: " << param_network_terminals << endl;
      ocin_name_fatal(modname,tmp.str());
      exit(1);
    } 
    // Set num_x_tiles & num_y_tiles.
    // The actual network is configured below
  	
    param_num_x_tiles = param_num_y_tiles = nodes_per_dim;
  }
  
  
  if ((param_num_x_tiles > 0) && (param_num_y_tiles > 0)) {
    // Mesh size is specified via num_x_tiles and num_y_tiles. 
    // Prune extra nodes specified in the config file
    for (nid_it=node_inst_data.begin(); 
         nid_it != node_inst_data.end(); ) {
      // grab a pointer to this node's node_info struct
      ocin_node_info * info = &nid_it->second;
      string name = info->name;
      int x_coord = info->coord[0];
      int y_coord = info->coord[1];
	    
      if ((x_coord >= param_num_x_tiles) ||
          (y_coord >= param_num_y_tiles)) 
        {
          //cout << "Erasing node" << name << endl;
          node_inst_data.erase(nid_it++);
        }
      else {
        // valid node. Add to names list
        node_names.push_back(name);
        
        ++nid_it;
      }
    }
  }
  else {
    // Infer mesh size from the topology expressed node-by-node in the config file.
    // Figure out the dimensions of the mesh from the coordinates of the last node
    param_num_x_tiles = info->coord[0] + 1;
    param_num_y_tiles = info->coord[1] + 1;
  }
  
  // print config
  int mesh_size = param_num_x_tiles * param_num_y_tiles;
  cout << "Mesh size: " << param_num_x_tiles << " x " << param_num_y_tiles;
  cout << " = " << mesh_size;
  cout << " * " << concentration_factor << "(Concentration) = " << mesh_size * concentration_factor;
  if (param_network_terminals != 0)
    cout << " (inferred!)";
  cout << endl;
}


/* configure concentration */
void ocin_top::set_concentration() {
  nid_it = node_inst_data.begin();
  ocin_node_info *info = &nid_it->second;
  
  // Concentration: build a list of terminal ports at each network node
  // The following assumes that the concentration factor is the same
  //  in both X and Y dims for each network node.
  int concentration_per_dim = (int)sqrt(info->terminals);
  // Concentration factor must be perfect square
  if (info->terminals % concentration_per_dim != 0) {
    stringstream tmp;
    tmp << "Concentration factor must be a perfect square (eg: 1,4,9). Concentration: " << info->terminals << endl;
    ocin_name_fatal(modname,tmp.str());
    exit(1);
  } 
  	 
  int terminals_x = param_num_x_tiles * concentration_per_dim;
  int terminals_y = param_num_y_tiles * concentration_per_dim;
	
  for (int x=0; x<terminals_x; x++) {
    for (int y=0; y<terminals_y; y++) {
      // map the terminal to a network node
      int x_coord = x / concentration_per_dim;
      int y_coord = y / concentration_per_dim;
		
      stringstream name, tname;
      name << "" << x_coord << "." << y_coord;
      tname << "term." << x << "." << y;
  		
      ocin_node_info *info = &(node_inst_data[name.str()]);
 
      info->terminal_names.push_back(tname.str());
      vector <int> v;
      v.push_back(x);
      v.push_back(y);
      info->terminal_coords.push_back(v);
  		
      // update terminal translation maps
      terminal2node_map[tname.str()] = name.str();
      terminal2port_map[tname.str()] = info->terminal_names.size() - 1; 
    }
  }
  
  cout << "Network terminals: " << terminal2node_map.size() << endl;
  
  /*  
  // DELETE
  map<string, string>::iterator it;
  for (it=terminal2node_map.begin(); it != terminal2node_map.end(); it++) {
  string tname = it->first;
  string nname = it->second;
  cout << "Term: " << tname << ", node: " << nname << ". port: " << terminal2port_map[tname] << endl;
  }
  */
}


/* Compute per-channel BW given the bi-section BW and channel count */
void ocin_top::set_bandwidth() {    
  /* adjust percent_inj_bw for flit count, channel width and Concentration */
  ocin_node_info * info = &(node_inst_data.begin()->second);
  int concentration_per_dim = (int)sqrt(info->terminals);
  // adjust bisection BW by concentratio amount
  param_scaled_bisection_width = param_baseline_channel_width * concentration_per_dim;
  param_bisection_channel_count = bisection_channel_count();
  
  param_bisection_channel_width = param_scaled_bisection_width / param_bisection_channel_count;
	
  // ave num of flits per packet:
  double temp_inj_bw1;
  param_avg_pkt_size = 0;

  if (param_random_req_size) {
    temp_inj_bw1 = (((double)param_flit_max-1)/2.0) +1.0;
  }
  else if (param_bimodal_req_size) {

    // Original
    /*
      param_bimodal_size1_flits = ceil( (double)param_bimodal_size1 / param_bisection_channel_width);
      param_bimodal_size2_flits = ceil( (double)param_bimodal_size2 / param_bisection_channel_width);
    
      temp_inj_bw1 = ((double)param_bimodal_size1_flits + param_bimodal_size2_flits) / 2.0;
    */

    // Normalized to mesh's injection rate *in flits*

    // compute flit sizes wrt unconcentrated mesh to determine the injection rate (normalized to mesh)
    param_bimodal_size1_flits = (int)ceil( (double)param_bimodal_size1 / param_baseline_channel_width);
    param_bimodal_size2_flits = (int)ceil( (double)param_bimodal_size2 / param_baseline_channel_width);
    
    temp_inj_bw1 = ((double)param_bimodal_size1_flits + param_bimodal_size2_flits) / 2.0;
    
    // compute actual flit sizes for given topology
    param_bimodal_size1_flits = (int)ceil( (double)param_bimodal_size1 / param_bisection_channel_width);
    param_bimodal_size2_flits = (int)ceil( (double)param_bimodal_size2 / param_bisection_channel_width);    

    // Normalized to mesh's injection rate *in bits*
    /*
      param_avg_pkt_size = (unsigned)((param_bimodal_size1 + param_bimodal_size2) / 2.0);
      //	temp_inj_bw1 =  1 / (double)param_avg_pkt_size;  // pointless, but makes it compatible w/ other calculations
      temp_inj_bw1 = 1 / (double)param_baseline_channel_width; 
    */	                                                 // for computing param_adj_inj_bw below
  }
  else {
    temp_inj_bw1 = (double) param_flit_max;
  }


  param_adj_inj_bw = ((double)param_percent_inj_bw/100.0) / temp_inj_bw1; 

  // 6/30/08 pg - I'm dividing by the number of generators so we get
  // an even split when there is more than one gen (IE hotspot)

  param_adj_inj_bw = param_adj_inj_bw/(double)gen_types.size();

#ifdef DEBUG
  {
    stringstream tmp;
    tmp << "Scaled adjusted inj_bw number is: " << param_adj_inj_bw ;
  
    ocin_name_debug(modname,tmp.str());
  }
#endif

  // DELETE
  cout << "Scaled BW = " << param_adj_inj_bw <<endl;
}


/* Compute the bisection channel count given a topology
 * and logical connectivity
 */
int ocin_top::bisection_channel_count() {
	
  // The calculation below assumes a square mesh and only looks at X-dir bisection
  // Notify the user if the mesh is non-square
  if (param_num_x_tiles != param_num_y_tiles) {
    stringstream tmp;
    tmp << "Non-square mesh specified." << endl;
    tmp << "The current bisection channel count only works for square meshes (ignores the Y-dir)." << endl;
    tmp << "Adjust top::bisection_channel_count() to support non-square meshes." << endl;
  
    ocin_name_fatal(modname,tmp.str());
    exit(1);
  }
	
  int bisection = param_num_y_tiles / 2;
  int bisection_ch_cnt = 0;
	
  for (nid_it=node_inst_data.begin(); 
       nid_it != node_inst_data.end(); nid_it++) {
    // grab a pointer to this node's node_info struct
    ocin_node_info *info = &nid_it->second;
    
    int x_coord = info->coord[0];
    int y_coord = info->coord[1]; 
    if ((x_coord == 0) &&
        (y_coord < bisection)) {
      // iterate over all outbound channels and count
      // how many cross the bisection.
      // The trick is to count all physical channels mapping
      // to the same logical channel as 1 ch.
      map <int, bool> lch_map; 
      for (int i=0; i<info->logical_ports.size(); i++) {
        int lch = info->logical_ports[i];
        // channel counted?
        if (lch_map[lch] == true)
          continue;
	    	
        // channel spans bisection?
        int phys_span = info->phys_ports[i];
        int dest = y_coord + phys_span;
        if ((dest < param_num_y_tiles) &&
            (dest >= bisection)) {
          bisection_ch_cnt++;
          lch_map[lch] = true;
	    		
          cout << "Counting logical channel " << lch << ". Cnt = " << bisection_ch_cnt << endl;
        } 
	    	
      }
    }
  }
    
  return bisection_ch_cnt;
}


void ocin_top::print_stats(bool totals) {
  cout << "\n\n";
  if (!totals) {
    cout << "Incremental Stats at cycle: "<<ocin_cycle_count <<endl;
  } else {
    cout << "Totals Stats at cycle: "<<ocin_cycle_count <<endl;
  }

  int pkt_count = msg_map.size();
  cout << "Current number of monitored packets in the network: "<< pkt_count <<endl; 
  cout << "Number of uncounted packets: "<< (-1*ocin_gen::temp_pid) <<endl;

  if(param_max_packets >0) {
    tsim_u64 left;
    if (ocin_gen::pid >0) {
      left = param_max_packets - ocin_gen::pid;
    } else if (ocin_gen::pid == -1){
      left = param_max_packets;
    } else {
      left = 0;
    }
        
    cout << "Number of packets left to inject: "<<left<<endl;
  }

  // first gather stats for the nodes:
  if(param_node_stats ==1) {
    for(nodes_it = nodes.begin(); nodes_it != nodes.end(); nodes_it++) {
      
      ocin_router * nodept = &(nodes_it->second);
      
      ocin_node_info *node_info = nodept->node_info;
      
      cout << fcs() << "Stats for: "<< nodes_it->first <<endl;
      
      
      /* Print out stats for the VC FIFOs */
      
      // Note: to exclude Ejector fifos, iterate until (i<first_ej_idx)

      int avg_fifo_size, fifo_size_sum;
      int num_fifos;    // # of fifos @ given port
      ocin_mon_vc_fifo fmon;
      
      for (int i=0; i<nodept->vc_fifos.size(); i++) {
        fmon.clear();
        
        stringstream s;
        s << "*" << node_info->name << ".";
       	if (i == node_info->port_count) {
          s << "INJ_MON";
        }
        else if (i == node_info->port_count+1) {
          s << "EJ_MON"; 
        }
        else {
          s << "VC" << i << "_MON";
        }
        fifo_size_sum = 0;
        num_fifos=0;
        for (int j=0; j<nodept->vc_fifos[i].size(); j++, num_fifos++) {
          // Print the current VC FIFO
          //nodept->vc_fifos[i][j].mon->smart_print(ocin_cycle_count, true /*print all*/, false /*indent*/, totals);
          
          // Aggregate the stats
          fmon += *(nodept->vc_fifos[i][j].mon);

          // Track the combined size of all composed fifos for subsequent averaging
          fifo_size_sum += nodept->vc_fifos[i][j].mon->get_fifo_size();
          
          // Checkpoint the monitor
          nodept->vc_fifos[i][j].mon->checkpoint(ocin_cycle_count);
        }
        // Find the avg size of the composed fifos & init the new monitor
        avg_fifo_size = fifo_size_sum / num_fifos;
        fmon.init(s.str(), avg_fifo_size);
        // Scale the stats based on the number of composed monitors
        fmon.scale_stats(num_fifos);
        
        fmon.smart_print(ocin_cycle_count, true, false, totals);
      }
      
      
      /* Port monitor stats */
      
      int total_ports = node_info->port_count + 1;  // inc. local port
      ocin_mon_port pmon;     // used to add up all input or output port
      // monitors
      
      cout << "\n";
      
      // First, get stats for all input ports
      for (int i=0; i<total_ports; i++) {     // iterate over input ports
        pmon.clear();
        stringstream s;
        s << "*PIN" << i << "_MON." << node_info->name;
        pmon.init(s.str() );
        for (int j=0; j<total_ports; j++) {   // iterate over out ports @
          // each in port
          pmon += nodept->pin_mon[i][j];
          nodept->pin_mon[i][j].smart_print(false /*print a monitor even
                                                    if it saw no
                                                    activity?*/, 
                                            true /*verbose - print
                                                   detailed VC & XB
                                                   stats?*/,
                                            totals); 
          nodept->pin_mon[i][j].checkpoint();
        }
        pmon.smart_print(false /*skip if inactive*/, true /*verbose*/, totals);
      }
      
      cout << "\n";
      
      // Stats for output ports
      for (int i=0; i<total_ports; i++) {     // iterate over input ports
        pmon.clear();
        stringstream s;
        s << "*POUT" << i << "_MON." << node_info->name;
        pmon.init(s.str() );
        for (int j=0; j<total_ports; j++) { // iterate over out ports @
          // each in port
          pmon += nodept->pout_mon[i][j];
          nodept->pout_mon[i][j].smart_print(false /*print a monitor
                                                     even if it saw no
                                                     activity?*/, 
                                             true /*verbose - print
                                                    detailed VC & XB
                                                    stats?*/,
                                             totals);
          nodept->pout_mon[i][j].checkpoint();
        }
        pmon.smart_print(false /*skip if inactive*/, true /*verbose*/, totals);
      }
    }
  }

  if(param_node_bw_stats == 1) {
    bool header = true;
    cout<<"Node by node offered BW (Y=src, X=dst):\n";
    // first print the offered BW:
    for(vector<string>::iterator name_it = node_names.begin();
        name_it != node_names.end(); name_it++) {
      node_inst_data[*name_it].stats.print(node_names, header,false,totals,
                                           gens.begin()->second->mon->inj_end, 
                                           gens.begin()->second->mon->chpt_inj_end);
      
      if (header) {
        header = false;
      }
      
      
    }
    
    
    // print the accepted BW:
    cout<<"\nNode by node accepted BW (Y=src, X=dst):\n";
    header = true;
    
    for(vector<string>::iterator name_it = node_names.begin();
        name_it != node_names.end(); name_it++) {
      node_inst_data[*name_it].stats.print(node_names, header,true,totals, 
                                           gens.begin()->second->mon->inj_end, 
                                           gens.begin()->second->mon->chpt_inj_end);
      
      if (header) {
        header = false;
      }
      node_inst_data[*name_it].stats.checkpoint();  
      

    }
    

    // Print channel utilization (raw flit counts)
    for(int x=0; x <wires.size(); x++) {
      if (totals) {
        cout << wires[x]->name << ", " <<wires[x]->mon.get_all_flit_count()<<endl;
      } else {
        cout << wires[x]->name << ", " <<wires[x]->mon.get_chpt_flit_count()<<endl;
      }
    }
   
  }
  
  // now use packet mons in the generators to handle those stats...
  ocin_mon_packet total_stats;
  total_stats.init("Combined stats", nodes.size());
  if (totals) {

    for(gen_it = gens.begin(); gen_it != gens.end(); gen_it++) {
      gen_it->second->mon->print(totals); // print individual gen
                                          // stats

      total_stats += *gen_it->second->mon; // sum up totals
      
    }
    // Print combined stats summed up for all monitors
    if (gens.size() > 1) 
      total_stats.print(totals);

  } else {

    // checkpoint the packet stats

    for(gen_it = gens.begin(); gen_it != gens.end(); gen_it++) {
      gen_it->second->mon->print(totals); // print individual gen
                                          // stats
      total_stats += *gen_it->second->mon; // sum up totals

      gen_it->second->mon->checkpoint(); // checkpoint
    }
    // Print combined stats summed up for all monitors
    if (gens.size() > 1) 
      total_stats.print(totals);  // print totals
  }

}


void ocin_top::print_config() {
  // Are monitors off?
#ifndef OCIN_MON
  {
    cout << endl << "** All monitoring disabled **" << endl; 
  }
#endif


  // Assume that all routers are configured the same
  // and grab node_info descriptor for rt0
  nid_it=node_inst_data.begin(); 
  ocin_node_info *node_info = &nid_it->second;

    
  // Error checking
  if (param_ignore_escape_vc && param_ignore_escape_vc_Ydir) {
    stringstream tmp;
    tmp << "Conflicting parameters specified:";
    tmp << " both ignore_escape_vc and ignore_escape_vc_Ydir are set";
    ocin_name_fatal(modname,tmp.str());
    exit(1);
  }
  if (param_low_bw_rca) {
    if (param_low_bw_rca_latency < 2) {
      stringstream tmp;
      tmp << "Bad parameter specified:";
      tmp << " Low BW RCA requires the update frequency to be greater than every cycle.";
      ocin_name_fatal(modname,tmp.str());
      exit(1);
    }
  }

  string flit_size_prefix = param_random_req_size ? "1-" : "";
    
  if (node_info != NULL) {
    cout << endl;
    cout << "=== Sim config: \n";
    cout << "  config file:         \t" << param_netcfg_file <<endl;
    cout << "  channel BW (base):   \t" << param_baseline_channel_width << endl;
    cout << "  channel BW (scaled): \t" << param_scaled_bisection_width << endl;
    cout << "    channels:          \t" << param_bisection_channel_count << endl;
    cout << "    BW/ch:             \t" << param_bisection_channel_width << endl;
    if (!param_bimodal_req_size)
      cout << "  max flit size:       \t" << flit_size_prefix << param_flit_max << endl;
    else {
      cout << "  bimodal flit sizes:  \t" << param_bimodal_size1 << " & " << param_bimodal_size2 << " bits" << endl;
      cout << "          flit count:  \t" << param_bimodal_size1_flits << " & " << param_bimodal_size2_flits << endl;
    }
    cout << "  injection BW:        \t" << param_percent_inj_bw << "% \n";
    if (param_inj_vc_count != 1)
      cout << "  injection VC count:  \t" <<  param_inj_vc_count << endl;
    cout << "  ejection VC count:   \t" <<  param_ej_vc_count << endl;
    cout << "  aggressive VC alloc: \t" << param_aggressive_vc_alloc << endl;
    if (param_incr_chkpt)
      cout << "  chkpt interval:      \t" << param_chkpt_interval <<endl;
    //		cout << "  ignore escape VC:    \t" << param_ignore_escape_vc << endl;
    //		cout << "  ignore escape VC Ydir\t" << param_ignore_escape_vc_Ydir << endl; 
    //		cout << "     Ydir heuristic    \t" << param_ignore_escape_vc_Ydir_heuristic << endl;
    //		if (param_cost_reg_history != 1)
    //			cout << "  cost reg history:    \t" << param_cost_reg_history << endl;
    //		cout << "  adaptive_1avail:     \t" << param_adaptive_1avail << endl;
    //		cout << "  0-lat cost msg upd.:  " << ((param_0delay_cost_msg_update == 0) ? "false" : "true") << endl;
    //		cout << "  same-cycle cost upd.: " << ((param_same_cycle_local_cost == 0) ? "false" : "true") << endl;
    //		if (node_info->rt_cost_mgr == "quadrant")
    //			cout << "  use max quadrnt cost: " << ((param_use_max_quadrant_cost == 0) ? "false" : "true") << endl;
    cout << "  router pipeline lat  \t" << param_router_pipeline_latency << endl;
    cout << "  wire delay           \t" << param_wire_delay << endl;
    if (param_hops_per_cycle > 0)
      cout << "  hops per cycle       \t" << param_hops_per_cycle << endl;
    //cout << "  cost mult local:     \t" << param_cost_multiplier_local << endl;
    //cout << "  cost mult remote:    \t" << param_cost_multiplier_remote << endl;
    //		cout << "  cost precision       \t" << param_cost_precision << endl;
    //		cout << "  edge cost fn:        \t" << param_edge_cost_fn << endl;
    //		if (node_info->rt_cost_mgr != "local") {
    //			cout << "  low-BW RCA:      \t" << param_low_bw_rca << endl;
    //			if (param_low_bw_rca)
    //				cout << "  low-BW RCA freq: \t" << param_low_bw_rca_latency << endl;
    //		}
	
    // Print router config options.
    string ss = param_selfsim_inj ? " (self-sim)" : "";   // self-sim injection process?
	     
    cout << endl;
    cout << "=== Node config: \n";
    for(int x=0; x < node_info->src_type.size(); x++) {
      cout << "  Traffic type:   \t" << node_info->src_type[x] << ss << endl;
    }
    cout << "  Routing fn:     \t" << node_info->rt_algo_type << endl;
    cout << "  Route sel fn:   \t" << node_info->rt_sel_type << endl;
    //    	cout << "  Route cost fn:  \t" << node_info->rt_cost_fn << endl;
    //    	cout << "  Route cost reg: \t" << node_info->rt_cost_reg << endl;
    cout << "  Route cost mgr: \t" << node_info->rt_cost_mgr << endl;
    cout << "  XB Alloc:       \t" << node_info->xb_alloc << endl;
    cout << "  VCs/channel:    \t" << node_info->vc_count << endl;
    cout << "  VC FIFO depth:  \t" << node_info->que_depth << endl;
    cout << "  Concentration   \t" << node_info->terminals << endl;
  }
    
  cout << endl;
}
