#include "ocin_vis_fifo.h"

void ocin_vis_fifo::init (int _x1, int _y1, int b_size, vis_dir_t _dir) {

  dir = _dir;



  // have to convert to point 1 is upper left, point 2 is lower right
  if (dir == S) {
    x1 = _x1;
    y1 = _y1;
    x2 = _x1 + b_size;
    y2 = _y1 + b_size;

  } else if (dir == E) {
    x1 = _x1;
    y1 = _y1;
    x2 = _x1 + b_size;
    y2 = _y1 + b_size;

  } else if (dir == N) {
    x1 = _x1;
    y1 = _y1;
    x2 = _x1 + b_size;
    y2 = _y1 + b_size;

  } else if (dir == W) {
    x1 = _x1;
    y1 = _y1;
    x2 = _x1 + b_size;
    y2 = _y1 + b_size;

  } else {
    stringstream t;
    t << "Error: Illegal direction!\n";
    ocin_fatal(t.str());
    exit(0);
  }

}



/* prints the fifo into the image poined to by the pointer with given
   color*/
void ocin_vis_fifo::print(gdImagePtr im, float scale_val) {

  //int thick = ((int)((float)thickness *scale_val)) + 1;

  //int scaled_a_size = (int) ((float)(a_size - 4) *scale_val) + 4;

  // select the color for the box
  int cnt = ((int)(510.0*scale_val)) - 255;
  cnt = cnt &0xfffffffe;	// make sure we have an even number

  int color;
  if (cnt < 0) {
    color = gdImageColorResolve(im, 0, 255+cnt, (cnt*-1));
  } else {
    color = gdImageColorResolve(im, cnt, 255-cnt, 0);
  }

  //  color = gdImageColorResolve(im, 0, 0, 0);
  gdImageFilledRectangle(im, x1, y1, x2, y2, color);
  
 
  
}

void ocin_vis_fifo::hold_value(int _value) {
  value = _value;
}

int ocin_vis_fifo::get_value() {
  return value;
}
