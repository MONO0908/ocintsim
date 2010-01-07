#ifndef OCIN_ROUTER_RU_H_
#define OCIN_ROUTER_RU_H_

#include "ocin_defs.h"
#include "ocin_interface.h"
#include "ocin_helper.h"
//#include "ocin_router.h"   --- this messes up the inheritance hierarchy

class ocin_router;

class ocin_router_ru {
  public:
    string name;
    string node_name;
    ocin_router *parent_router;
    
    ocin_router_ru() {};
    virtual void init(ocin_router *parent, const string &_name);
    /* all routing units MUST implement route(flit) */
    virtual void route (ocin_flit *flit) = 0; 
    
  protected:
    vector <int> _coord;
    int port_count;      // total # of out ports on this router
    
    /* Support for various BEC routing schemes.
     * These really belong in their respective .h files, but the 
     * sim seg faults if I put them there. */

    // Maps of valid outputs in the X & Y dirs.
    // Retrieved by indexing into these two vectors by 
    //  destination's X and & coordinates, respectively.
    vector <map <int, set <int> > > _x_routes;  
    vector <map <int, set <int> > > _y_routes;
    
    // Support for O1TURN BEC routing.
    // The maps contain same output ports, but different VC sets 
    // segrating X-first and Y-first routes.
    vector <map <int, set <int> > > _xFirst_x_routes, _yFirst_x_routes;  
    vector <map <int, set <int> > > _xFirst_y_routes, _yFirst_y_routes;

};

#endif /*OCIN_ROUTER_RU_H_*/
