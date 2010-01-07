#include "ocin_gen_file.h"

void ocin_gen_file::init (tsim_object *parent, string _name) {
  
  gen_done = false;

  name = _name;

  ocn_proper = false;
  opn = false;
  splash = false;

  // is this the second proc?
  cout << "File gen name: "<<name<<endl;
  filegen_num = 0;              // assume single proc
  if (param_ocn_4proc == 1) {
    if (name.compare("generator_file_0") == 0) {
      filegen_num = 0;
    } else if (name.compare("generator_file2_1") == 0) {
      filegen_num = 1;
    }  else if (name.compare("generator_file3_2") == 0) {
      filegen_num = 2;
    }  else if (name.compare("generator_file4_3") == 0) {
      filegen_num = 3;
    }
    
    cout << "File gen number :"<<filegen_num<<endl;
  }

  tsim_module::init (parent, name);
  
  top_p = (ocin_top *) parent;

  // setup the packet monitor
  mon = new ocin_mon_packet;
  mon->init(name, nodes.size());

  // if the file is a standard ocn net cfg file then we will use the
  // proper src node and etc
  // *** Remove me for other uses!!!
  //   if ((param_netcfg_file.compare("ocin_ocn_net.cfg") == 0)||
  //       (param_netcfg_file.compare("ocin_ocn_lesscolumn.cfg") == 0)) {
  //     ocn_proper = true;
  //   } else {
  //     ocn_proper = false;
  //   }

  // for the moment we are forcing ocn_proper to false...
  ocn_proper = true;
  

  cycle_offset = 0;             // for now this is just stuck at 0

  last_inj_cycle = 0;           // for rollovers
  high_inj_cycle = 0;           // upper bits of cycle

  filecomplete = false;

#ifdef DEBUG
  {
    stringstream tmp;
    tmp  << "Initializing Packet Generator.";
    ocin_name_debug(name,tmp.str());
  }
#endif


  // To speed things up a bit we are creating an order'ed vector of
  // local_iu pointers here and I'll iterate over them instead using
  // the node map directly.

  // iterate over the sources and push pointers onto a list
  vector<string>::iterator srcs_it;
  int tmpx,tmpy;
  max_x = 0;
  max_y = 0;

  for (srcs_it = srcs.begin(); srcs_it != srcs.end(); srcs_it++) {
    string src_term = srcs_it->c_str();
    string src = terminal2node_map[src_term];
    int port = terminal2port_map[src_term];
    local_iu_ptrs.push_back(&(nodes[src].local_iu[port]));
    local_iu_ptrs_map[src_term] = &(nodes[src].local_iu[port]);

    
#ifdef DEBUG
    {
      stringstream tmp;
      tmp  << "Packet Gen source:"<<srcs_it->c_str();
      ocin_name_debug(name,tmp.str());
    }
#endif

    // we also want to know the max x and max y
    vector <int> tmp = nodes[src].node_info->terminal_coords[port];
    tmpx = tmp[0];
    tmpy = tmp[1];
    if (tmpx > max_x) {
      max_x = tmpx;
    }
    if (tmpy > max_y) {
      max_y = tmpy;
    }

  }
  
  // map terminal id's to coordinates
  int term_id = 0;
  for (int x=0; x<=tmpx; x++) {
  	for (int y=0; y<=tmpy; y++) {
		vector <int> coords;
		coords.push_back(x);
		coords.push_back(y); 
    	termid2coords.push_back(coords);
    	term_id++;
  	}
  }

#ifdef DEBUG
  {
    for (srcs_it = dsts.begin(); srcs_it != dsts.end(); srcs_it++) {
      stringstream tmp;
      tmp  << "Packet Gen ejector:"<<srcs_it->c_str();
      ocin_name_debug(name,tmp.str());
    }
  }
#endif

  filepart = 0;                 // initializing file part to 0

  // First determine if this is a gzipped file
  string::size_type loc = param_tracefile_name.find( ".gz", 0);

  if (loc != string::npos ) {
    is_compressed = true;

    // now determine if it's a multipart trace
    loc = param_tracefile_name.find("0.bin.gz", 0);
    if (loc != string::npos) {
      is_multipart = true;
    } else {
      is_multipart = false;
    }
  } else {
    is_compressed = false;
    loc = param_tracefile_name.find("0.bin", 0);
    if (loc != string::npos) {
      is_multipart = true;
    } else {
      is_multipart = false;
    }
  }

  // setup the addr->translation table
  //setup the trans table
  trans_table[0] = 0x11;        // note:this notation is yx
  trans_table[1] = 0x21;
  trans_table[2] = 0x31;
  trans_table[3] = 0x41;
  trans_table[4] = 0x12;
  trans_table[5] = 0x22;
  trans_table[6] = 0x32;
  trans_table[7] = 0x42;
  trans_table[8] = 0x81;
  trans_table[9] = 0x71;
  trans_table[10] = 0x61;
  trans_table[11] = 0x51;
  trans_table[12] = 0x82;
  trans_table[13] = 0x72;
  trans_table[14] = 0x62;
  trans_table[15] = 0x52;

  // now we can go ahead and open the file
  openfile();

  // Here we setup the bitwidth and masks for !ocn_proper
  if ((!ocn_proper) && (!splash)) {
    cout << "Max x" <<max_x << "  Max y "<< max_y<<endl;
    srcymask = (max_y/2) << L1OFFSET;
    dstymask = (max_y) << L2OFFSET;
    dstxshift = (L2OFFSET + log_base2(max_y+1));
    dstxmask = (max_x-2) << dstxshift;
  }

  // if param_midpoint_file is 1 then we will fast forward to the
  // middle of the file before running the test...
  if(param_midpoint_file == 1) {
    ff_file();
  }
}


