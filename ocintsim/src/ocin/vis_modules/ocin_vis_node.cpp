#include "ocin_vis_node.h"


void ocin_vis_node::init (const vector <int>& coord, const vector <int>& max, 
                          int size, int space, 
                          int offset, int wirespace, double _ft_size, 
                          int _thickness, int _a_size, int _b_size,
                          ocin_node_info *_n_info, ocin_router *_node, 
			  string _name) {


  // setup the defining vars
  x1 = coord[0]*(size + space) +offset;
  x2 = x1 + size;
  y1 = coord[1]*(size + space) +offset;
  y2 = y1 + size;
  name = _name;
  ft_size = _ft_size;
  thickness = _thickness;
  a_size = _a_size;
  b_size = _b_size;
  n_info = _n_info;
  node = _node;
  

  // now calc the appropriate position for the start of the string
  // (basically centered in the rectangle). Good luck if the string is
  // too long...

  int brect[8];
  char *fc = "/usr/share/fonts/ttf-bitstream-vera/Vera.ttf";
  char *err;

  char buf[30];
  memset( buf, '\0', 30 );
  name.copy( buf, 24 );

  // this stuff does not work at UT for some reason
  // printing string to null to get size of text
  err = gdImageStringFT(NULL,&brect[0],0,fc,ft_size,0.,0,0,buf);
  if (err) {fprintf(stderr,err); exit(0);}

  // 1/2 box size plus edge give middle then subtract off 1/2 text
  // size. I'm not gonna worry about rounding here..
  str_x = (x1+size/2) - (brect[2]-brect[6])/2;
  str_y = (y1+size/2) + (brect[3]-brect[7])/2;
  

  // instead I'll just approximate center
  // str_x = (x1+size/2);
  // str_y = (y1+size/2);
  
  // Instantiate wires for all directions that are legal

  if (coord[0] != 0) {          // not on the west edge
    wires[W].init(x1, y1+wirespace, space-b_size, a_size, thickness, W);
    fifos[W].init(x1-b_size, y1+(2*wirespace)-b_size/2, b_size, W);
  }

  if (coord[1] != 0) {          // not on the north edge
    wires[N].init(x1+wirespace, y1, space-b_size, a_size,  thickness, N);
    fifos[N].init(x1+(2*wirespace)-b_size/2, y1-b_size, b_size, N);
  }

  if (coord[0] != max[0]) {     // not on the east edge
    wires[E].init(x1+size, y1+wirespace*2, space-b_size, a_size, thickness, E);
    fifos[E].init(x1+size, y1+wirespace-b_size/2, b_size, E);
  }

  if (coord[1] != max[1]) {     // not on the south edge
    wires[S].init(x1+wirespace*2, y1+size, space-b_size, a_size, thickness, S);
    fifos[S].init(x1+wirespace-b_size/2, y1+size, b_size, S);
  }

  // Now setup the chpt stuff and max/min for vis types:
  ltype = ltype_notdefined;
  ftype = ftype_notdefined;

  if (param_vis_fifo_type.compare("free_buff") == 0) {
    invert_fifo_polarity = true;
    //    fifo_max_val = n_info->vc_count * n_info->que_depth * param_stats_interval;
    fifo_max_val = 0;
    fifo_min_val = 0;
    ftype = ftype_free_buff;
  }
  if (param_vis_fifo_type.compare("vc_alloc") == 0) {
    invert_fifo_polarity = false;
    //    fifo_max_val = n_info->vc_count * param_stats_interval;
    fifo_max_val = 0;
    fifo_min_val = 0;
    ftype = ftype_vc_alloc;
  }

  if (param_vis_link_type.compare("used_buff") == 0) {
    invert_link_polarity = false;
    link_max_val = n_info->vc_count * (n_info->que_depth-3);
    link_min_val = 0;
    ltype = ltype_free_buff;
  }
  if (param_vis_link_type.compare("vc_used") == 0) {
    invert_link_polarity = false;
    link_max_val = n_info->vc_count;
    link_min_val = 0;
    ltype = ltype_vc_alloc;
  }
  if (param_vis_link_type.compare("xbar_gnts") == 0) {
    invert_link_polarity = false;
    link_max_val = 0;
    link_min_val = 0;
    ltype = ltype_xbar_gnts;
  }
  if (param_vis_link_type.compare("xbar_reqs") == 0) {
    invert_link_polarity = false;
    link_max_val = 0;
    link_min_val = 0;
    ltype = ltype_xbar_reqs;
  }
  if (param_vis_link_type.compare("xbar_demand") == 0) {
    invert_link_polarity = false;
    //    link_max_val = n_info->vc_count;
    link_max_val = 0;
    link_min_val = 0;
    ltype = ltype_xbar_demand;
  }
  if (param_vis_link_type.compare("xb_buff") == 0) {
    invert_link_polarity = false;
    //    link_max_val = n_info->vc_count;
    link_max_val = 0;
    link_min_val = 0;
    ltype = ltype_xb_buff;
  }
  if (param_vis_link_type.compare("link_util") == 0) {
    invert_link_polarity = false;
    link_max_val = 0;
    link_min_val = 0;
    ltype = ltype_link_util;
  }
  if (param_vis_link_type.compare("pkt_delay") == 0) {
    invert_link_polarity = false;
    link_max_val = 0;
    link_min_val = 0;
    ltype = ltype_pkt_delay;
  }

}

