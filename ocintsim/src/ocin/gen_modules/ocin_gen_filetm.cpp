/* FileTM
 * Supports a generic trace format of the form:
 * <inj_time, src_x, src_y, dst_x, dst_y, msg_size>, where msg_size is in bits.
 * Trace format is defined in ocin_gen_filerec, class TraceFormat.
 * Initially, used to support TMLinux traces from Witchell's group, hence the name.
 */

#include "ocin_gen_filetm.h"

void ocin_gen_filetm::init (tsim_object *parent, string _name) {
  
  gen_done = false;

  name = _name;
  
  tsim_module::init (parent, name);
  
  top_p = (ocin_top *) parent;
  
  // setup the packet monitor
  mon = new ocin_mon_packet;
  mon->init(name, nodes.size());

#ifdef DEBUG
  {
    stringstream tmp;
    tmp  << "Initializing Packet Generator";
    ocin_name_debug(name,tmp.str());
  }
#endif


  // To speed things up a bit we are creating an order'ed vector of
  // local_iu pointers here and I'll iterate over them instead using
  // the node map directly.

  // iterate over the sources and push pointers onto a list
  vector<string>::iterator srcs_it;
  for (srcs_it = srcs.begin(); srcs_it != srcs.end(); srcs_it++) {
  	string src_term = srcs_it->c_str();
  	string src = terminal2node_map[src_term];
  	int port = terminal2port_map[src_term];
    local_iu_ptrs.push_back(&(nodes[src].local_iu[port]));
    local_iu_ptrs_map[src_term] = &nodes[src].local_iu[port];

#ifdef DEBUG
    {
      stringstream tmp;
      tmp  << "Packet Gen source:"<<srcs_it->c_str();
      ocin_name_debug(name,tmp.str());
    }
#endif
  }

  vector<string>::iterator dsts_it;
#ifdef DEBUG
    {

      for (dsts_it = dsts.begin(); dsts_it != dsts.end(); dsts_it++) {
        stringstream tmp;
        tmp  << "Packet Gen ejector:"<<dsts_it->c_str();
        ocin_name_debug(name,tmp.str());
      }
    }
#endif

  // First we need to establish the max for each coord
  int dimensions = DIMENSIONS;
  max_coords.assign( dimensions, 0); // set it up with 0's first
  max_coords[0] = param_num_x_tiles - 1;
  max_coords[1] = param_num_y_tiles - 1;
  
  // open the trace file for reading
  traceFile.open(param_tracefile_name.c_str(), ifstream::in);
  
  // clear next_rec
  next_rec.set_invalid(); 
  
  // clear EOF flat
  eof_reached = false;
}


/* Generate a packet */
void ocin_gen_filetm::inject_packet(TraceFormat *rec) {
	stringstream src_term, dst_term;
	src_term << "term." << rec->src_node_x << "." << rec->src_node_y;
	dst_term << "term." << rec->dst_node_x << "." << rec->dst_node_y;
	
	string src_tname = src_term.str();
	string dst_tname = dst_term.str();


	param_flit_max = (tsim_u64)ceil(rec->packet_size / (float)param_bisection_channel_width); 
	ocin_msg * new_msg = gen_packet(src_tname, dst_tname);
	
	int flit_cnt = new_msg->flits.size();
    for (int y= 0; y < flit_cnt; y++) {
      
      (local_iu_ptrs_map[src_tname])->push_flit(new_msg->flits[y]);
// DELETE
//cout << "Injecting flit #" << y << ": " << src_tname << "->" << dst_tname << endl;
    }
}



// since the generator does not take any inputs from other modules we
// can do all our work in here:
void ocin_gen_filetm::update() {
  
  
  unsigned src_count = srcs.size();
  unsigned dst_idx, src_idx;
  
  TraceFormat rec;

  while(true) {


  	// read a new packet  *OR*  
  	//  retry a postponed packet that has already been read from file?
  	if (next_rec.is_invalid() && !eof_reached) {
		// read the next record from file
    	traceFile >> rec;
    	
    	// Reached EOF? 	
	    if(traceFile.eof()) {
	    	cout << "EOF detected! " << endl;
	      	eof_reached = true;
	      	break;
	    }
    	
// DELETE
//cout << fcns(name) << "Record read! " << rec.insertion_time << endl;
  	}
  	else if (!next_rec.is_invalid()) {  
	    // retry a postponed record
  		rec = next_rec;
  	}
  	else {
  		break;
  	}
    
    
    uint64_t rec_time = rec.insertion_time;
    
    if (rec_time > ocin_cycle_count) {
    	// Postpone packet injection until ready time
    	next_rec = rec;
    	
    	break;
    }
    else if (rec_time < ocin_cycle_count) {
    	// Deadline passed!
    	stringstream tmp;
      	tmp << "Packet deadline passed!! Current cycle: " << ocin_cycle_count << endl;
      	tmp << "Packet info: " << endl;
      	tmp << rec.str();
      	ocin_name_fatal(name,tmp.str());
		exit(1);
    }
    else {
    	// Inject the packet	
    	inject_packet(&rec);
    	
    	next_rec.set_invalid();
    }
	
  }
  
  if (eof_reached && next_rec.is_invalid()) {
  	this->sim()->stop();
  	traceFile.close();
  }
}

void ocin_gen_filetm::finalize() {
}
