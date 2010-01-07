#include "ocin_cost_msg.h"


ocin_cost_msg::ocin_cost_msg() {
	_cost = 0;
	arrival_time = 0;
	
	// init remaining fields
	init();
}


ocin_cost_msg::ocin_cost_msg(int new_cost) {
	_cost = new_cost;
	
	// init remaining fields
	init();	
}


// copy constructor
ocin_cost_msg::ocin_cost_msg(ocin_cost_msg *msg) {
	_cost = msg->get_cost();
	quadrant = msg->quadrant;
	p_out = msg->p_out;  
	p_in = msg->p_in;
	
	// init remaining fields
	init();	
}


/* init()
 *  - called by all constructors to init common fields
 */
void ocin_cost_msg::init() {
	// assign a unique number to the msg
	msg_id = unique_id;
	ocin_cost_msg::unique_id++;
}


/* print()
 */
string ocin_cost_msg::print2str() {
	stringstream ss;
	ss << "UID = " << msg_id;
	ss << ", quad = " << quadrant;
	ss << ", p_out = " << p_out;
	ss << ". Cost = " << _cost;
	
	return ss.str();
}

unsigned int ocin_cost_msg::unique_id = 0;