// here we open a trace file.  We incapsulate the compressed file
// access and header reading here.
void ocin_gen_file::openfile() {

  if (is_compressed) {
    if((gzpatt_file=gzopen(param_tracefile_name.c_str(), "rb")) == NULL)  {
      stringstream tmp;
      tmp << "Error: Unable to open file " << param_tracefile_name;
      ocin_name_fatal(name,tmp.str());
      exit(1);
    }
  } else {
    if((patt_file=fopen(param_tracefile_name.c_str(), "rb")) == NULL)  {
      stringstream tmp;
      tmp << "Error: Unable to open file " << param_tracefile_name;
      ocin_name_fatal(name,tmp.str());
      exit(1);
    }
  }


  if(filepart == 0) {         // only look for header on first file
    // part
    unsigned long header;
    if (is_compressed) {
      gzread( gzpatt_file, &header, sizeof(header));
    } else {
      fread(&header, sizeof(unsigned long), 1, patt_file);
    }
    
    if (header == 7) {        // opn headers have 0x7 as first word
      opn = true;
    } else {
      opn = false;            // ocn headers have 0x6 as first word
    }
    opn= false;                 // ****REMOVE ME once I fix the ocn
                                // ****trace files!!!

    if (header == 0x0A2D2D2D) { // first word is some space and a
                                // newline
      splash = true;
      opn = false;
    } else {
      splash = false;
    }

    cout <<"Got Header of "<<header<<endl;
  }
  


  //#ifdef DEBUG
  {
    stringstream tmp;
    tmp  << "Opened file "<<param_tracefile_name << " it is :";
    if (is_compressed) {
      tmp << "compressed, ";
    }else {
      tmp << "not compressed, ";
    }
    
    if (is_multipart) {
      tmp << "multipart #"<<filepart<<" ,";
    }else {
      tmp << "not multipart, ";
    }

    if (splash) {
      tmp << "Splash-2 tracefile.";
    }else {
      if (opn) {
        tmp << "OPN tracefile.";
      }else {
        tmp << "OCN tracefile.";
      }
    }

    ocin_name_warn(name,tmp.str());
    //    printf("Warning! dividing size by 8 for wierd parsec traces!\n");
  }
  //#endif

}

