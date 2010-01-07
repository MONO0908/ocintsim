#include "ocin_router_xballoc.h"

void ocin_router_xballoc::init(ocin_router *parent, const string &_name) {
    name = _name;
	parent_router = parent;
}

