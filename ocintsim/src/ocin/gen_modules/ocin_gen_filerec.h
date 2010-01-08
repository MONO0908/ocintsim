#ifndef OCIN_GEN_FILEREC_H
#define OCIN_GEN_FILEREC_H
#include <sstream>
#include <string>
#include <vector>
#include <stdint.h>

#include "ocin_defs.h"

/* This header contains 2 header formats: 
 *  a) ocin_gen_filerec - bin format used by ocn, opn and splash traces
 *  b) TraceFormat - text format used by Witchell's TM group
 */ 


enum req_t {
  REQ_R = 0,
  REQ_W = 1,
  REQ_S = 2,
  REP_R = 3,
  REP_W = 4,
  REP_S = 5
};

/* Record format used by ocin_gen_file */
class ocin_gen_filerec {
public:
  
  // x,y locations of source and dest
  vector <int> src_coord;
  vector <int> dst_coord;

  // block id for opn records
  int blockid;                  // OPN
  
  tsim_u64 addr;                // system address of request (OCN
                                // reqs)
  
  int size;                     // number of flits

  tsim_u64 inj_time;            // time to inject packet

  req_t type;                // could be read write or swap (OCN)

  int class_type;               // packet class

  ocin_gen_filerec() {             // init everything on creation
    src_coord.push_back(-1);    /* preseed the coords */
    src_coord.push_back(-1);
    dst_coord.push_back(-1);
    dst_coord.push_back(-1);
    addr = 0;
    size = -1;
    inj_time = 0;
    type = REQ_R;
    class_type = 0;
  };
  string printsstr();           /* for nice printing */
};

/* Record format used by ocin_gen_filetm */
class TraceFormat {

 public:
    uint64_t insertion_time;
	int src_node_x;
	int src_node_y;
	int dst_node_x;
	int dst_node_y;
	int packet_size;
	
	TraceFormat() {}
	
	TraceFormat(uint64_t insertion_time,
	            int src_node_x,
	            int src_node_y,
	            int dst_node_x,
	            int dst_node_y,
	            int packet_size) {
	    this->insertion_time = insertion_time;
	    this->src_node_x = src_node_x;
	    this->src_node_y = src_node_y;
	    this->dst_node_x = dst_node_x;
	    this->dst_node_y = dst_node_y;
	    this->packet_size  = packet_size;
	  };
	  
	inline void set_invalid() { this->packet_size = -1; };  
	inline bool is_invalid()  { return (this->packet_size < 0); };
	string str();
};

std::ostream& operator << (std::ostream& trace_stream, const TraceFormat& trace_packet);

std::istream& operator >> (std::istream& trace_stream, TraceFormat& trace_packet);


/* So, to read the entire file, you would do something like this:
	void foo() {
	  ifstream traceFile("trace_file.trace");
	  TraceFormat tfmt;
	
	  while(true) {
	    traceFile >> tfmt;
	    if(traceFile.eof())
	      break;
	
	    // We succesfully read a record into tfmt, now we can process it
	    processTraceRecord(tfmt);
	  }
	}
*/

#endif
