#include "ocin_router_vcalloc.h"

void ocin_router_vcalloc::init(ocin_router *parent, const string &_name) {
    name = _name;
	parent_router = parent;
}