// this fast-forwards the file to the middle request to start
// simulation from there.
void ocin_gen_file::ff_file() {

  int file_size;
  int midpoint_entrynum=0;
  struct  stat    statinfo;
  if (splash) {
    // we can get cycles from the file name
    unsigned last_cycle=0;
    unsigned ff_cycles;

    size_t  pos = param_tracefile_name.find("_", 0); 
    string cycle_str = param_tracefile_name.substr(pos+1);    

    sscanf(cycle_str.c_str(), "%d", &last_cycle);
    
    cout << "Splash tracefile: "<<param_tracefile_name<< ":"<<cycle_str<<" Ends on cycle:"<<last_cycle <<endl;

   
    // if we have enough cycles to run past the end of the trace, then
    // skip the startup and go on
    if (((float)last_cycle*.9) < (float)max_cycles) {
      ff_cycles = (unsigned)((float)last_cycle * .1);
      
    } else {

      // Otherwise we will simulate the cycles around the middle of the
      // benchmark up to max cycles.
      ff_cycles = (last_cycle/2) - (max_cycles/2);
    }

    cout << "FF cycles: "<<ff_cycles<<endl;

    ocin_gen_filerec * trash;
    
    unsigned trash_cycle=0;
    int x=0;
    while (trash_cycle < ff_cycles) {
      trash = get_record();
      if (trash != NULL) {
        trash_cycle = trash->inj_time;
        if (x % 10000 == 0) {
          cout << "Fast Forward #" << x << " inj_cycle: " << trash_cycle <<endl;
        }
        delete trash;
        //36 846 869
      }
      x++;
    }
    cout << "Last Fast Forward #" << x << " cycle offset: " << trash_cycle <<endl;
    cycle_offset = (tsim_u64)trash_cycle;
    high_inj_cycle = 0;
    last_inj_cycle = 0;
    
  } else {                      // non-splash fast forward, note this
                                // is screwed up for opn traces...

    int err =  stat(param_tracefile_name.c_str(),&statinfo);
    if (err != 0) {
      stringstream tmp;
      tmp << "Error: Unable to stat file " << param_tracefile_name;
      ocin_name_fatal(name,tmp.str());
      exit(1);
    }
    if (is_compressed) {
      file_size = (int)((float)statinfo.st_size / (float) .66); // roughly 66% compression
    } else {
      file_size = statinfo.st_size;
    }
    
    midpoint_entrynum = file_size / 24; // 12 bytes per entry and we want the middle...
    //  midpoint_entrynum = (file_size/12) - 100;
    
    // provide ~10% of offset for each sucessive concurrent trace
    midpoint_entrynum = (int)((double)midpoint_entrynum * (1.0 - .1*filegen_num));

    
    ocin_gen_filerec * trash;
    
    unsigned trash_cycle;
    int x;
    for ( x = 0; x < midpoint_entrynum; x++) {
      trash = get_record();
      if (trash != NULL) {
        trash_cycle = trash->inj_time;
        if (x % 10000 == 0) {
          cout << "Fast Forward #" << x << " inj_cycle: " << trash_cycle <<endl;
        }
        delete trash;
      }
    }
    cout << "Last Fast Forward #" << x << " cycle offset: " << trash_cycle <<endl;
    cycle_offset = (tsim_u64)trash_cycle;
    high_inj_cycle = 0;
    last_inj_cycle = 0;
  }
    
}

void ocin_gen_file::closefile() {

  if (is_compressed) {
    gzclose(gzpatt_file);
  } else {
    fclose(patt_file);
  }
}


