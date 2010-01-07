#include "ocin_helper.h"

/* gen_msg(num_flits):
 *  num_flits: # of flits to create
 *  - Allocs a new msg object
 *  - Creates num_flits flits
 */
ocin_msg * gen_msg(int num_flits) {
  ocin_msg *msg = new ocin_msg();
  //  msg->vc_set.clear();
  for (int i=0; i<num_flits; i++) {
    ocin_flit *flit = new ocin_flit(msg, i);
    // set header & tail flags
    flit->is_header = (i == 0) ? true : false;
    flit->is_tail = (i == num_flits-1) ? true : false;

    msg->flits.push_back(flit);
  }
  return msg;
}

////////// TO STRING methods //////////

string msg2sptrace(ocin_msg *msg) {
  stringstream ss;
  ss.clear();

  int radix = 8;
  int src = msg->src_addr[0] + msg->src_addr[1]*radix;
  int dst = msg->dest_addr[0] + msg->dest_addr[1]*radix;
  ss << msg->gen_time << " " << src << " " << dst << " " << msg->flits.size() << endl;

  return ss.str();
}


/* msg2str()
 */
string msg2str(ocin_msg *msg, bool print_flits) {
  stringstream ss;
  vector <int> ::iterator iitr;
  set <int> ::iterator sitr;
  vector<ocin_flit *> ::iterator fitr;
  map <int, set <int> >::iterator rtitr;
  set <int>::iterator vcitr;

  ss.clear();
  ss << "\n** MSG pid: "<< msg->pid << endl;

  ss << "src addr = ";
  for (iitr=msg->src_addr.begin(); iitr != msg->src_addr.end(); iitr++) {
    ss << (*iitr) << " ";
  }
  ss << " (term: " << msg->src_t << ")\n";

  ss << "dest addr = ";
  for (iitr=msg->dest_addr.begin(); iitr != msg->dest_addr.end(); iitr++) {
    ss << (*iitr) << " ";
  }
  ss << " (term: " << msg->dst_t << ")\n";
  
  // print the quadrant for a mesh
  if (msg->src_addr.size() == 2) {
    ss << "quadrant = " << quadrant2str(msg->quadrant) << "\n";
  }

  //ss << "rt dimension = " << msg->rt_dimension << "\n";
  //ss << "rt direction = " << msg->rt_dir << "\n";
  ss << "out port = " << msg->p_out << "\n";

  ss << "upstream vc = " << msg->vc_out_index << "\n";

  ss << "routes = \n";
  for (rtitr=msg->routes.begin(); rtitr !=msg->routes.end(); rtitr++) {
    int pout = (*rtitr).first;
    set <int> &vc_set = msg->routes[pout];
    ss << "   rt: "<<pout<< " vcs: ";
    for (vcitr=vc_set.begin();vcitr != vc_set.end();vcitr++) {
      ss << (*vcitr) <<" ";
    }
    ss << "\n";
  }

  ss << "\nreroutes = "<< msg->reroutes;
  ss << "\n";

  if (print_flits) {
    for (fitr=msg->flits.begin(); fitr != msg->flits.end(); fitr++) {
      ss << flit2str(*fitr);
    }
  }
  
  return ss.str();
}


/* flit2str()
 */
string flit2str(ocin_flit *flit) {
  stringstream ss;
  vector <int> ::iterator iitr;

  ss.clear();
	
  ss << "Flit # " << flit->idx << " (" << flit_type2str(flit) << ")\n";
  ss << "  inj time = " << flit->inj_time << "\n";
  ss << "  current location = ";
  for (iitr=flit->curr_addr.begin(); iitr != flit->curr_addr.end(); iitr++) {
    ss << (*iitr) << " ";
  }
  ss << "\n";
  ss << "  current vc = " << flit->vc_idx <<"\n";
  
  return ss.str();
}


/* flit_type2str()
 */
string flit_type2str (ocin_flit *flit) {
  if (flit->is_header)
    return (flit->is_tail ? "SFM" : "SOM");
  if (flit->is_tail)
    return "EOM";
  return "COM";
}


/* vector2str()
 */
string vector2str(vector <int> *v) {
  string s;
  vector <int> ::iterator itr;
  for (itr=v->begin(); itr != v->end(); itr++) {
    char t[3];
    sprintf(t, "%d", *itr);
    s += t;
    s += "  ";
  }
  return s;
}


/* deque2str()
 */
string deque2str(deque <int> *d) {
  string s;
  deque <int> ::iterator itr;
  for (itr=d->begin(); itr != d->end(); itr++) {
    char t[3];
    sprintf(t, "%d", *itr);
    s += t;
    s += "  ";
  }
  return s;	
}


/* list2str()
 */
string list2str(list <int> *l) {
  string s;
  list <int> ::iterator itr;
  for (itr=l->begin(); itr != l->end(); itr++) {
    char t[3];
    sprintf(t, "%d", *itr);
    s += t;
    s += "  ";
  }
  return s;
}



/* set2str()
 */
string set2str(set <int> *s) {
  string ss;
  set <int> ::iterator itr;
  for (itr=s->begin(); itr != s->end(); itr++) {
    char t[3];
    sprintf(t, "%d", *itr);
    ss += t;
    ss += "  "; 
  }
  return ss;
}


