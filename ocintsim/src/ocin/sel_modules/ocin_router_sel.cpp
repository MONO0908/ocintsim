#include "ocin_router_sel.h"

void ocin_router_sel::init(ocin_router *parent, const string &_name) {
	parent_router = parent;
	name = _name;
	
	// max int
	MAX_INT = numeric_limits<int>::max();

#ifdef DEBUG	
        {
          stringstream tmp;
          tmp   << "Initialized.";
          ocin_name_debug(name,tmp.str());
        }
#endif
}



