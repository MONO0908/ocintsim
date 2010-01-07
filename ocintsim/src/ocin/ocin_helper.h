#ifndef OCIN_HELPER_H_
#define OCIN_HELPER_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include <list>
#include <string>

ocin_msg * gen_msg(int num_flits);

// to string methods
string msg2str(ocin_msg *msg, bool print_flits);
string flit2str(ocin_flit *flit);
string flit_type2str(ocin_flit *flit);
string vector2str(vector <int> *v);
string deque2str(deque <int> *d);
string list2str(list <int> *l);
string set2str(set <int> *s);
string map_set2str(map <int, set <int> > *m);
string quadrant2str(Quadrant q);
string fcs();
string fcns(string name);

string msg2sptrace(ocin_msg *msg);

// Prioritized matching
int priority_match(list <int> *priority_list, 
                   vector <int> *request_list, 
                   bool update);
int priority_match(list <int> *priority_list, 
                   vector <int> *request_list, 
                   vector <int> &vc2port,
                   bool update);
void update_priority(list <int> *priority_list, int r); 

// Debuging and logging messages:
void ocin_debug(const string &str);
void ocin_warn(const string &str);
void ocin_fatal(const string &str);
void ocin_name_debug(const string& name, const string &str);
void ocin_name_warn(const string& name, const string &str);
void ocin_name_fatal(const string& name,const string &str);
 
// Economically give out random bits: 
//unsigned ocin_rand_bits(int bits);

/* Calculate the min-path hop count for a msg */
int ocin_count_hop(ocin_msg *msg);

/* Mappings between dimensions, directions, and out ports */
// Conventional 2D mesh (no ECs)
inline int get_port(int dim, int dir) { return ((dim * 2) + dir); };
inline int get_dim (int port)        { return (port / 2); };
inline int get_dir (int port)        { return (port % 2); };
// EC/BEC mesh (ppd = ports per dir)
inline int get_bec_port(int port, int dir_idx, int ppd) { return port + (dir_idx*ppd); }; 
inline int get_bec_dim (int port, int total_ports) { return (port >= (total_ports / 2)); };

/* ordinal directions */
inline int isXdir (int port) { return ( (port == EAST) || (port == WEST)); };
inline int isYdir (int port) { return ( (port == SOUTH) || (port == NORTH)); };


// support for quadrants
Quadrant get_quadrant (vector <int> src_addr, vector <int> dest_addr);
vector<int> get_quadrant_output_ports (Quadrant q);
vector<int> get_quadrant_input_ports (Quadrant q); 

// more support for port mappings
vector<int> get_dim_output_ports(int dim);
vector<int> get_all_network_ports(int exclude_port); 
inline int pin2pout_straight(int p_in) { return get_port(get_dim(p_in), (get_dir(p_in)+1)%2); }; 


#endif /*OCIN_HELPER_H_*/
