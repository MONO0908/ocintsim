#include "ocin_router.h"


void ocin_router::init (tsim_object *parent, ocin_node_info *node_dsc ) {
  string name = node_dsc->name;
  tsim_u64 clock_period = node_dsc->clock_period;
  tsim_module::init (parent, name, clock_period );

  top_p = (ocin_top *) parent;
  node_info = node_dsc;

#ifdef DEBUG
  {
    stringstream tmp;
    tmp  << "Initializing router ";
    ocin_name_debug(name,tmp.str());
  }
#endif

  _terminals = node_info->terminals;
  _ports = node_info->port_count;
  int total_ports = _ports + 1;  // includes local port
  int vc_count = node_info->vc_count;
  int i, j;
  stringstream *ss;
  
  
  /* VC FIFOs*/
  // @ input ports
  int buffers = node_info->que_depth;
  for (i=0; i<_ports; i++) {
    vector <ocin_router_vc_fifo> *v = new vector <ocin_router_vc_fifo>();
    vc_fifos.push_back(*v); 
    for (j=0; j<vc_count; j++) {
      ss = new stringstream();
      *ss << "VC" << i << "_" << j;
      ocin_router_vc_fifo *f = new ocin_router_vc_fifo();
      f->init(this              /*parent router*/, 
              ss->str()         /*name*/, 
              buffers           /* # of buffers */,
              j                 /*vc index*/,
              false             /* not an injector */,
              false             /* not an ejector */ );
      (vc_fifos.back()).push_back(*f);
      delete ss;
    }
  }

  // @ injection port
  buffers = node_info->que_depth;  // TODO: we should have a dedicated # for the injection port
  for (i=0; i<_terminals; i++) {
    vector <ocin_router_vc_fifo> *injf = new vector <ocin_router_vc_fifo>();
    vc_fifos.push_back(*injf);
    for (j=0; j<param_inj_vc_count; j++) {
      ss = new stringstream();
      *ss << "VC_INJ" << i << "_" << j;
      ocin_router_vc_fifo *f = new ocin_router_vc_fifo();
      f->init(this                /* parent router */, 
              ss->str()           /* name */,
              buffers             /* # of buffers */,
              j                   /* vc index */,
              true                /* is injector */,
              false               /* not an ejector */ );
      (vc_fifos.back()).push_back(*f);
      delete ss;
    }
	  
    // record the index of the first injector fifo
    if (i==0)
      first_inj_fifo_idx = vc_fifos.size()-1;
  }

  
  // @ ejection port
  // Note: this is a special set of VC FIFOs in that they are considered 
  //       to be at "next hop."  Their purpose is to model the finite
  //       buffer capacity at the receiver.
  buffers = node_info->que_depth;  // TODO: we should have a dedicated # for the ejection port
  for (i=0; i<_terminals; i++) {
    vector <ocin_router_vc_fifo> *ejf = new vector <ocin_router_vc_fifo>();
    ej_fifos.push_back(*ejf);
    for (j=0; j<param_ej_vc_count; j++) {
      ss = new stringstream();
      *ss << "VC_EJ" << i << "_" << j;
      ocin_router_vc_fifo *f = new ocin_router_vc_fifo();
      f->init(this                /* parent router */, 
              ss->str()           /* name */,
              buffers             /* # of buffers */,
              j                   /* vc index */,
              false               /* not an injector */,
              true                /* ejection port */ );
      (ej_fifos.back()).push_back(*f);
      delete ss;
    }
  }

  // assign each VC a unique number
  number_vcs();
  
  // deal with logical ports
  create_lports();
  
  // Set each fifo's monitor's pointer to the parent fifo.
  // Doing it from within the fifo's init() method via &*this
  // doesn't seem to work (compiles but pointers are wrong @ runtime)
  for (int i=0; i<vcid2vc.size(); i++) {
    ocin_router_vc_fifo *v = vcid2vc[i]; 
    v->mon->set_fifo_ptr(v);
  }
  
  
  /* downstream VC status vectors */
  // @ output ports
  for (i=0; i<_ports; i++) {
    // each vc_status vector will be initialized by corresponding input units.
    vector <ocin_vc_status> *vc_stat = new vector <ocin_vc_status>();
    downstream_vc_stats.push_back(*vc_stat);
	  
    deque <int> *free_vc_q = new deque<int>();
    free_vc_list.push_back(*free_vc_q);
  }
  // @ ejection port
  for (i=0; i<_terminals; i++) {
    vector <ocin_vc_status> *vc_stat = new vector <ocin_vc_status>();
    downstream_vc_stats.push_back(*vc_stat);
	  
    deque <int> *free_vc_q = new deque<int>();
    free_vc_list.push_back(*free_vc_q);
  }
  
  
  /* Output unit */
  for (i=0; i<_ports; i++) {
    // 	  ocin_router_ou *ou = new ocin_router_ou( /*not used!*/ downstream_vc_stats[i]);
    ocin_router_ou *ou = new ocin_router_ou();
    output_units.push_back(*ou);
    ss = new stringstream();
    *ss << "OU" << i;
    output_units[i].init(this /*parent router*/, 
                         ss->str() /*name*/, 
                         node_info->output_links[i] /*channel out*/);
    delete ss;
	  
    // For each VC @ the corresponding input port,
    // set the credit port to point to this output port
    for (int j=0; j<vc_fifos[i].size(); j++) {
      vc_fifos[i][j].credit_port = &output_units[i];
    }
  } 

  /* Ejection port (acts like an OU) */
  int ej_idx = _ports;
  for (i=0; i<node_info->terminals; i++, ej_idx++) {
    ss = new stringstream();
    *ss << "EJ" << i;

    ocin_router_ej *ou = new ocin_router_ej(downstream_vc_stats[ej_idx], 
                                            free_vc_list[ej_idx],
                                            ej_fifos[i]);
    local_ou.push_back(*ou);
    local_ou[i].init(this      /*parent router*/, 
                     ss->str() /*name*/);
                     
    delete ss;
  }
  
  /* Routing unit */
  // @ input port
  // Map strings to ints via a map so that we can do a switch
  string ru_type = node_info->rt_algo_type;
  map <string, Routing_Algos> ru_map;
  ru_map["xydor"] = XY_DOR;
  ru_map["yxdor"] = YX_DOR;
  ru_map["xydor_bec"] = XY_DOR_BEC;
  ru_map["o1turn_bec"] = O1TURN_BEC;
  ru_map["adaptive_xy"] = ADAPTIVE_XY; 
  ru_map["preroute"] = PREROUTE; 
  

  switch (ru_map[ru_type]) {
  case XY_DOR:
    routing_units = new ocin_router_ru_xydor[total_ports];
    break;
  case YX_DOR:
    routing_units = new ocin_router_ru_yxdor[total_ports];
    break;
  case ADAPTIVE_XY:
    routing_units = new ocin_router_ru_adaptive_xy[total_ports];
    break;
  case XY_DOR_BEC:
    routing_units = new ocin_router_ru_xydor_bec[total_ports];
    break;
  case O1TURN_BEC:
    routing_units = new ocin_router_ru_o1turn_bec[total_ports];
    break;
  case PREROUTE:
    routing_units = new ocin_router_ru_preroute[total_ports];
    break;
  case UNKNOWN: 
    {
      stringstream tmp;
      tmp  << "Unknown routing algo: " << ru_type;     
      ocin_name_fatal(node_info->name,tmp.str());
    }
    
    exit(0);
  }
  // initialize the Routing Units
  // @input ports
  for (i=0; i<_ports; i++) {
    ss = new stringstream();
    *ss << "RU_" << ru_type << "_" << i;
    routing_units[i].init(this /*parent router*/, ss->str() /*name*/);
    delete ss;
  }  
  // @ inj port  
  // assume same RU type as before; ow, ru_type needs to be something different
  {
    inj_ru = &routing_units[_ports];
    ss = new stringstream();
    *ss << "RU_" << ru_type << "_LOCAL";
    inj_ru->init(this, ss->str());
    delete ss;
  }
  
  
  /* Route selection units (Sel, Cost, etc) */
  
  // Cost Reg (1 per output port)
  if (node_info->rt_cost_mgr.compare("quadrant") == 0) {
    cost_regs = *(new vector <vector <ocin_router_cost_reg> >(4));
  }
  else {
    cost_regs = *(new vector <vector <ocin_router_cost_reg> >(1));
  }
  
  // Init the cost regs
  for (int i=0; i<cost_regs.size(); i++) {  // for each quadrant
    for (j=0; j<_ports; j++) {          // for each port 
      ocin_router_cost_reg *cost_reg = new ocin_router_cost_reg();
      cost_regs[i].push_back(*cost_reg);
      ss = new stringstream();
      if (cost_regs.size() > 1) 
        *ss << "COST_REG_" << quadrant2str(static_cast<Quadrant>(i)) << "_" << i;
      else 
        *ss << "COST_REG_" << j;
      cost_regs[i][j].init(this /*parent router*/, 
                           ss->str() /*name*/, 
                           j /*pout idx*/);
      delete ss;  
    }
  	
    /* Ejection port (acts like an OU) */
    {
      int ej_idx = _ports;
      ocin_router_cost_reg *cost_reg = new ocin_router_cost_reg();
      cost_regs[i].push_back(*cost_reg);
      ss = new stringstream();
      if (cost_regs.size() > 1) 
        *ss << "COST_REG_EJ_" << quadrant2str(static_cast<Quadrant>(i));
      else 
        *ss << "COST_REG_EJ";
      cost_regs[i][ej_idx].init(this /*parent router*/, 
                                ss->str() /*name*/, 
                                ej_idx /*pout idx*/); 
      delete ss;  	
    }  
  } 

  
  // Cost/congestion manager
  if (node_info->rt_cost_mgr.compare("local") == 0) {
    cost_mgr = new ocin_router_cost_mgr_local(cost_regs);
  }
  else if (node_info->rt_cost_mgr.compare("1D") == 0) {
    cost_mgr = new ocin_router_cost_mgr_1D(cost_regs);
  }
  else if (node_info->rt_cost_mgr.compare("fanin") == 0) {
    cost_mgr = new ocin_router_cost_mgr_fanin(cost_regs);
  }
  else if (node_info->rt_cost_mgr.compare("quadrant") == 0) {
    cost_mgr = new ocin_router_cost_mgr_quadrant(cost_regs);
  }
  else if (node_info->rt_cost_mgr.compare("1domni") == 0) {
    cost_mgr = new ocin_router_cost_mgr_1domni(cost_regs);
  }
  else if (node_info->rt_cost_mgr.compare("none") == 0) {
    cost_mgr = NULL;
  }
  else {
    stringstream tmp;
    tmp  << "Invalid Cost Mgr type: " << node_info->rt_cost_mgr << "\n";     
    ocin_name_fatal(node_info->name,tmp.str());
    exit(0); 
  }
 
  if (cost_mgr) {
    ss = new stringstream();
    *ss << "CONG_MGR";
    cost_mgr->init(this, ss->str() );
    delete ss;
  }

  
  // Cost function
  route_cost = new ocin_router_cost(downstream_vc_stats);
  ss = new stringstream();
  *ss << "COST";
  route_cost->init(this, ss->str() );
  delete ss;
  

  // Selector
  if (node_info->rt_sel_type.compare("first_match") == 0) {
    route_sel = new ocin_router_sel_1match(route_cost);
  }
  else if (node_info->rt_sel_type.compare("last_match") == 0) {
    route_sel = new ocin_router_sel_lastmatch(route_cost);
  }
  else if (node_info->rt_sel_type.compare("bec") == 0) {
    route_sel = new ocin_router_sel_bec(route_cost);
  }
  else if (node_info->rt_sel_type.compare("o1turn") == 0) {
    route_sel = new ocin_router_sel_o1turn(route_cost);
  }
  else if (node_info->rt_sel_type.compare("first_avail") == 0) {
    route_sel = new ocin_router_sel_1avail(route_cost);
  }
  else if (node_info->rt_sel_type.compare("no_turn") == 0) {
    route_sel = new ocin_router_sel_noturn(route_cost);
  }
  else if (node_info->rt_sel_type.compare("local") == 0) {
    route_sel = new ocin_router_sel_local(route_cost);
  }
  else if (node_info->rt_sel_type.compare("stat") == 0) {
    route_sel = new ocin_router_sel_stat(route_cost);
  }
  else if (node_info->rt_sel_type.compare("random") == 0) {
    route_sel = new ocin_router_sel_random(route_cost);
  }
  else {
    {
      stringstream tmp;
      tmp  << "Invalid Sel type: " << node_info->rt_sel_type << "\n";     
      ocin_name_fatal(node_info->name,tmp.str());
    }
    exit(0); 
  }
  ss = new stringstream();
  *ss << "SEL";
  route_sel->init(this, ss->str() );
  delete ss;

  
  /* Input Units */
  for (i=0; i<_ports; i++) {
    ss = new stringstream();
    *ss << "IU" << i;
    ocin_router_iu *iu = new ocin_router_iu(downstream_vc_stats[i], 
                                            free_vc_list[i], 
                                            vc_fifos[i],
                                            cost_mgr);
    input_units.push_back(*iu);
    input_units[i].init(this                      /*parent router*/, 
                        ss->str()                 /*name*/,
                        i                         /* port number*/, 
                        node_info->input_links[i] /*channel in*/,
                        &routing_units[i]         /*routing unit */);
	  
    delete ss;
  }


  /* Injection port (acts like an IU)*/
  for (i=0; i<node_info->terminals; i++) {
    ss = new stringstream();
    *ss << "INJ" << i;
  	
    int inj_idx = first_inj_fifo_idx + i;
    ocin_router_inj *iu = new ocin_router_inj(vc_fifos[inj_idx]);
    local_iu.push_back(*iu);
    local_iu[i].init(this, 
                     ss->str(),
                     //gen_ptr,
                     inj_ru);
    delete ss;
  }
  
  
  /* Port monitors */
  for (i=0; i<total_ports; i++) {
    vector <ocin_mon_port> *pin_m = new vector <ocin_mon_port>();
    vector <ocin_mon_port> *pout_m = new vector <ocin_mon_port>();
    for (j=0; j<total_ports; j++) {
      stringstream si;
      si << node_info->name << ".PIN" << i << "_MON" << j;
      ocin_mon_port *im = new ocin_mon_port();
      im->init(si.str());
      pin_m->push_back(*im);

      stringstream so;
      so << node_info->name << ".POUT" << i << "_MON" << j;
      ocin_mon_port *om = new ocin_mon_port();
      om->init(so.str());
      pout_m->push_back(*om);
    }
    pin_mon.push_back(*pin_m);
    pout_mon.push_back(*pout_m);
  }

  
  /* VC Allocator */
  // TODO: no way to select the VC allocator is provided. Add mechanism similar to RU select.
  if (node_info->vc_alloc.compare("2level") == 0) {
    vc_allocator = new ocin_router_vcalloc_2level(vc_fifos, 
                                                  ej_fifos,
                                                  free_vc_list,
                                                  pin_mon,
                                                  pout_mon);
  }
  else {
    {
      stringstream tmp;
      tmp  << "Invalid VC alloc type: " << node_info->vc_alloc << "\n";     
      ocin_name_fatal(node_info->name,tmp.str());
    }
    
    exit(0); 
  }
  ss = new stringstream();
  *ss << "VC_ALLOC";
  vc_allocator->init(this, ss->str() );
  delete ss;

   
  /* X-bar */
  if (node_info->xbar_type.compare("fullcon") == 0) {
    xbar = new ocin_router_xbar(vc_fifos, output_units /*by ref*/, local_ou /*by ref*/);
  }
  else {
    {
      stringstream tmp;
      tmp  << "Invalid X-bar type: " << node_info->xbar_type << "\n";     
      ocin_name_fatal(node_info->name,tmp.str());
    }

    exit(0); 
  }
  ss = new stringstream();
  *ss << "XBAR";
  xbar->init(this, ss->str() );
  delete ss;
  

  /* X-bar Allocator */
  // TODO: no way to select the X-bar allocator is provided. Add mechanism similar to RU select.
  spec_xb_alloc = false;
  sequential_alloc = false;
  if (node_info->xb_alloc.compare("2level") == 0) {
    xb_allocator = new ocin_router_xballoc_2level(vc_fifos, 
                                                  downstream_vc_stats,
                                                  free_vc_list,
                                                  pin_mon,
                                                  pout_mon, 
                                                  xbar);
  }
  else if (node_info->xb_alloc.compare("spec_2level") == 0) {
    xb_allocator = new ocin_router_xballoc_speculative_2level(vc_fifos, 
                                                              downstream_vc_stats,
                                                              free_vc_list,
                                                              pin_mon,
                                                              pout_mon,
                                                              xbar);
    spec_xb_alloc = true;
  }
  else if (node_info->xb_alloc.compare("bec_2level") == 0) {
    xb_allocator = new ocin_router_xballoc_bec_2level(vc_fifos,
                                                      lvc_fifos, 
                                                      downstream_vc_stats,
                                                      free_vc_list,
                                                      pin_mon,
                                                      pout_mon,
                                                      xbar);
    sequential_alloc = true;
  }
  else if (node_info->xb_alloc.compare("bec_power_2level") == 0) {
    xb_allocator = new ocin_router_xballoc_bec_power_2level(vc_fifos,
                                                            lvc_fifos, 
                                                            downstream_vc_stats,
                                                            free_vc_list,
                                                            pin_mon,
                                                            pout_mon,
                                                            xbar);
    sequential_alloc = true;
  }
  else if (node_info->xb_alloc.compare("bec_spec_2level") == 0) {
    xb_allocator = new ocin_router_xballoc_bec_spec_2level(vc_fifos,
                                                           lvc_fifos, 
                                                           downstream_vc_stats,
                                                           free_vc_list,
                                                           pin_mon,
                                                           pout_mon,
                                                           xbar);
    spec_xb_alloc = true;
  }
  else {
    {
      stringstream tmp;
      tmp  << "Invalid X-bar alloc type: " << node_info->xb_alloc << "\n";     
      ocin_name_fatal(node_info->name,tmp.str());
    }

    exit(0); 
  }
  ss = new stringstream();
  *ss << "XB_ALLOC";
  xb_allocator->init(this, ss->str() );
  delete ss;
  
    
#ifdef DEBUG
  {
    stringstream tmp;
    tmp  << "Finished initializing router ";
    ocin_name_debug(name,tmp.str());
  }
#endif

}