/* map_set2str()
 *  - print the content of map <int, set <int>>
 *  - used to print the content of a route map
 */
string map_set2str(map <int, set <int> > *m) {
  stringstream ss;
  map <int, set <int> >::iterator itr;
  for (itr=m->begin(); itr != m->end(); itr++) {
    ss << itr->first << ":  ";
    ss << set2str(&(itr->second)) << endl;
  }
  return ss.str();
}


/* quadrant2str()
 */
string quadrant2str(Quadrant q) {
  switch (q) {
  case NE_Q: return "NE_Q";
  case SE_Q: return "SE_Q";
  case NW_Q: return "NW_Q";
  case SW_Q: return "SW_Q";
  default : return "INVALID";
  }
}

/* fcs():
 *  - Formatted Cycles String
 *  - prints the cycles count in the format: "[NNN]: " 
 */
string fcs() {
  string s;
  char t[15];
  sprintf(t, "%d", ocin_cycle_count);
  s = t;
	
  return ("[" + s + "] ");
}


/* fcns():
 *  - Formatted Cycles & Name String
 *  - prints cycle count & name in the format: "[NNN] name: "
 */
string fcns(string name) {
  string s = fcs();
  string t = name + ": ";
	
  return (s+t);
}

/////// PRIORITIZED MATCHING /////////

/* priority_match():
 *  - Used for resolving arbitration conflicts (VC & X-bar).
 *  - Since we assume that that the number of VCs/port is small and conflicts
 *    for each individual VC are rare, don't bother doing anything clever.
 *    Scan down the lists until a match is found in O(n^2) time.
 *  - if update_priority is set, winner's priority is lowered
 */
int priority_match(list <int> *priority_list, 
                   vector <int> *request_list, 
                   bool update) {
  list <int> ::iterator p_itr;
  vector <int> ::iterator r_itr;
	
  int winner = -1;
	
  for (p_itr = priority_list->begin(); p_itr != priority_list->end(); p_itr++) {
    for (r_itr = request_list->begin(); r_itr != request_list->end(); r_itr++) {
      if ((*p_itr) == (*r_itr)) {
        winner = *r_itr;
        goto winner_found;
      }
    }
  }

 winner_found:

  // update priorities
  if (update && (winner > -1)) {
    //update_priority(priority_list, winner);
    priority_list->erase(p_itr);       // remove the winner from priority list
    priority_list->push_back(winner);  // re-insert @ the end (lowest priority)
  }	
  return winner;
}

/* priority_match():
 *  - the only difference from above is that this version also takes a map used
 *    to translate each request.  The translated request is then used in
 *    comparisons to priority list elements.  The returned value is untranslated.
 */
int priority_match(list <int> *priority_list, 
                   vector <int> *request_list, 
                   vector <int> &translation_map,
                   bool update) {
  list <int> ::iterator p_itr;
  vector <int> ::iterator r_itr;

  int winner = -1;
  int r = -1;          // request id mapped to port or vc index
	
  for (p_itr = priority_list->begin(); p_itr != priority_list->end(); p_itr++) {
    for (r_itr = request_list->begin(); r_itr != request_list->end(); r_itr++) {
      // map vc_id to port/class/etc
      r = translation_map[*r_itr];
      if ((*p_itr) == r) {
        winner = *r_itr;
        goto winner_found;
      }
    }
  }

 winner_found:

  // update priorities
  if (update && (winner > -1)) {
    //update_priority(priority_list, r);
    priority_list->erase(p_itr);       // remove the winner from priority list
    priority_list->push_back(r);       // re-insert @ the end (lowest priority)
  }
  return winner;
}

void update_priority(list <int> *priority_list, int r) {
  list <int> ::iterator p_itr;
	
  for (p_itr = priority_list->begin(); p_itr != priority_list->end(); p_itr++) {
    if (r == (*p_itr)) {
      priority_list->erase(p_itr);       // remove the winner from priority list
      priority_list->push_back(r);       // re-insert @ the end (lowest priority)
      break;     // terminate (the iterator is invalid, anyways)
    }
  }

}


void ocin_debug( const string& msg )
{
  if (tsim_debugging) {
    cout << fcs() << " DGB: " << msg <<endl;
  }
}

void ocin_warn( const string& msg )
{
  cout << fcs() << "WARN: " << msg <<endl;

}

void ocin_fatal( const string& msg )
{
  cout << fcs() << "FATAL: " << msg <<endl;  
}

void ocin_name_debug(const string& name, const string& msg )
{

  if (tsim_debugging) {
    cout << fcs() << " DGB "<<name<< ": " << msg <<endl;
  }
}

void ocin_name_warn(const string& name, const string& msg )
{
  cout << fcs() << "WARN "<<name<< ": " << msg <<endl;

}

void ocin_name_fatal(const string& name, const string& msg )
{
  cout << fcs() << "FATAL "<<name<< ": " << msg <<endl;  
}



// note this is probably only worth it for random sizes <= 16 bits
// after that the accounting gets expensive...
// unsigned ocin_rand_bits(int bits) {

