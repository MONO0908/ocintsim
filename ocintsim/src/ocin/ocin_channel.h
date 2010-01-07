#ifndef OCIN_CHANNEL_H_
#define OCIN_CHANNEL_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_credit.h"
#include "ocin_cost_msg.h"
#include "ocin_mon_chan.h"
#include <list>

/* Channel definition:
 * A channel is composed of control & data signals.
 * Data are flits.
 * Control are side-band signals for things like credits & congestion notification.
 * To model a channel with latency >1 cycle, arrival_time is used to indicate
 * when the flit (or control) is ready to be pulled by the receiver.
 * To model a pipelined or logically partitioned channel, all signals and their 
 * arrival_time are lists. Thus, transmitter pushes data & arrival times to the end 
 * of the list and receiver pulls from the front.
 */
class ocin_channel {
  public:
    string name;
  
    ocin_channel() {};
    void init(const string &_name, int delay);
    
    // tx functions
    void transmit_flit(ocin_flit *flit);
    void transmit_credit(ocin_credit *credit);
    void transmit_cost(ocin_cost_msg *cost_msg);
    
    // rcv functions
    ocin_flit     * receive_flit();
    ocin_credit   * receive_credit();
    ocin_cost_msg * receive_cost();

    ocin_mon_chan mon;
  
  private:
    int _wire_delay;
    int _rca_serial_delay;      // extra delay for low-BW RCA
    
    list <ocin_flit *> _data;
	
	list <ocin_credit *> _credits;

	list <ocin_cost_msg *> _cost;
    
};

#endif /*OCIN_CHANNEL_H_*/