/* number_vcs():
 *  - assign each VC in the router a unique number
 *  - make a mapping from each VC id to its input port & priority class
 *  - first, all network VCs are numbered, next inj VCs, and then ej VCs
 */
void ocin_router::number_vcs() {
  int id=0;
  int port=0;
  vector <vector <ocin_router_vc_fifo> >::iterator vc_fifos_i;
  //vector <ocin_router_vc_fifo>::iterator vc_fifos_j;
  vector <ocin_router_vc_fifo>::iterator vc_fifos_k;
  
  // iterate over all input vc_fifos (inc. injection port)
  for (vc_fifos_i=vc_fifos.begin(); vc_fifos_i != vc_fifos.end(); vc_fifos_i++, port++) {
    // iterate over classes (vc_fifos_j) here
	
    for (vc_fifos_k=vc_fifos_i->begin(); vc_fifos_k != vc_fifos_i->end(); vc_fifos_k++) {
      vc_fifos_k->vc_id = id;
      vc_fifos_k->vc_port = port; // set the physical port
      vcid2port.push_back(port);
      vcid2vcidx.push_back(vc_fifos_k->vc_index);
      //vcid2class[id] = p_class;
      vcid2vc.push_back(&(*vc_fifos_k));
		
      //cout << "vc_id = " << id << ", idx = " << vc_fifos_k->vc_index << ", port = " << port << "\n"; 
      id++;
    } // for all VCs at this port & class
    // for all classes
  } // for all ports
  
  // record the index of the first ejector fifo
  first_ej_idx = id;
  
  // iterate over fifos @ the ejection port
  int local_port = _ports;
  
  for (vc_fifos_i=ej_fifos.begin(); vc_fifos_i != ej_fifos.end(); vc_fifos_i++) {
    // iterate over classes (vc_fifos_j) here
    for (vc_fifos_k=vc_fifos_i->begin(); vc_fifos_k != vc_fifos_i->end(); vc_fifos_k++) {
      vc_fifos_k->vc_id = id; 	  		
      vc_fifos_k->vc_port = local_port; // set the physical port
      vcid2port.push_back(local_port);
      vcid2vcidx.push_back(vc_fifos_k->vc_index);
      //vcid2class[id] = p_class;
      vcid2vc.push_back(&(*vc_fifos_k));
		
      //cout << "vc_id = " << id << ", idx = " << vc_fifos_k->vc_index << ", port = " << local_port << "\n"; 
      id++;
    } // for all VCs at the ejector port
    local_port++;
    // for all classes
  } 
  
} // number_vcs()