//   static unsigned long long rand_var = 0;
//   static int bits_left = 0;
//   unsigned long long mask = 0;

//   // need to gen a new rand var if the number of bits we need is more
//   // than we have
//   if (bits > bits_left) {
//     rand_var = (rand_var <<32) | ((unsigned long long) random());
//     bits_left += 32;

// #ifdef DEBUG
//     ocin_debug ("Generating a new random var");
// #endif

//   }

//   // need to build the mask (is shifting cheaper than multplying???)
//   for(int x = 0; x<bits; x++) {
//     mask = (mask<<1) | 1;
//   }
  
//   bits_left = bits_left - bits;

//   unsigned ret_var = (unsigned) (rand_var & mask);
//   rand_var = rand_var >> bits;

//   return ret_var;
// }
    
// This function returns the minimum hop count from source to
// destination.
int ocin_count_hop(ocin_msg *msg) {

  int dimensions = msg->src_addr.size();
  int count = 0;
  for (int j = 0; j <dimensions; j++) {
    count += abs(msg->src_addr[j] - msg->dest_addr[j]);
  }
  return count;
}

/* get_quadrant()
 *  - compute the quadrant that a packet will route in
 *    based on src & dest addresses
 *  - mesh node numbering:
 *  
 *    (0,0) ..... (x,0)
 *    .................
 *    .................
 *    (0,y) ..... (x,y)
 */
Quadrant get_quadrant (vector <int> src_addr, vector <int> dest_addr) {
  // make sure we're dealing w/ a 2-D mesh
  if (src_addr.size() != 2) {
    stringstream tmp;
    tmp << "Quadrant calculation is only supported for a 2-D mesh. " << endl;
    tmp << "Current network radix = " << src_addr.size() << endl;     
    ocin_name_fatal("",tmp.str());
    exit(0); 
  }
	
  int x=0;
  int y=1;
  if (src_addr[x] > dest_addr[x]) {
    if (src_addr[y] > dest_addr[y]) {
      return NW_Q;
    }
    else {
      return SW_Q;
    }
  }
  else {
    if (src_addr[y] > dest_addr[y]) {
      return NE_Q;
    }
    else {
      return SE_Q;
    }
  }
}


/* get_quadrant_output_ports
 *  - static mapping for a 2-D mesh 
 */
vector<int> get_quadrant_output_ports (Quadrant q) {
  vector <int> ports;
	
  switch (q) {
  case NE_Q: 
    {
      ports.push_back(0);
      ports.push_back(3);
      return ports;
    }
  case SE_Q: 
    {
      ports.push_back(0);
      ports.push_back(2);
      return ports;
    }
  case NW_Q: 
    {
      ports.push_back(1);
      ports.push_back(3);
      return ports;
    }
  case SW_Q: 
    {
      ports.push_back(1);
      ports.push_back(2);
      return ports;
    }
  default:
    {
      stringstream tmp;
      tmp << "Invalid quadrant specified in get_quadrant_output_ports(). Quadrant =  " << q << endl;     
      ocin_name_fatal("",tmp.str());
      exit(0); 
    }
  }
}


/* get_quadrant_output_ports
 *  - static mapping for a 2-D mesh 
 */
vector<int> get_quadrant_input_ports (Quadrant q) { 
  vector <int> ports;
	
  switch (q) {
  case NE_Q: 
    {
      ports.push_back(1);
      ports.push_back(2);
      return ports;
    }
  case SE_Q: 
    {
      ports.push_back(1);
      ports.push_back(3);
      return ports;
    }
  case NW_Q: 
    {
      ports.push_back(0);
      ports.push_back(2);
      return ports;
    }
  case SW_Q: 
    {
      ports.push_back(0);
      ports.push_back(3);
      return ports;
    }
  default:
    {
      stringstream tmp;
      tmp << "Invalid quadrant specified in get_quadrant_input_ports(). Quadrant =  " << q << endl;     
      ocin_name_fatal("",tmp.str());
      exit(0); 
    }
  }
}


/* get_dim_output_ports(int dim) 
 *  - static mapping for a 2-D mesh
 */
vector<int> get_dim_output_ports(int dim) {
  vector <int> ports;
	
  switch (dim) {
  case 0:
    {
      ports.push_back(0);
      ports.push_back(1);
      return ports;
    }
  case 1:
    {
      ports.push_back(2);
      ports.push_back(3);
      return ports;
    }
  default:
    {
      stringstream tmp;
      tmp << "Invalid dim in get_dim_output_ports(). "
          << "Only 2 dims are currently support (feel free to extend). " << endl;
      tmp << "Dim =  " << dim << endl;     
      ocin_name_fatal("",tmp.str());
      exit(0); 
    }
  }
}


/* get_all_network_ports()
 *  - returns a vector of all network (non-local) ports
 *  - exclude_port param allows to specify a port to exclude
 *  - limited to 2D mesh
 */
vector<int> get_all_network_ports(int exclude_port) {
  vector<int> ports;
  for (int i=0; i<4; i++) {
    if (i != exclude_port)
      ports.push_back(i);
  }
  return ports;
}


