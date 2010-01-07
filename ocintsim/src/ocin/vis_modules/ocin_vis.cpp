#include "ocin_vis.h"


void ocin_vis::init () {
  // Setup the initial spacing and related variables:
  size = 70;                    // size in pixels of nodes
  space = 70;                   // space btw nodes
  offset = 40;                  // space around outside of diagram
  wirespace = size/3;           // space bwt wires
  ft_size = 20.0;               // fontsize
  thickness = 7;                // line thickness
  a_size  = 15;                 // arrowhead size
  b_size = 1;			// size of fifo box

  // Iterate through the array of nodes, creating vis_nodes and
  // figuring out what the network extents are.


  x_num = 0;
  y_num = 0;


  // first figure out the max x or y 
  for (nid_it=node_inst_data.begin();
       nid_it != node_inst_data.end(); nid_it++) {
    if(nid_it->second.coord[0] > x_num) {
      x_num = nid_it->second.coord[0];
    }
    if(nid_it->second.coord[1] > y_num) {
      y_num = nid_it->second.coord[1];
    }
  }

  max.push_back(x_num);
  max.push_back(y_num);

  for (nid_it=node_inst_data.begin();
       nid_it != node_inst_data.end(); nid_it++) {
    
    string name = nid_it->first;

    ;
    vis_nodes[name].init(nid_it->second.coord, max, size, space, 
                         offset, wirespace, ft_size, thickness, a_size, b_size,
                         &nid_it->second, &nodes[name], name);

  }


  // now calc image size
  im_x_max = size*(x_num+1)+space*x_num +2*offset;
  im_y_max = size*(y_num+1)+space*y_num +2*offset;

  // Adding space on the right for a color bar
 
  im_x_max += 300;

  // correcting for 16 pixel boundary
  int temp = 16 - (im_x_max % 16);
  im_x_max += temp;

  temp = 16 - (im_y_max % 16);
  im_y_max +=temp;



}

void ocin_vis::print(bool final) {

  /* Allocate the image */
  //  gdUseFontConfig(1);
  im = gdImageCreate(im_x_max, im_y_max);
  //im = gdImageCreateTrueColor(im_x_max, im_y_max);

  //gdImageFilledRectangle(im, 0, 0, im_x_max, im_y_max, white);

  /* Allocate the color white (red, green and blue all maximum), used
     on background. */
  white = gdImageColorAllocate(im, 255, 255, 255);

  black = gdImageColorAllocate(im, 0, 0, 0); // and black

  // make color bar on right
  color_bar();


  // first call the ckpt function to grab a value
  for (vis_it=vis_nodes.begin();
       vis_it != vis_nodes.end(); vis_it++) {
    
    vis_it->second.chpt(final);
  }
  
  
  // now print the extents on the color bar:
  // first top value:
  int y_pos = 43;
  int x_pos = im_x_max - 270;
  char buf[100];
  int brect[8];
  char *fc = "/usr/share/fonts/ttf-bitstream-vera/Vera.ttf";
  char *err;

  sprintf(buf, "Link monitor type:\n%s ", param_vis_link_type.c_str());
  
  err = gdImageStringFT(im,&brect[0],black,fc,ft_size,0.0,x_pos,y_pos,buf);
  if (err) {fprintf(stderr,err);}

  y_pos += 65;
  x_pos += 20;
  if (param_vis_link_type.compare("pkt_delay") == 0) {
    sprintf(buf, "-- %d cycles",ocin_vis_node::link_max_val); 
  } else if (param_vis_link_type.compare("xbar_demand") == 0) {
    sprintf(buf, "-- %d requests",ocin_vis_node::link_max_val); 
  } else {
    sprintf(buf, "-- %d used",ocin_vis_node::link_max_val); 
  }

  err = gdImageStringFT(im,&brect[0],black,fc,ft_size,0.0,x_pos,y_pos,buf);
  if (err) {fprintf(stderr,err);}

  //  gdImageString(im, gdFontGetLarge(), x_pos, y_pos, (unsigned char *)buf, black);

  // bottom value
  if (param_vis_link_type.compare("pkt_delay") == 0) {
    sprintf(buf, "-- %d cycles",ocin_vis_node::link_min_val); 
  } else if (param_vis_link_type.compare("xbar_demand") == 0) {
    sprintf(buf, "-- %d requests",ocin_vis_node::link_min_val); 
  }else {
    sprintf(buf, "-- %d used",ocin_vis_node::link_min_val); 
  }

  y_pos += 255;
  err = gdImageStringFT(im,&brect[0],black,fc,ft_size,0.0,x_pos,y_pos,buf);
  if (err) {fprintf(stderr,err);}
  //  gdImageString(im, gdFontGetLarge(), x_pos, y_pos, (unsigned char *)buf, black);

  // Also print the types of monitor shown
  x_pos = im_x_max -260;
  y_pos = im_y_max/2;           // start 1/2 way down



  // print some simulation information for reference
  nid_it=node_inst_data.begin();
  
  sprintf(buf, "Traffic type: \n%s", nid_it->second.src_type[0].c_str()); 

  y_pos += 50;
  //  err = gdImageStringFT(im,&brect[0],black,fc,ft_size,0.0,x_pos,y_pos,buf);
  //  if (err) {fprintf(stderr,err);}



  // then go through each node object and call their print function

  for (vis_it=vis_nodes.begin();
       vis_it != vis_nodes.end(); vis_it++) {
    
    vis_it->second.print(im);
  }
  

  // create a file name
  stringstream filename;
  char cycle_txt[20];
  sprintf(cycle_txt,"%012d",ocin_cycle_count);

  // place cycle count in image:
  //  gdImageString(im, gdFontGetLarge(),10, 10, (unsigned char *)cycle_txt, black);

  if (final) {
    filename << "final_" <<cycle_txt<<".png";
  } else {
    filename << "heat_" <<cycle_txt<<".png";
  }
  
  pngout = fopen(filename.str().c_str(), "wb");

  /* Output the image to the disk file in PNG format. */
  gdImagePng(im, pngout);

  /* Close the files. */
  fclose(pngout);

  /* Destroy the image in memory. */
  gdImageDestroy(im);

  //  if ((param_vis_link_type.compare("xbar_demand") == 0) ||(param_vis_link_type.compare("pkt_delay") == 0)) {
  //  if (param_vis_link_type.compare("pkt_delay") == 0) {
    
    ocin_vis_node::link_min_val = 0; 
    ocin_vis_node::link_max_val = 0;
    //  }

}

void ocin_vis::color_bar() {
  int color;
  int y = 100;                   // start 20 px down from top
  int x = im_x_max -260;         // start 40 px from right edge

  for(int cnt = 0; cnt < 255; cnt++) {
    color = gdImageColorResolve(im, cnt, cnt, cnt);

    gdImageSetPixel(im, x, y, color);
    gdImageSetPixel(im, x+1, y, color);
    gdImageSetPixel(im, x+2, y, color);
    gdImageSetPixel(im, x+3, y, color);
    gdImageSetPixel(im, x+4, y, color);
    gdImageSetPixel(im, x+5, y, color);
    gdImageSetPixel(im, x+6, y, color);
    gdImageSetPixel(im, x+7, y, color);
    gdImageSetPixel(im, x+8, y, color);
    gdImageSetPixel(im, x+9, y, color);
    gdImageSetPixel(im, x+10, y, color);
    y++;
  }
}
