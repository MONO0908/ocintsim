#ifndef OCIN_VIS_ARROW_H
#define OCIN_VIS_ARROW_H

using namespace std;

#include "gd.h"
#include "ocin_vis_helper.h"
#include <iostream>
#include <fstream>
#include "ocin_helper.h"

/* This is the info associated with just one arrow instance. Also
   handles the drawing of the arrow itself*/
class ocin_vis_arrow {
 public:
  
  vis_dir_t dir;                /* specifies the direction of this
                                   arrow  */

  int value;

  /* diagram coordinates */
  int x1;                       /* X of line start */
  int x2;                       /* X of line end */
  int y1;                       /* Y of line start */
  int y2;                       /* Y of line end */
  
  /* extra points for the arrow head */
  gdPoint points[3];            /* calced from end of line and the
                                   size of the arrow head "a_size" */

  int thickness;
  int a_size;

  /* setup the initial variables up*/
  void init (int _x1, int _y1, int w_len, int _a_size, 
             int _thickness, vis_dir_t _dir);

  void hold_value(int _value);
  int get_value();

  /* prints the arrow into the image poined to by the pointer */
  void print(gdImagePtr im, float scale_val);
  
  
};

#endif
