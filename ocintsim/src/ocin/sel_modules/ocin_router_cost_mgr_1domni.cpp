/* Simple manager that updates the cost register 
 * with its rolling history sum. 
 */

#include "ocin_router_cost_mgr_1domni.h"

void ocin_router_cost_mgr_1domni::init(ocin_router *parent, const string &_name) {
	
	// call base init() w/ fully specified name
	stringstream *ss = new stringstream();
	*ss << parent->node_info->name.c_str() << "." << _name;
	ocin_router_cost_mgr::init(parent, ss->str());
	delete ss;
        nodes = NULL;
	
#ifdef DEBUG	
        {
          stringstream tmp;
          tmp   << "Initialized. Cost manager = 1DOMNI";
          ocin_name_debug(name,tmp.str());
        }
#endif
}


/* update_cost()
 *  - simple updater, uses the rolling history sum as the cost 
 */
void ocin_router_cost_mgr_1domni::update_cost() {
  if (nodes==NULL) {
    nodes = parent_router->nodes;
    //    cout << "nodes in csmgr "<<nodes<<endl;
  }


  string localname = name;

  for (int i=0; i < (_cost_regs[0].size() -1); i++) { // i is output port
    vector <int> costs;

    int cost =0;
    costs.push_back(_cost_regs[0][i].get_hist_sum());// first grab the
                                                // local cost

    vector <int> thiscoord = parent_router->node_info->coord; /* router's
                                                                 multi-dimensional
                                                                 coordinates */
    ocin_router * nextnode;

    // then we recurse the correct direction
    bool done = false;
    while (done != true) {
      stringstream d;           // prosepective downstream node name
      int dir = get_dir(i);
      int dim = get_dim(i);

      // get coordinates of downstream node
      if (dir ==0) {
        thiscoord[dim]++;
      } else {
        thiscoord[dim]--;
      }

      d << thiscoord[0] << "." << thiscoord[1];
      string next = d.str();
      if ((*nodes).find(next) == (*nodes).end()) {
        done = true;            // past the end of the network

        // {
        //   stringstream tmp;
        //   tmp   << "Couldn't find node:" <<thiscoord[0]<<"."<<thiscoord[1]<<endl;
        //   ocin_name_warn(name,tmp.str());
        // }

      } else {
        nextnode = &((*nodes)[next]);
        
        costs.push_back(nextnode->cost_regs[0][i].get_hist_sum());
        // {
        //   stringstream tmp;
        //   tmp   << "Getting cost from node:"<<thiscoord[0]<<"."<<thiscoord[1]<< " in dir of port " << i <<endl;
        //   ocin_name_warn(name,tmp.str());
        // }

      }
    }

    cost = costs.back();        // so we dont div the first one

    // decaying average:
    while(!costs.empty()) {
      cost = (cost + costs.back())/2;
      costs.pop_back();
    }

    _cost_regs[0][i].set_cost(cost);
    
// #ifdef DEBUG	
//     {
//       stringstream tmp;
//       tmp   << "Cost reg @ pout " << i << " = " << cost;
//       ocin_name_debug(name,tmp.str());
//     }
// #endif
    
  }
}