/* create_lports():
 *  - assign logical port number to each VC
 *  - group VCs into logical ports
 *  - remember that ports are composed of network + injection ports
 *    (ejection ports are considered "downstream")
 */
void ocin_router::create_lports() {
	
  // determine # of logical ports per direction
  map <int, int> lport_map;
  for (int i=0; i<node_info->logical_ports.size(); i++) {
    int lport = node_info->logical_ports[i];
    lport_map[lport]++;
  }
  int lports_per_dir = lport_map.size();
  // x4 to account for the four cardinal directions
  int lport_cnt = lports_per_dir * 4;

  // allocate the data struct for lport vc fifos
  vector <vector <ocin_router_vc_fifo *> > *t;
  t = new vector <vector <ocin_router_vc_fifo *> > (lport_cnt); 
  lvc_fifos = *t;
	
  // populate the lvc_fifos
  int lport;
  int vc_id = 0;
  int dirs = DIMENSIONS * DIRECTIONS;
	
  for (int i = 0; i<dirs; i++) {
    int lport_offset = i * lports_per_dir;
    for (int j = 0; j<node_info->phys_ports.size(); j++) {
      // Logical port index (config file starts port numbering at 1)
      int lport_idx = node_info->logical_ports[j] - 1;
      lport = lport_idx + lport_offset;  
      for (int k = 0; k<vc_fifos[j].size(); k++, vc_id++) {
        // grab a pointer to the VC
        ocin_router_vc_fifo *vc = vcid2vc[vc_id];
        vc->vc_lport = lport; 
        vcid2lport.push_back(lport);
        vcid2lvcidx.push_back(lvc_fifos[lport].size());
  			
        // add the VC to its logical port
        lvc_fifos[lport].push_back(vc);
        // DELETE
        //cout << node_info->name << ": VC_ID " << vc_id << ", pport " << vc->vc_port << ", lport " << lport << ", k " << k << ", lports " << lport_cnt << endl;
			
	  		
			
        // Paranoid check
        if ( (j + (i*node_info->phys_ports.size() )) != vcid2port[vc_id]) {
          stringstream tmp;
          tmp  << "Port mismatch! j = " << (j + (i*node_info->phys_ports.size()));
          tmp  << ", vcid2port = " << vcid2port[vc_id];     
          ocin_name_fatal(node_info->name,tmp.str());
          exit(0);
        }
			
#ifdef DEBUG
        {
          stringstream tmp;
          tmp  << "VC_ID " << vc_id << ", pport " << vc->vc_port << ", l_port " << vc->vc_lport;
          ocin_name_debug(node_info->name,tmp.str());
        }
#endif
      }
    }
  }

  // Add the injectors as a logical port
  for (int inj_idx = first_inj_fifo_idx; inj_idx < vc_fifos.size(); inj_idx++) {
    vector <ocin_router_vc_fifo *> *inj = new vector <ocin_router_vc_fifo *>();
    lvc_fifos.push_back(*inj);
    // set the logical port number
    lport++;
    	
    for (int i=0; i<vc_fifos[inj_idx].size(); i++) {
      // grab a pointer to the inj VC
      int vc_id = vc_fifos[inj_idx][i].vc_id;
      ocin_router_vc_fifo *vc = vcid2vc[vc_id];

      vc->vc_lport = lport;
      vcid2lport.push_back(lport);
      vcid2lvcidx.push_back(lvc_fifos[lport].size());
      //cout << node_info->name << ": VC_ID " << vc_id << ", pport " << vc->vc_port << ", lport " << lport << ", lidx " << vcid2lvcidx[vc_id] << ", lports " << lport_cnt << endl;
      // add the VC to logical VC fifos
      (lvc_fifos.back()).push_back(vc);
    }
  }
    
  /* Map ports to lports */
  for (int i=0; i<vc_fifos.size(); i++) {
    ocin_router_vc_fifo *vc = &vc_fifos[i][0]; 
    port2lport.push_back(vc->vc_lport);
  }
} 


