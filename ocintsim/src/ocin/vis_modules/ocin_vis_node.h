#ifndef OCIN_VIS_NODE_H
#define OCIN_VIS_NODE_H

using namespace std;

#include "gd.h"
#include "gdfontl.h"
#include "ocin_vis_helper.h"
#include "ocin_vis_arrow.h"
#include "ocin_vis_fifo.h"
#include "ocin_node_info.h"
#include "ocin_mon_port.h"
#include "ocin_router_vc_fifo.h"
#include "ocin_channel.h"
#include <string>
#include <vector>
#include <map>

// Value-Defintions of the different type values for use in switch
// statements
enum fifo_type { ftype_notdefined,
                 ftype_free_buff,
                 ftype_vc_alloc};

enum link_type { ltype_notdefined,
                 ltype_link_util,
                 ltype_xbar_gnts,
                 ltype_xbar_reqs,
                 ltype_xbar_demand,
                 ltype_xb_buff,
                 ltype_pkt_delay,
                 ltype_free_buff,
                 ltype_vc_alloc};


class ocin_router;

/* This is the info and pointers associated with just one node
   instance. Also handles the drawing of the node itself (not wires)*/
class ocin_vis_node {
 public:
  
  string name;                  /* name of node (same as key of*/
  //                                   node_info map */
  
  /* diagram coordinates */
  int x1;                       /* left side */
  int x2;                       /* right side */
  int y1;                       /* bottom */
  int y2;                       /* top */
  
  int str_x;                    /* X start for string */
  int str_y;                    /* Y start for string */

  int thickness;                // thickness of lines
  int a_size;                   // arrowhead size
  int b_size;                   // fifo size
  double ft_size;               /* font size */

  bool invert_fifo_polarity;	/* flag used to determine if less is
				   more */
  bool invert_link_polarity;	/* flag used to determine if less is
				   more */

  ocin_node_info * n_info;      /* used to grab stats info */

  ocin_router * node;      /* used to grab stats info */

  map<vis_dir_t, ocin_vis_arrow> wires; /* map of wires from this
                                           node */
  map<vis_dir_t, ocin_vis_arrow>::iterator w_it; // iterator for wires

  map<vis_dir_t, ocin_vis_fifo> fifos; /* map of fifos on this node */
  map<vis_dir_t, ocin_vis_fifo>::iterator f_it; // iterator for fifos

  link_type ltype;              // used to improve perfomance of link
                                // selection
  fifo_type ftype;

  /* setup the initial variables up*/
  void init (const vector <int>& coord, const vector <int>& max, 
             int size, int space, int offset, int wirespace, 
             double _ft_size, int _thickness, int _a_size, int _b_size,
             ocin_node_info *_n_info, ocin_router * _node, string _name);
  
  /* prints the node into the image poined to by the pointer */
  void print(gdImagePtr im);
  
  void chpt(bool final);

  static int fifo_max_val;           /* maximum value for fifo color map */
  static int fifo_min_val;           /* min value */
  static int link_max_val;           /* maximum value for link color map */
  static int link_min_val;           /* min value */
  
};

#endif