// This reads a single record from the file, returns a file_rec
// pointer or null if the file is done
ocin_gen_filerec * ocin_gen_file::get_record() {

  ocin_gen_filerec * filerec_ptr = NULL;
  
  unsigned long temp[256];

  unsigned long comp_temp;      // working reg for file access
  char bid_temp;                // working reg for blockid

  int count, count2;            // number of bytes read

  tsim_u64 inj_cycle;           // temporary injection cycle
  if (splash) {
    char inputstr[100];
    char * endflag = NULL;
    if (is_compressed) {
      endflag = gzgets(gzpatt_file, inputstr, 99);
    } else {
      endflag = fgets(inputstr, 99, patt_file);
    }

    //    printf("Got this back %x",endflag);

    if (endflag == NULL) {
      return filerec_ptr;           // return a null pointer if we hit the
    } 

    istringstream inputline(inputstr);

    unsigned cycle, src, dst, size;
    inputline >>cycle>>src>>dst>>size;

    if(!inputline) {            // we could not get proper data from
                                // the inputstringstream
      return filerec_ptr;       // return the null
    }

    filerec_ptr = new ocin_gen_filerec; // now we'll need this...

    //    cout <<"Got line: "<< inputstr<<endl;
    //    cout << "Found cycle: "<<cycle<<" src:"<<src<<" dst"<<dst<<" size:"<<size<<endl;
    

    // now we can start filling the info:
    
    filerec_ptr->inj_time = (tsim_u64)cycle - cycle_offset;

    // initialzed on new:
    //    filerec_ptr->addr = 0;          // opn has no address associated

    // src and dst are id numbers, must convert to coords
    vector<int> src_coords = termid2coords[src];
    vector<int> dst_coords = termid2coords[dst];
    filerec_ptr->src_coord = src_coords;
    filerec_ptr->dst_coord = dst_coords;
/*     
    filerec_ptr->src_coord[1] = src / param_num_x_tiles;
    filerec_ptr->src_coord[0] = src % param_num_y_tiles;
    filerec_ptr->dst_coord[1] = dst / param_num_x_tiles;
    filerec_ptr->dst_coord[0] = dst % param_num_y_tiles;
*/

    filerec_ptr->size =  (size);

  }else {
    if(opn) {
      if (is_compressed) {
        
        // read first word of entry
        count = gzread(gzpatt_file, &comp_temp, sizeof(comp_temp) ); 
        // and blockid byte
        count2 = gzread(gzpatt_file, &bid_temp, sizeof(bid_temp)); 

      } else {
        // read first word of entry
        count = fread(&comp_temp, sizeof(comp_temp), 1, patt_file); 
        // and blockid byte
        count2 = fread(&bid_temp, sizeof(bid_temp), 1, patt_file); 
      }

      if ((count !=4)||(count2 != 1)) {
        return filerec_ptr;           // return a null pointer if we hit the
        // end
      } else {
     
        // otherwise we'll need one of these:
        filerec_ptr = new ocin_gen_filerec;
      }

      // cycle count is 16 bits in OPN traces
      inj_cycle = (high_inj_cycle << 16) |
        (tsim_u64)(comp_temp & 0xffff) - cycle_offset;

      if (inj_cycle < last_inj_cycle) { // rollover occurred
        high_inj_cycle++;
        inj_cycle = (high_inj_cycle << 16) |
          (tsim_u64)(comp_temp & 0xffff) - cycle_offset;
      }

      // now that that is done we can sent the new last_inj_cycle
      last_inj_cycle = inj_cycle;

      comp_temp = (comp_temp>>16) &0xffff; // put the netinfo in the
      // lower part for easer
      // retrieval

      // now we can start filling the info:
    
      filerec_ptr->inj_time = inj_cycle;

      // initialzed on new:
      //    filerec_ptr->addr = 0;          // opn has no address associated

      filerec_ptr->src_coord[1] =  0xf & comp_temp;
      filerec_ptr->src_coord[0] = ((0xf0 & comp_temp)>>4);
      filerec_ptr->dst_coord[1] = ((0xf00 & comp_temp)>>8);
      filerec_ptr->dst_coord[0] = ((0xf000 & comp_temp)>>12);
      filerec_ptr->blockid = (int)bid_temp &0x7f;

      //    filerec_ptr->class_type = 0; inited at new
      filerec_ptr->size =  1;    // all opn packets are one flit
      //    filereq_ptr->type = REQ_R;   // inited at new

    } else {                      // OCN traffic trace
      if (is_compressed) {
        count = 0;
        // need a small loop here to get all three words of the entry
        for(int x= 0; x<3; x++) {
          count += gzread(gzpatt_file, &temp[x], 4 ); 
        }
      } else {
        // fread allows us to do this in one step
        count = fread(temp, 4, 3, patt_file); 
        count = count*4;
      }
    
      //      printf("Got %x,%x,%x\n",temp[0],temp[1],temp[2]);
      if (count !=12) {
        return filerec_ptr;       // return a null pointer if we hit the
        // end
      } else {
      
        // otherwise we'll need one of these:
        filerec_ptr = new ocin_gen_filerec;
      }
    
      // cycle count is 32 bits in OCN traces
    
      //    if(cycle_offset != 0) {
      //      cout << "Got hi_inj: "<< high_inj_cycle<< " temp[0]:" <<temp[0] << " cycle_offset:" <<cycle_offset <<endl; 
      //    }
      inj_cycle = ((high_inj_cycle <<32) + (tsim_u64)temp[0] - cycle_offset) ;
      //      printf("Inj cycle %x\n",inj_cycle);
      //if (last_inj_cycle != 0) { // rollover occurred
      if (inj_cycle < last_inj_cycle) { // rollover occurred
        high_inj_cycle++;
        inj_cycle = (tsim_u64)((high_inj_cycle <<32) + (tsim_u64)temp[0] - cycle_offset) ;
      }

      // now that that is done we can sent the new last_inj_cycle
      last_inj_cycle = inj_cycle;


      // now we can start filling the info:
    
      filerec_ptr->inj_time = inj_cycle;

      // initialzed on new:
      filerec_ptr->addr = (tsim_u64)temp[1]; // second word is system
      // address

      if (ocn_proper) {
      
        filerec_ptr->src_coord[1] = 0xf & temp[2];
        filerec_ptr->src_coord[0] = 3- ((0xf0 & temp[2])>>4);
        // destination determined by sys address...
        //    filerec_ptr->dst_coord[1] = ((0xf00 & comp_temp)>>8);
        //    filerec_ptr->dst_coord[0] = ((0xf000 & comp_temp)>>12);

        get_dst_translation(filerec_ptr); // this is where addr->dst
        // calc'ed
      } else {
        spread_translation(filerec_ptr);
      }

      filerec_ptr->blockid = 0;   // no blockid in ocn traces

      filerec_ptr->class_type = ((0xf0000 & temp[2])>>16);
      filerec_ptr->size =  ((0x700000 & temp[2]) >> 20);

      if ((temp[2] & 0x1000000) > 0) { // R=1 for reads
        filerec_ptr->type = REQ_R;       
      } else {
        filerec_ptr->type = REQ_W;
      }

    }

#ifdef DEBUG
    //    if (cycle_offset !=0) 
    {
      stringstream tmp;
      tmp  << "Read trace record: " <<filerec_ptr->printsstr();
      ocin_name_warn(name,tmp.str());
    }
#endif
    
  }
  return filerec_ptr;

}

