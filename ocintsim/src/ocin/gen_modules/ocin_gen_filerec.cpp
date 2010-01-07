#include "ocin_gen_filerec.h"

string ocin_gen_filerec::printsstr () {
  stringstream temp;
  temp << "Inject time: " << tsim_u64tos(inj_time) << " Addr: " 
       << tsim_u64tohs(addr) << " size:" << size << " Type: " 
       << (int)type <<" SRC:" <<src_coord[0] <<"," << src_coord[1] 
       << " DST:"  <<dst_coord[0] <<"," << dst_coord[1]  
       << " blockid:" << blockid <<"\n";
  return temp.str();
};


string TraceFormat::str() {
	stringstream tmp;
	tmp << "  Inject time: " << tsim_u64tos(insertion_time) << endl
	    << "  Src x.y:  " << src_node_x << "." << src_node_y << endl
	    << "  Dst x.y:  " << dst_node_x << "." << dst_node_y << endl
	    << "  Packet size: " << packet_size << endl;
	return tmp.str();
}

std::ostream& operator << (std::ostream& trace_stream, const TraceFormat& trace_packet) {
  trace_stream << trace_packet.insertion_time << " ";
  trace_stream << trace_packet.src_node_x     << " ";
  trace_stream << trace_packet.src_node_y     << " ";
  trace_stream << trace_packet.dst_node_x     << " ";
  trace_stream << trace_packet.dst_node_y     << " ";
  trace_stream << trace_packet.packet_size    << std::endl;

  return trace_stream;
}


std::istream& operator >> (std::istream& trace_stream, TraceFormat& trace_packet) {
  trace_stream >> trace_packet.insertion_time;
  if(trace_stream.eof()) {
    return trace_stream;
  }
  trace_stream >> trace_packet.src_node_x;
  trace_stream >> trace_packet.src_node_y;
  trace_stream >> trace_packet.dst_node_x;
  trace_stream >> trace_packet.dst_node_y;
  trace_stream >> trace_packet.packet_size;

  return trace_stream;
}
