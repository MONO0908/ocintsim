#include "ocin_router_ru.h"

void ocin_router_ru::init(ocin_router *parent, const string &_name) {
	parent_router = parent;
	name = _name;

#ifdef DEBUG	
        {
          stringstream tmp;
          tmp   << "Initialized.";
          ocin_name_debug(name,tmp.str());
        }
#endif
}

