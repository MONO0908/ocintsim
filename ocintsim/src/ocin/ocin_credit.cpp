#include "ocin_credit.h"


void ocin_vc_status::init(const string &_name) {
	name = _name;
	max_credits = credits;
	avail = true;
	
	cost = 1000 + param_flit_max;
}


/* allocate()
 *  - assign this VC to a given msg
 */
void ocin_vc_status::allocate(ocin_msg  *msg) {
	avail = false;
	
	// track cost
	cost -= (1000 + msg->flits.size());
}


/* inc_credits():
 *  - increment the credit count for the downstream VC
 *  - if the VC was is eligible for re-alloc, update the free_vc_list
 *  - called by IU & INJ
 */
void ocin_vc_status::inc_credits() {

  credits++;
  // track cost
  cost++;
	  
#ifdef DEBUG
  {
    stringstream tmp;
    tmp  << "receiving credit @ vc_idx " << vc_idx 
         << ". Updated credits = " << credits;
    ocin_name_debug(name,tmp.str());
  }
#endif		
	
		// VC can be freed after an increment in 2 cases:
		// 1. (aggressive alloc): credit count == 1
		// 2. (conservative alloc): credit count == max credits
		// In both cases, EOM must have been sent prior to freeing of the VC (avail=1).
		// The reason why the free_vc_list is updated only when credit count == 1
		// with aggressive VC alloc is that the count must have been 0 
		// prior to this update. (Tail flit was sent, decrementing credits to 0.)
		// If the tail was sent out and credit count > 0 with aggressive VC re-use,
		// the VC would have been re-allocated at that point (in dec_credits() ).
		if (avail && 
		    ( (!param_aggressive_vc_alloc && (credits==max_credits)) ||
		       (param_aggressive_vc_alloc && (credits==1)))) {
			free_vc_list->push_back(vc_idx);

#ifdef DEBUG
            {
              stringstream tmp;
              tmp  << "Updated free_vc_list: " << deque2str(free_vc_list);
              ocin_name_debug(name,tmp.str());
            }
#endif
			// update cost
			cost += 1000;
		}
}


/* dec_credits()
 *  - decrement the credit count for the downstream VC
 *  - free the VC if it's eligible
 *  - called by XB alloc
 */
void ocin_vc_status::dec_credits(bool eom) {
	
	credits--;
	
#ifdef DEBUG
        {
          stringstream tmp;
          tmp  << "dec credit @ vc_idx " << vc_idx 
               << ". Updated credits = " << credits;	
          ocin_name_debug(name,tmp.str());
        }

#endif

	// if the flit using up the credit is a tail (end-of-msg),
	// we can actually re-allocate the VC
	if (eom) {
		avail = true;
		if (param_aggressive_vc_alloc && (credits > 0)) {
			free_vc_list->push_back(vc_idx);

#ifdef DEBUG
            {
              stringstream tmp;
              tmp  << "EOM: freeing a VC..  Updated free_vc_list: " << deque2str(free_vc_list);
              ocin_name_debug(name,tmp.str());
            }

#endif			
		}
	}
}

