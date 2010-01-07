#include "ocin_vis_arrow.h"

void ocin_vis_arrow::init (int _x1, int _y1, int w_len, int _a_size, 
                           int _thickness, vis_dir_t _dir) {

  x1 = _x1;
  y1 = _y1;
  dir = _dir;
  a_size = _a_size;
  thickness = _thickness;



  if (dir == S) {
    x2 = x1;
    y2 = y1 + w_len;

  } else if (dir == E) {
    x2 = x1 + w_len;
    y2 = y1;

  } else if (dir == N) {
    x2 = x1;
    y2 = y1 - w_len;

  } else if (dir == W) {
    x2 = x1 - w_len;
    y2 = y1;

  } else {
    stringstream t;
    t << "Error: Illegal direction!\n";
    ocin_fatal(t.str());
    exit(0);
  }

}



/* prints the arrow into the image poined to by the pointer with given
   color*/
void ocin_vis_arrow::print(gdImagePtr im, float scale_val) {

  int thick = ((int)((float)thickness *scale_val)) + 1;

  int scaled_a_size = (int) ((float)(a_size - 4) *scale_val) + 4;

  // create points for arrow head based on scale factor
  if (dir == S) {
    points[1].x = x2 - scaled_a_size;
    points[1].y = y2 - scaled_a_size;
    points[2].x = x2 + scaled_a_size;
    points[2].y = y2 - scaled_a_size;
  } else if (dir == E) {
    points[1].x = x2 - scaled_a_size;
    points[1].y = y2 + scaled_a_size;
    points[2].x = x2 - scaled_a_size;
    points[2].y = y2 - scaled_a_size;
  } else if (dir == N) {
    points[1].x = x2 - scaled_a_size;
    points[1].y = y2 + scaled_a_size;
    points[2].x = x2 + scaled_a_size;
    points[2].y = y2 + scaled_a_size;
  } else if (dir == W) {
    points[1].x = x2 + scaled_a_size;
    points[1].y = y2 + scaled_a_size;
    points[2].x = x2 + scaled_a_size;
    points[2].y = y2 - scaled_a_size;
  }
  // setup the arrow head points
  points[0].x = x2;             // first point is end of the line
  points[0].y = y2;

  // select the color for the arrow head
  int cnt = 255 - ((int)(250.0*scale_val));
  cnt = cnt &0xfffffffe;	// make sure we have an even number
				// (?)
  int color;
  color = gdImageColorResolve(im, cnt, cnt, cnt);

  // if (cnt < 0) {
  //   color = gdImageColorResolve(im, 0, 255+cnt, (cnt*-1));
  // } else {
  //   color = gdImageColorResolve(im, cnt, 255-cnt, 0);
  // }


  
  // draw the arrowhead first (why not)
  gdImageFilledPolygon(im, points, 3, color);

  // then the line
  gdImageSetThickness(im, thick);
  gdImageLine(im, x1, y1, x2, y2, color);
  
 
  
}

void ocin_vis_arrow::hold_value(int _value) {
  value = _value;
}

int ocin_vis_arrow::get_value() {
  return value;
}