// this function performs the sys addr to network addr translation,
// currently it is locked to the OCN config from the prototype, could
// be different though..
void ocin_gen_file::get_dst_translation(ocin_gen_filerec * filerec_ptr){
  unsigned int low_mt;
  unsigned int split_bit;
  unsigned int mt_num;
  tsim_u64 addr = filerec_ptr->addr;

  low_mt = (addr & 0x1C0) >> 6;
  split_bit = (addr & 0x20000) >> 17;

  mt_num = low_mt + (split_bit << 3);

  unsigned int tempout = trans_table[mt_num];
  
  filerec_ptr->dst_coord[0] = (0xf & tempout);

  filerec_ptr->dst_coord[1] = ((0xf0 & tempout)>>4);

}

// this function performs the sys addr to src and dst network
// addresses.  Assumptions are that the network is power of 2 in y and
// a power of 2 plus 1 in x.  The src will be lined up on the X=0
// column and the dsts will be spread across the remaining X=1 to
// X=max_x columns.
void ocin_gen_file::spread_translation(ocin_gen_filerec * filerec_ptr){
  tsim_u64 addr = filerec_ptr->addr;


  //filegen_num |  recoding
  //   0        |   as in trace
  //   1        |   Y inverted
  //   2        |   X inverted
  //   3        |   X and Y inverted
  
  filerec_ptr->src_coord[0] = 0;
  filerec_ptr->src_coord[1] = (addr & srcymask) >> L1OFFSET;
  filerec_ptr->dst_coord[0] = ((addr & dstxmask) >> dstxshift) +1;
  filerec_ptr->dst_coord[1] = (addr & dstymask) >> L2OFFSET;

  //   if (cycle_offset !=0) {
  //     stringstream tmp;
  //     tmp  << "Default: " <<filerec_ptr->printsstr();
  //     ocin_name_warn(name,tmp.str());
  //   }


  switch (filegen_num) 
    {
    case 0 :
      filerec_ptr->src_coord[0] = 0;
      filerec_ptr->src_coord[1] = (addr & srcymask) >> L1OFFSET;
      filerec_ptr->dst_coord[0] = ((addr & dstxmask) >> dstxshift) +1;
      filerec_ptr->dst_coord[1] = (addr & dstymask) >> L2OFFSET;
      break;
    case 1 :
      filerec_ptr->src_coord[0] = 0;
      filerec_ptr->src_coord[1] = max_y - ((addr & srcymask) >> L1OFFSET);
      filerec_ptr->dst_coord[0] = ((addr & dstxmask) >> dstxshift) +1;
      filerec_ptr->dst_coord[1] = max_y - ((addr & dstymask) >> L2OFFSET);
      break;
    case 2 :
      filerec_ptr->src_coord[0] = max_x;
      filerec_ptr->src_coord[1] = (addr & srcymask) >> L1OFFSET;
      filerec_ptr->dst_coord[0] = (max_x -1) - ((addr & dstxmask) >> dstxshift);
      filerec_ptr->dst_coord[1] = (addr & dstymask) >> L2OFFSET;
      break;
    case 3 :
      filerec_ptr->src_coord[0] = max_x;
      filerec_ptr->src_coord[1] = max_y - ((addr & srcymask) >> L1OFFSET);
      filerec_ptr->dst_coord[0] = (max_x -1) - ((addr & dstxmask) >> dstxshift);
      filerec_ptr->dst_coord[1] = max_y - ((addr & dstymask) >> L2OFFSET);
      break;
    default :
      cout << "Error: filegen_num = "<<filegen_num;
      break;
    }

  //   if (cycle_offset !=0) {
  //     stringstream tmp;
  //     tmp  << "Reconfiged: " <<filerec_ptr->printsstr();
  //     ocin_name_warn(name,tmp.str());
  //   }
}

