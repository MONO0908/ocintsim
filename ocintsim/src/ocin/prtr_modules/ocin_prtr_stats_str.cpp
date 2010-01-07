#include "ocin_prtr_stats_str.h"

bool comp_node (ocin_prtr_stats_node * a, ocin_prtr_stats_node * b){ 

  return (a->distance < b->distance);
}


void ocin_prtr_stats_str::init(map<string, ocin_node_info> &node_inst_data, 
                               map<string, ocin_router> &rtrs) {

  name = "PRTR_STATS";
  // Here we need to setup the stats structure based upone the node
  // info.

  map<string, ocin_node_info>::iterator nid_it; /* iterator for
                                                   node_isnt_data */

  // First create all the stats nodes 
  for (nid_it=node_inst_data.begin(); 
       nid_it != node_inst_data.end(); nid_it++) {

    string name = nid_it->first;
    ocin_node_info * info = &nid_it->second;

    nodes[name] = new ocin_prtr_stats_node; // make stats node

    nodes[name]->edges.assign((info->coord.size())*2, 
                              (ocin_prtr_stats_node *)NULL); // initialize
                                                             // pointer
                                                             // vector
    nodes[name]->weights.assign((info->coord.size())*2, 1); // init
                                                             // weight
                                                             // vector

    nodes[name]->name = name;   // assign name in node
    nodes[name]->rt_ptr = &rtrs[name];


#ifdef DEBUG
    {
      stringstream tmp;
      tmp << "Creating prerouter stats node: " << name ;
      
      ocin_name_debug(name,tmp.str());
    }
#endif


  }

  // Now I can get all the edge pointers sorted out
  for (nid_it=node_inst_data.begin(); 
       nid_it != node_inst_data.end(); nid_it++) {
    string src_name = nid_it->first; // grab src coords
    string dst_name;
    ocin_node_info * info = &nid_it->second;
    vector <int> src_coord = info->coord;
    int p_out;

    // for each port in the src grab dest coords and figure out what
    // port it would be relative the dest.
    for (int x=0; x < info->port_dest.size(); x++) {
      dst_name = info->port_dest[x];
      vector <int> dst_coord = node_inst_data[dst_name].coord; 

      // grab a pointer to the dst stat node
      ocin_prtr_stats_node * dst_ptr = nodes[dst_name];

      bool found = false;
      for (int dim=0; dim<src_coord.size(); dim++){
        if (src_coord[dim] != dst_coord[dim]) {
          //  Figure out the output port for this hop
          int rt_dir = (dst_coord[dim] > src_coord[dim]) ? INC : DEC ; 
          p_out = get_port(dim, rt_dir);

          // place pionter in appropriate edge
          nodes[src_name]->edges[p_out] = dst_ptr;
          found = true;


#ifdef DEBUG
          {
            stringstream tmp;
            tmp << "Connecting prerouter stats node " << src_name << " to " << dst_name << " via port "<<p_out;
            
            ocin_name_debug(src_name,tmp.str());
          }
#endif

          break;
        }
      }

      if (!found) {
        stringstream tmp;
        tmp  << "Error Unable to find output port from "<< src_name << " to "<<dst_name;     
        ocin_name_fatal(src_name,tmp.str());
        exit(1);
      }
    }
  }

  // now reset them:
  reset();

  last_src = "";
  last_cycle = 12341234;        // some random number that wont match
                                // with cycle 0
}

void ocin_prtr_stats_str::print() {
  map<string, ocin_prtr_stats_node *>::iterator nid_it; 

  cout << "Prerouter stats struct data at cycle "<< ocin_cycle_count<<endl;

  for (nid_it=nodes.begin(); nid_it != nodes.end(); nid_it++) {
    string name = nid_it->first;
    ocin_prtr_stats_node * node = nid_it->second;

    stringstream str;
    str << "From node: "<<name<<endl;
    for (int x = 0; x < node->edges.size(); x++) {
      if (node->edges[x] == NULL) {
        continue;
      }
      str << "\tto "<< node->edges[x]->name << " : " << node->weights[x] <<endl;
    }
    cout <<str.str();
  }
}

