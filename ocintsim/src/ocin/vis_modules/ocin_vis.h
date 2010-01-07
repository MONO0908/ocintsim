#ifndef OCIN_VIS_H_
#define OCIN_VIS_H_

/* Bring in gd library functions */
#include "gd.h"
#include "gdfontl.h"
#include "ocin_channel.h"
#include "ocin_node_info.h"
#include "ocin_vis_node.h"
#include "ocin_vis_helper.h"
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <sstream>

 class ocin_vis {
public:

  map<string, ocin_node_info> & node_inst_data; /* Ref of map of info
                                                  read from netcfg
                                                  file.  Key is the
                                                  name of the node. */

  map<string, ocin_node_info>::iterator nid_it; /* iterator for
                                                   node_isnt_data */

  map<string, ocin_router> & nodes; /* Ref to map of network nodes (a
				     node may be a router or an
				     injection- ejection port).  Key
				     is the name of the node*/

  // map of the ocin vis nodes, matching node_inst_data
  map<string, ocin_vis_node> vis_nodes;

  map<string, ocin_vis_node>::iterator vis_it;


  vector<int> max;              // max coord numbers

  /* on instantiation set up refs */
  ocin_vis(map<string, ocin_node_info> &_node_inst_data, 
	   map<string, ocin_router> &_nodes )
    : node_inst_data(_node_inst_data), nodes(_nodes) {};
  // Some variables used to define size and spacing of nodes
  int x_num;
  int y_num;
  int im_x_max;
  int im_y_max;

  int size;
  int space;
  int offset;
  int wirespace;
  int still_hot;
  int thickness;
  int a_size;
  int b_size;
  double ft_size;

  /* Declare the image */
  gdImagePtr im;
  /* Declare output files */
  FILE *pngout;
  /* Declare color indexes */
  int black;
  int white;


  void init ();
  void print (bool final);
  void color_bar();


};

#endif