// this function hides the complexity of multi file access from the
// upper levels. Returns a filerec pointer, or NULL if all files are
// done.
ocin_gen_filerec * ocin_gen_file::readfile() {

  ocin_gen_filerec * filerec_ptr;
  // First attempt to get a record from the current file
  filerec_ptr = get_record();

  // If its null and this is multipart, close the current file and
  // open next file (if we can), then read a record from next file
  if (is_multipart) {
    if(filerec_ptr == NULL) {
      closefile();

      // figure out the name of the next part...

      stringstream tmp_tail;
      tmp_tail << filepart;
      if (is_compressed) {
        tmp_tail  << ".bin.gz";
      }else {
        tmp_tail  << ".bin";
      }
      string::size_type begintail = 
        param_tracefile_name.find( tmp_tail.str(), 0 );

      // isolate the basename
      string basename = param_tracefile_name.substr(0, begintail);

      // create a new tail
      filepart++;
      stringstream newname;
      newname <<basename << filepart;
      if (is_compressed) {
        newname  << ".bin.gz";
      }else {
        newname  << ".bin";
      }
      param_tracefile_name = newname.str();

      // using stat to determine if the file exists

      struct  stat    statinfo;

      if(stat(param_tracefile_name.c_str(),&statinfo) != -1) {
        // file exists we should be able to call openfile safely now
        openfile();

        // and now we can run get record again
        filerec_ptr = get_record();

      } // otherwise we just keep our null in filerec_ptr so update
        // knows the file is done
    }

  } // if its not multifile then we are just done, pass the null
    // upstream

  return filerec_ptr;
  
}