/* select_output_ports()
 *  - When multiple output ports are available, perform selection
 *    prior to VC arbitration.
 */
void ocin_router::select_output_ports() {
  vector <vector <ocin_router_vc_fifo> >::iterator vc_fifos_i;
  vector <ocin_router_vc_fifo>::iterator vc_fifos_k;
  for (vc_fifos_i=vc_fifos.begin(); vc_fifos_i != vc_fifos.end(); vc_fifos_i++) {
    for (vc_fifos_k=vc_fifos_i->begin(); vc_fifos_k != vc_fifos_i->end(); vc_fifos_k++) {
      if (vc_fifos_k->needs_vc) {
        ocin_msg *msg = vc_fifos_k->front_msg();

        // get the dest_port for the requesting VC
        int p_out = route_sel->select(msg);

        // Update the routing info for the input FIFO w/ the requested output port.
        vc_fifos_k->p_out = p_out;
      } 
    }
  } 
} // select_output_ports() 


void ocin_router::evaluate() {
  // Evaluate occurs first each cycle.  This is where you would grab
  // data from incomming wires and perform whatever work you want to
  // do in the cycle.
  int _ports = node_info->port_count;
  int i;

  // TODO: where in the cycle do we process credits???
  //       current model: credits become available @ the beginning of the cycle
  
  // local injection port
  for (i=0; i<_terminals; i++) {
    local_iu[i].receive();
    if ((param_repreroute_cycles > 0)&&(param_preroute_pkts != 0)) { // this is added to re-preroute
      // packets that have been hung
      // for some cycles
      local_iu[i].repreroute();
    }
  }
  
  // Receive & push into VC FIFOs arriving flits.
  // This will also perform routing.
  for (i=0; i<_ports; i++) {
    input_units[i].receive();
    if ((param_repreroute_cycles > 0) &&(param_preroute_pkts != 0)) {// this is added to re-preroute
      // packets that have been hung
      // for some cycles
      
      input_units[i].repreroute();
    }
  }
  
  // Select an output port (for adaptive routing)
  select_output_ports();
  
  // Perform VC & XB allocation
  // The order of the calls depends on whether XB alloc is speculative.
  // This is a simulation artifact; in reality, these would be in parallel.
  // Sequential allocation models a non-speculative realistic allocation
  // with VC and XB allocators taking 1 cycle each.
  if (spec_xb_alloc) {	  
    // X-bar Allocation
    xb_allocator->allocate_xbar();
    
    // VC Allocation
    vc_allocator->allocate_vc();
    // Finalize XB alloc decisions based on outcome of VC alloc.
    xb_allocator->alloc_finalize();
  }
  else if (sequential_alloc) {	  
    // X-bar Allocation
    xb_allocator->allocate_xbar();
    
    // VC Allocation
    vc_allocator->allocate_vc();
  }
  else {
    // VC Allocation
    vc_allocator->allocate_vc();
  
    // X-bar Allocation
    xb_allocator->allocate_xbar();
  }
  
  // X-bar traversal
  xbar->transfer();

  
#ifdef OCIN_MON	
  // update fifo stats
  for (int i=0; i<vcid2vc.size(); i++) {
    vcid2vc[i]->mon->update();
  }
#endif

  // Local congestion info computed in the same cycle? 
  // eg: vc/xb requests (but NOT grants or stalls)
  if (param_same_cycle_local_cost != 0) {
    // Update cost registers 
    if (cost_mgr) {
      for (int i=0; i<cost_regs.size(); i++) {
        for (int j=0; j<cost_regs[0].size(); j++) {
          cost_regs[i][j].update();
        }
      }
    }
  }
    
  // Model an extra cycle pipeline delay before launching 
  // the congestion msg?
  if (param_0delay_cost_msg_update != 0) {  // no delay: wire + aggregate in 1 cycle
    // update the cost reg using *last* cycle's stats
    // captured by cost_regs during update
    if (cost_mgr)
      cost_mgr->update_cost();
	      
    // Drive outputs
    for (i=0; i<_ports; i++) {
      output_units[i].transmit();
    }
  }
  else {                               // delay: wire=1 cycle, aggregate=1 cycle
    // Drive outputs
    for (i=0; i<_ports; i++) {
      output_units[i].transmit();
    }
	  
    // update the cost reg using *last* cycle's stats
    // captured by cost_regs during update
    if (cost_mgr)
      cost_mgr->update_cost();
  }
  
  // Local congestion info can NOT be computed in the same cycle? 
  // eg: free vcs, free buffers (fn of grants, which come late in the cycle)
  if (param_same_cycle_local_cost == 0) {
    // Update cost registers
    if (cost_mgr) {
      for (int i=0; i<cost_regs.size(); i++) {
        for (int j=0; j<cost_regs[0].size(); j++) {
          cost_regs[i][j].update();
        } 
      }
    }
  }

}

void ocin_router::update() {
  
}

void ocin_router::finalize() {

}