// This function grabs values for each directiction's wires to be used
// in printing
void ocin_vis_node::chpt(bool final) {

  bool max_relative = false;    // true if we want to determine the
                                // min and max dynamically

  if (final) {
    max_relative = true;
  }

  // Depending on what data we want to show, we grab the data here...

  // fifos are painted first
  int total_ports = n_info->port_count;

  switch (ftype)
    {
    case ftype_free_buff:
      for (int i=0; i<total_ports; i++) {     // iterate over input ports
        int value = 0;
        for (int j=0; j<node->vc_fifos[i].size(); j++) {
          if (final) {
            value += node->vc_fifos[i][j].mon->get_all_buffers_free();
          } else {
            value += node->vc_fifos[i][j].mon->get_chpt_buffers_free();
          }
        }
        fifos[(vis_dir_t)i].hold_value(value);	

        if (max_relative) {
          //we want to scale to max seen not abs max since 
          
          if ((fifo_max_val == 0) && (fifo_min_val == 0)) {
            fifo_max_val = value;
            fifo_min_val = value;
          } else {
            if (fifo_max_val < value) {
              fifo_max_val = value;
            }
            if (fifo_min_val > value) {
              fifo_min_val = value;
            }
          }
        }
      }
      break;
    case ftype_vc_alloc:
      for (int i=0; i<total_ports; i++) {     // iterate over input ports
        int value = 0;
        for (int j=0; j<node->vc_fifos[i].size(); j++) {
          if (final) {
            value += node->vc_fifos[i][j].mon->get_all_cycles_allocated();
          } else {
            value += node->vc_fifos[i][j].mon->get_chpt_cycles_allocated();
          }
        }	
        fifos[(vis_dir_t)i].hold_value(value);

        if (max_relative) {
          //we want to scale to max seen not abs max since 
          
          if ((fifo_max_val == 0) && (fifo_min_val == 0)) {
            fifo_max_val = value;
            fifo_min_val = value;
          } else {
            if (fifo_max_val < value) {
              fifo_max_val = value;
            }
            if (fifo_min_val > value) {
              fifo_min_val = value;
            }
          }
        }
      }
      break;
    default:
      stringstream t;
      t << "Unknown param_vis_fifo_type: "<< param_vis_fifo_type <<endl ;

      ocin_name_fatal(name,t.str());
      exit(0);
      break;
    }


  // here are the link stats
  switch (ltype)
    {
    case ltype_free_buff:
      for (int i=0; i<total_ports; i++) {     // iterate over output ports
        int value = 0;
        for( int j=0; j<node->downstream_vc_stats[i].size(); j++) {
          value +=  n_info->que_depth - node->downstream_vc_stats[i][j].credits;
        }

        wires[(vis_dir_t)i].hold_value(value);	

        if ((link_max_val == 0) && (link_min_val == 0)) {
          link_max_val = value;
          link_min_val = value;
        } else {
          if (link_max_val < value) {
            link_max_val = value;
          }
          if (link_min_val > value) {
            link_min_val = value;
          }
        }
      }
      break;
    case ltype_vc_alloc:
      for (int i=0; i<total_ports; i++) {     // iterate over output ports
        int value = 0;

        value = n_info->vc_count - node->free_vc_list[i].size();

        wires[(vis_dir_t)i].hold_value(value);

        if ((link_max_val == 0) && (link_min_val == 0)) {
          link_max_val = value;
          link_min_val = value;
        } else {
          if (link_max_val < value) {
            link_max_val = value;
          }
          if (link_min_val > value) {
            link_min_val = value;
          }
        }

      }
      break;
    case ltype_xbar_gnts:
      for (int i=0; i<total_ports; i++) {     // iterate over input ports
        int value=0;
        for (int j=0; j<total_ports; j++) { // iterate over out ports @
          // each in port
          if (final) {
            value += node->pout_mon[i][j].xmon.get_all_global_gnts();
          } else {
            value += node->pout_mon[i][j].xmon.get_chpt_global_gnts();
          }
        }
        wires[(vis_dir_t)i].hold_value(value);
        if (max_relative) {
          //we want to scale to max seen not abs max since 
          
          if ((link_max_val == 0) && (link_min_val == 0)) {
            link_max_val = value;
            link_min_val = value;
          } else {
            if (link_max_val < value) {
              link_max_val = value;
            }
            if (link_min_val > value) {
              link_min_val = value;
            }
          }
        }
      }
      break;
    case ltype_link_util:
      for (int i=0; i<total_ports; i++) {     // iterate over input ports
        int value=0;

        if (n_info->input_links[i] != NULL) {
          if (final) {
            value = n_info->input_links[i]->mon.get_all_flit_count();
          } else {
            value = n_info->input_links[i]->mon.get_chpt_flit_count();
          }
        } else {
          value = 0;
        }

        wires[(vis_dir_t)i].hold_value(value);
        //        if (max_relative) {
          //we want to scale to max seen not abs max since 
          
        if ((link_max_val == 0) && (link_min_val == 0)) {
          link_max_val = value;
          link_min_val = value;
        } else {
          if (link_max_val < value) {
            link_max_val = value;
          }
          if (link_min_val > value) {
            link_min_val = value;
          }
        }
          //        }
      }
      break;
    case ltype_xbar_reqs:
      for (int i=0; i<total_ports; i++) {     // iterate over input ports
        int value=0;
        max_relative = true;
        for (int j=0; j<total_ports; j++) { // iterate over out ports @
          // each in port
          if (final) {
            value += node->pout_mon[i][j].xmon.get_all_global_reqs();
          } else {
            value += node->pout_mon[i][j].xmon.get_chpt_global_reqs();
          }
        }
        wires[(vis_dir_t)i].hold_value(value);
        if (max_relative) {
          //we want to scale to max seen not abs max since 
          
          if ((link_max_val == 0) && (link_min_val == 0)) {
            link_max_val = value;
            link_min_val = value;
          } else {
            if (link_max_val < value) {
              link_max_val = value;
            }
            if (link_min_val > value) {
              link_min_val = value;
            }
          }
        }
      }
      break;
    case ltype_xb_buff:
      for (int i=0; i<total_ports; i++) {     // iterate over input ports
        int value=0;
        max_relative = true;
        for (int j=0; j<total_ports; j++) { // iterate over out ports @
          // each in port
          if (final) {
            value += node->pout_mon[i][j].xmon.global_demand(true /*final*/);
          } else {
            value += node->pout_mon[i][j].xmon.global_demand(false /*final?*/);
          }
        }

        for( int j=0; j<node->downstream_vc_stats[i].size(); j++) {
          value +=  n_info->que_depth - node->downstream_vc_stats[i][j].credits;
        }

        wires[(vis_dir_t)i].hold_value(value);
        if ((link_max_val == 0) && (link_min_val == 0)) {
          link_max_val = value;
          link_min_val = value;
        } else {
          if (link_max_val < value) {
            link_max_val = value;
          }
          if (link_min_val > value) {
            link_min_val = value;
          }
        }
      }
      break;
    case ltype_xbar_demand:
      for (int i=0; i<total_ports; i++) {     // iterate over input ports
        int value=0;
        max_relative = true;
        for (int j=0; j<total_ports; j++) { // iterate over out ports @
          // each in port
          if (final) {
            value += node->pout_mon[i][j].xmon.global_demand(true /*final*/);
          } else {
            value += node->pout_mon[i][j].xmon.global_demand(false /*final?*/);
          }
        }
        wires[(vis_dir_t)i].hold_value(value);
        if (max_relative) {
          //we want to scale to max seen not abs max since 
          
          if ((link_max_val == 0) && (link_min_val == 0)) {
            link_max_val = value;
            link_min_val = value;
          } else {
            if (link_max_val < value) {
              link_max_val = value;
            }
            if (link_min_val > value) {
              link_min_val = value;
            }
          }
        }
      }
      break;
    case ltype_pkt_delay:
      for (int i=0; i<total_ports; i++) {     // iterate over input ports
        int value = 0;

        // go through each vc fifo looking for flits headed for this
        // output port and sum up thier latency
        vector <vector <ocin_router_vc_fifo> > &vc_fifos = node->vc_fifos;
        
        vector <vector <ocin_router_vc_fifo> >::iterator vc_fifos_i;
        vector <ocin_router_vc_fifo>::iterator vc_fifos_k;
        
        // iterate over all input vc_fifos (inc. injection port)
        for (vc_fifos_i=vc_fifos.begin(); vc_fifos_i != vc_fifos.end(); vc_fifos_i++) {
          // iterate over classes (vc_fifos_j) here	
          for (vc_fifos_k=vc_fifos_i->begin(); vc_fifos_k != vc_fifos_i->end(); vc_fifos_k++) {
            if (!(vc_fifos_k->is_empty())) {
              ocin_flit * flit = vc_fifos_k->front_flit();
              // Only if this output port is one of the the ports in the
              // flit's routes map
              if (flit->msg->routes.find(i) != flit->msg->routes.end()) { 
                // add to delay sum
                value += ocin_cycle_count - flit->arrival_time;
              }
            }
          }
        }   

        wires[(vis_dir_t)i].hold_value(value);

        if ((link_max_val == 0) && (link_min_val == 0)) {
          link_max_val = value;
          link_min_val = value;
        } else {
          if (link_max_val < value) {
            link_max_val = value;
          }
          if (link_min_val > value) {
            link_min_val = value;
          }
        }
      }
      break;
    default:
      stringstream t;
      t << "Unknown param_vis_link_type: "<< param_vis_link_type <<endl ;
      
      ocin_name_fatal(name,t.str());
      exit(0);
      break;
    }


}