// This function pulls records from the file and places then in the
// current records list.  It stops pulling once it hits a record that
// is past the current cycle. This function also looks through the
// curr_reps list to see if there are any to fold into the curr_reqs.
void ocin_gen_file::pull_records() {

  tsim_u64 last_cycle;
  ocin_gen_filerec * filerec_ptr;
  
  if (filecomplete == false) {
    // only pull new records if the tail record is out of date
    if (!curr_recs.empty()) {
      cr_it = curr_recs.end();
      cr_it--;
      last_cycle = (*cr_it)->inj_time; // inj time of last record
      
    } else {                  // if the recs are empty just mark it
      // as 0
      last_cycle = 0;
    }
    
    while ((last_cycle <= ocin_cycle_count) && (filecomplete == false)) {
      filerec_ptr = readfile();

      if (filerec_ptr == NULL) {
        filecomplete = true;
        gen_done = true;
      } else {
        last_cycle = filerec_ptr->inj_time;

        //        cout << "Pushing: " <<filerec_ptr->printsstr()<<endl;

        curr_recs.push_back(filerec_ptr);
      

        if ((!opn) &&(!splash)) {               // if this is an ocn trace
          // then we must create the reply filerec_ptr too
          ocin_gen_filerec * filerep_ptr = new ocin_gen_filerec;
          
          // have to adjust inj time by hop dist (as a hack...)
          int dist = 
            abs(filerec_ptr->src_coord[0] -  filerec_ptr->dst_coord[0]) 
            + abs(filerec_ptr->src_coord[1] - filerec_ptr->dst_coord[1]);
          
          filerep_ptr->inj_time = filerec_ptr->inj_time + dist +3;
          filerep_ptr->addr =  filerec_ptr->addr;        
          
          // flipping src and dst for rep
          filerep_ptr->src_coord[1] = filerec_ptr->dst_coord[1];
          filerep_ptr->src_coord[0] = filerec_ptr->dst_coord[0];
          filerep_ptr->dst_coord[1] = filerec_ptr->src_coord[1];
          filerep_ptr->dst_coord[0] = filerec_ptr->src_coord[0];
          
          filerep_ptr->blockid = 0;   // no blockid in ocn traces
          
          // assuming rep is the next class higher
          filerep_ptr->class_type = filerec_ptr->class_type + 1;
          
          // here we fix size correctly for both req and rep
          if(filerec_ptr->type == REQ_R) {
            filerep_ptr->type = REP_R;
            filerep_ptr->size = filerec_ptr->size;
            filerec_ptr->size = 1; // fixing req size
          } else {                // writes are big on req, small on rep
            filerep_ptr->type = REP_W;
            filerep_ptr->size = 1;
          }
        
          curr_reps.push_back(filerep_ptr);
#ifdef DEBUG
          {
            stringstream tmp;
            tmp  << "Creating Reply record: " <<filerep_ptr->printsstr();
            ocin_name_debug(name,tmp.str());
          }
#endif

        }
      }
    }
  } 

  // Finally here we search through the curr_reps looking for any that
  // have passed so we can put them in the reqs list and inject them
  if ((!opn) && (!splash)) {
    for(cp_it = curr_reps.begin(); cp_it != curr_reps.end(); cp_it++) {
      if ((*cp_it)->inj_time <= ocin_cycle_count) {
        // then pull it from there and put it in curr_recs
        
        filerec_ptr = (*cp_it);
        curr_reps.erase(cp_it);
        cp_it = curr_reps.begin(); // gotta restart iterater after an
        // erase
        curr_recs.push_front(filerec_ptr);
      }
    }
  }

}
      
          

// since the generator does not take any inputs from other modules we
// can do all our work in here:
void ocin_gen_file::update() {

  // first make sure that the curr_recs is up to date
  if(!filecomplete) {
    pull_records();
  }


  int dest_count = dsts.size();
  unsigned dst_idx;

  // now pull entries from curr_recs until its empty or inj_time is
  // later than current time.

  if (!curr_recs.empty()) {

    cr_it = curr_recs.begin();
    tsim_u64 last_cycle = (*cr_it)->inj_time; // inj time of first record

    bool done;

    //    cout << "last cycle: "<<last_cycle<<" curr_cycle:" 
    //         << ocin_cycle_count <<endl;

    if (last_cycle <= ocin_cycle_count) {
      done = false;
    }else{
      done = true;
    }
    while (!done) {

      ocin_gen_filerec * filerec_ptr = *(cr_it);
      
      // inject the packet      
      gen_packet(filerec_ptr); // our local overload of the parent
                               // class's gen_packet.
	  
	  
      // clean up stuff
      curr_recs.pop_front();
      delete filerec_ptr;

      //      cout << "curr_recs size: " <<curr_recs.size() <<endl;
      
      // we are done if the curr_recs is empty or if the injtime is
      // greater than now.
      if (!curr_recs.empty()) {
        cr_it = curr_recs.begin();
        last_cycle = (*cr_it)->inj_time; // inj time of first record
        if (last_cycle > ocin_cycle_count) {
          done = true;
        }

      } else {
        done = true;            // no entries left
      }


    }
  }    
}