void ocin_prtr_stats_str::route(ocin_msg * msg, string src, string dest) {

  //  cout <<"Prerouting from "<< src << " to " << dest <<" based upon:\n";
  //  print();

  ocin_prtr_stats_node * current;
  ocin_prtr_stats_node *next;
  current = nodes[src];
  next = NULL;

  int edge_count = current->edges.size();

  if (src == dest) {            // if we are already at the
                                // destination just push on the output
                                // port
    msg->preroutes.clear();
    msg->preroutes.push_back(edge_count);
    return;
  }


  if ((src != last_src)||(ocin_cycle_count != last_cycle)) { // skip
                                                             // re-calc
                                                             // if we
                                                             // don't
                                                             // need
                                                             // it...
  
    reset();                    // reset all the nodes and the
                                // unvisited list


    
    current->distance = 0;
    

    // I'm maintaining a list of unvisited nodes, I start with the
    // source as the current node.  For each neighbor from the current
    // node I relax the distance var and rt_est if the distance is
    // less than the current distance.  I then remove the current node
    // from the unvisited list and look for the next current node from
    // among the neighbors.  If all neighbors have been visited then
    // go to the lowest among the unvisited.
    while (current != NULL) {
      unvisited.remove(current);
      
      int dist=current->distance; // current running distance estimate.
      
      for (int x =0; x < edge_count; x++) {
        next = current->edges[x];
        
        if (next == NULL) {
          continue;               // skip non-connected edges
        }
        
        int next_dist = dist + current->weights[x];
        
        //  Relax the downstream node
        if (next_dist < next->distance) {
          deque <int> new_route = current->rt_est; // route to this point
          new_route.push_back(x);
          //          cout << "Relaxing " << next->name << " to " << next_dist << " with route:\n";
          //          print_route(src, new_route);
          
          next->distance = next_dist;
          next->rt_est = new_route;
        }
        
      }
      
      // next figure out which node to look at next
      if(unvisited.empty()) {
        current = NULL;
      }else {
        list <ocin_prtr_stats_node *>::iterator temp;
        temp = min_element(unvisited.begin(), unvisited.end(), comp_node);
        current = *temp;
      }
    }
    last_cycle = ocin_cycle_count;
    last_src = src;
  }

#ifdef DEBUG
  {
    stringstream tmp;
    tmp << ocin_cycle_count << ": Route pid "<< msg->pid <<" from "<< src << " to " << dest<< ":\n";
    ocin_name_debug(name,tmp.str());
    print_route(src, nodes[dest]->rt_est);
  }
#endif
  // print();
  // cout << "\n\n";

  // now we can pull out the desired path from the dest node
    
  msg->preroutes = nodes[dest]->rt_est;
  msg->preroutes.push_back(edge_count); // push on the
                                                    // output port on
                                                    // the end...
  return;
  
}

void ocin_prtr_stats_str::print_route(string src, deque <int> route) {
  deque <int>::iterator rt_it;
  ocin_prtr_stats_node * current = nodes[src];
  ocin_prtr_stats_node * next = NULL;
#ifdef DEBUG
  {
    stringstream tmp;

    for (rt_it = route.begin(); rt_it != route.end(); rt_it++) {
      if (*rt_it == 4) {
        tmp << current->name << " to output port\n";
    } else {
        next = current->edges[*rt_it];
        tmp << current->name << " to "<< next->name <<" : "<< current->weights[*rt_it] <<endl;
        current = next;
      }
    }

    ocin_name_debug(name,tmp.str());
  }
#endif
}
  



// just quickly iterate over the prerouter stats structures, the work
// gets done in the nodes themselves.
void ocin_prtr_stats_str::update() {
  map<string, ocin_prtr_stats_node *>::iterator nid_it; 
  
  for (nid_it=nodes.begin(); nid_it != nodes.end(); nid_it++) {
    ocin_prtr_stats_node * node = nid_it->second;

    node->update();
  }
  //print();
}

  
void ocin_prtr_stats_str::reset() {
  map<string, ocin_prtr_stats_node *>::iterator nid_it; 
 
  unvisited.clear();
  
  for (nid_it=nodes.begin(); nid_it != nodes.end(); nid_it++) {
    ocin_prtr_stats_node * node = nid_it->second;
    
    node->reset();
    unvisited.push_back(node);
  }
}
 
