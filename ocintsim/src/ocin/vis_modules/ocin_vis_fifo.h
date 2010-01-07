#ifndef OCIN_VIS_FIFO_H
#define OCIN_VIS_FIFO_H

using namespace std;

#include "gd.h"
#include "ocin_vis_helper.h"
#include <iostream>
#include <fstream>
#include "ocin_helper.h"

/* This is the info associated with just one fifo instance. Also
   handles the drawing of the fifo itself*/
class ocin_vis_fifo {
 public:
  
  vis_dir_t dir;                /* specifies the direction of this
                                   fifo  */

  int value;

  /* diagram coordinates */
  int x1;                       /* X of box start */
  int x2;                       /* X of box end */
  int y1;                       /* Y of box start */
  int y2;                       /* Y of box end */
  
  int thickness;
  int a_size;

  /* setup the initial variables up*/
  void init (int _x1, int _y1, int b_size, vis_dir_t _dir);

  void hold_value(int _value);
  int get_value();

  /* prints the fifo into the image poined to by the pointer */
  void print(gdImagePtr im, float scale_val);
  
  
};

#endif