void ocin_gen_file::finalize() {
}



// this version of gen_packet (unlike the ocin_gen one) takes a
// filerec pointer and uses that to produce a packet
void ocin_gen_file::gen_packet(ocin_gen_filerec * filerec_ptr) {
  stringstream src_term, dst_term;
  string src_tname, dst_tname;   // terminals
  string src, dst;             // nodes
  ocin_node_info * src_info;
  ocin_node_info * dst_info;

  // convert src & dest terminal coordinates to strings
  src_term << "term." << filerec_ptr->src_coord[0] << "." << filerec_ptr->src_coord[1];
  dst_term << "term." << filerec_ptr->dst_coord[0] << "." << filerec_ptr->dst_coord[1];
  src_tname = src_term.str();
  dst_tname = dst_term.str();


  ocin_msg * msg_ptr = NULL;

  // Src terminal valid?
  if (terminal2node_map.find(src_tname) != terminal2node_map.end()) {
    // found the source...
    src = terminal2node_map[src_tname];
    src_info  = &(node_inst_data[src]);
      
    // now make sure that the packet is one of our legal dsts
    if(terminal2node_map.find(dst_tname) != terminal2node_map.end()) {
      dst = terminal2node_map[dst_tname];
      dst_info  = &(node_inst_data[dst]);
          
      // first, set msg size

      // FIXME: Currently using param_flit_max to make sure
      // ocin_gen::gen_packet makes the correct sized packet, very
      // ugly hack...
      if (splash) {
        int pkt_size_in_bits;
        if (filerec_ptr->size == 2)  	 // orig Splash trace: 2 flit request 
        	pkt_size_in_bits = 64; 
        else if (filerec_ptr->size == 5) // orig Splash trace: 5 flit reply
        	pkt_size_in_bits = 288;
        else 							 // new traces: pkt size in bytes
        	pkt_size_in_bits = filerec_ptr->size * 8;
        
        // compute flit count
        param_flit_max = (tsim_u64)ceil(pkt_size_in_bits / (float)param_bisection_channel_width);
      }
      else {
        param_flit_max = filerec_ptr->size;
      }  

#ifdef DEBUG
      {
        stringstream tmp;
        tmp  << "Injecting packet from " << src_tname << "(" << src << ") --> " << dst_tname << "(" << dst << "). Size: " << param_flit_max;  // terminal,node
        ocin_name_debug(name,tmp.str());
      }
#endif
      // Now we can use the generic packet generator.
      ocin_msg * new_msg = ocin_gen::gen_packet(src_tname, dst_tname);

      int flit_cnt = new_msg->flits.size();
      for (int y= 0; y < flit_cnt; y++) {
        (local_iu_ptrs_map[src_tname])->push_flit(new_msg->flits[y]);
      }

    }
    else {
      // if we get here then we could not find the dest
      stringstream tmp;
      tmp  << "Tracefile packet has illegal destination: "<< filerec_ptr->printsstr() << ". Dest terminal name: " << src_tname;
      ocin_name_fatal(name,tmp.str());
      exit(1);        	
    }           

  } 
  else {
    // if we get here then we could not find the source in srcs        
    stringstream tmp;
    tmp  << "Tracefile packet has illegal source: " << filerec_ptr->printsstr() << ". Src terminal name: " << src_tname;
    ocin_name_fatal(name,tmp.str());
    exit(1);
  }
}       


      
