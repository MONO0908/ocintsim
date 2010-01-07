#ifndef OCIN_COST_MSG_H_
#define OCIN_COST_MSG_H_

#include "tsim.h"
#include "ocin_defs.h"
#include "ocin_helper.h"

class ocin_cost_msg {
  public:
	// support for msg tracking
	static unsigned int unique_id;  // global number
	unsigned int msg_id;            // individual msg number
	
	Quadrant quadrant;              // quadrant corresponding to this msg
	
	int p_out;  // target output port
	int p_in;   // input port over which the msg was received
	
	tsim_u64 arrival_time;    // Used by receiver on the wire. Models the wire delay


	ocin_cost_msg();
	ocin_cost_msg(int new_cost);
	ocin_cost_msg(ocin_cost_msg *msg);
	
	// setters & getters
	inline void update_cost(int new_cost) { _cost = new_cost; };
	inline int get_cost()                 { return _cost; };
	
	// Helpers
	string print2str();
  
  private:
	int _cost;   // cost transmitted by the msg 
	
	void init();
};

#endif /*OCIN_COST_MSG_H_*/