void ocin_vis_node::print(gdImagePtr im) {

  int black;
  int wire_color;

  black = gdImageColorResolve(im, 0, 0, 0);  

  /* Draw a rectangle occupying the central area. */
  gdImageSetThickness(im, 3);
  gdImageRectangle(im, x1, y1, x2, y2, black);


  //This stuff does not work at UT
  // Write name in it
  int brect[8];
  char *fc = "/usr/share/fonts/ttf-bitstream-vera/Vera.ttf";
  char *err;

  char buf[30];
  memset( buf, '\0', 30 );
  name.copy( buf, 24 );

  err = gdImageStringFT(im,&brect[0],black,fc,ft_size,0.0,str_x,str_y,buf);
  if (err) {fprintf(stderr,err);}
  

  
  // // instead I'm just going to use the (crappy) built in font
  // gdImageString(im, gdFontGetLarge(),str_x - (strlen(buf) * gdFontGetLarge()->w / 2),
  //               str_y - gdFontGetLarge()->h / 2, (unsigned char *)buf, black);

  float scale_val;

  // iterate over the arrows that we have and print them to image
  for(w_it = wires.begin(); w_it != wires.end(); w_it++) {

    int value = w_it->second.get_value();
    
    //cout << link_max_val<<":"<<link_min_val<<":"<<value<<endl;
    if (link_max_val != link_min_val) {
      scale_val = ((float)(value - link_min_val))/
        ((float)(link_max_val - link_min_val));
    } else {
      scale_val = 0;
    }

    if (invert_link_polarity) {
      scale_val = 1-scale_val;
    }

    w_it->second.print(im,scale_val);
  }

  // iterate over the fifos that we have and print them to image
  for(f_it = fifos.begin(); f_it != fifos.end(); f_it++) {
    int value = f_it->second.get_value();
    
    //cout << link_max_val<<":"<<link_min_val<<":"<<value<<endl;
    if (fifo_max_val != fifo_min_val) {
      scale_val = ((float)(value - fifo_min_val))/
        ((float)(fifo_max_val - fifo_min_val));
    } else {
      scale_val = 0;
    }

    //    cout << "fifo value: " << value << " scale val: " << scale_val<< endl;

    if (invert_fifo_polarity) {
      scale_val = 1-scale_val;
    }
    f_it->second.print(im,scale_val);
  }

}


/* Heat map algo:

 scaledValue = (c.Value - minValue) / (maxValue - minValue)

        If (scaledValue > 0.5) Then
            tmpShp.Fill.ForeColor.RGB = RGB(510 * (scaledValue - 0.5), 0, 0)
            Else
            tmpShp.Fill.ForeColor.RGB = RGB(0, 0, 255 - scaledValue * 510)
        End If

*/
