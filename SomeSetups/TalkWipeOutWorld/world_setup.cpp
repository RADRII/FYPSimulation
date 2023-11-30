#include "Resource.h"
#include "People.h"
#include "Util.h"
#include "Debug.h"
#include <cmath>
#include <iostream>

int areas_rep = 5;

bool setup_all_resources_and_locations();
bool link_all_locations(LocPtr n);

void set_up_population();

// this makes global all_home_loc contain pointers to new home locations
// and makes global all_res_entry_loc contains pointers to new res entry locations
// this makes the global all_res contains pointers to new Resources object
// the Resources creation itself makes the Location objects within each Resources object
// have to make entry locs first, with specific geometry, to pass to Resources contstructor
// and get the contained locations in their turn in the right places
bool setup_all_resources_and_locations() {

  /*************************/
  /* home locations        */
  /*************************/
  LocPtr h_ptr = NULL;
  h_ptr = new Location(HAB_ZONE);
  all_home_loc.push_back(h_ptr);
  h_ptr->x = -2;
  h_ptr->y = 3;
  h_ptr->id = "home";

  LocPtr n_ptr = NULL;
  n_ptr = new Location(NODE);
  n_ptr->x = 0;
  n_ptr->y = 3;
  n_ptr->id = "hub";

  
  /************************/
  /* res entries          */
  /************************/
  LocPtr entry = NULL;

  for(size_t i = 0; i < areas_rep; i++) {

      entry = new Location(RES_ENTRY);
      all_res_entry_loc.push_back(entry);
  }

  // dont have coords yet
  // should all be r from origin, spread round semi-circle
  // they are straight-line dist 2 part tho in this version thats not important
  
  double r, a;
  double pi = M_PI;
  r = 1/sin(pi/8);
  a = 1/(sqrt(2) * sin(pi/8));

  entry = all_res_entry_loc[0];  // N
  entry->x = 0;
  entry->y = (n_ptr->y) - r;

  entry = all_res_entry_loc[1]; // NE
  entry->x =  a;
  entry->y = (n_ptr->y) - a;

  entry = all_res_entry_loc[2]; // E
  entry->x = r;
  entry->y = (n_ptr->y) - 0;

  entry = all_res_entry_loc[3]; // SE
  entry->x = a;
  entry->y = (n_ptr->y) + a;

  entry = all_res_entry_loc[4]; // S
  entry->x = 0;
  entry->y = (n_ptr->y) + r;

  // // entry = all_res_entry_loc[5];  // NW
  // // entry->x =  -a;
  // // entry->y = (h_ptr->y) - a;

  // // entry = all_res_entry_loc[6]; // W
  // // entry->x = -r;
  // // entry->y = (h_ptr->y) - 0;

  // // entry = all_res_entry_loc[7]; // SW
  // // entry->x = -a;
  // // entry->y = (h_ptr->y) + a;

  /*******************************/
  /* resources from res  entries */
  /*******************************/

  //vector<string> res_names = {"one", "two", "three", "four", "five","six","seven","eight"};
  vector<string> res_names = {"one", "two", "three", "four", "five"};
  
  ResPtr res_ptr = NULL;
  LocPtr lp = NULL;

  setup_record << "**************\n";
  setup_record << "RESOURCE AREAS\n";
  setup_record << "**************\n";

  
  vector<int> yields;
  yields = { 100, 100, 100, 100, 100};

  
  vector<int> patch_reps;
  int top_patch_rep = 2;
  int decline = 0;

  for(size_t i = 0; i < areas_rep; i++) {
     int this_patch_rep;
     this_patch_rep = top_patch_rep - (i * decline);
     patch_reps.push_back(this_patch_rep);
  }
  
  int loc_rep;
  loc_rep = 10;

  float energy_conv;
  energy_conv = 0.25;
  
  for(size_t i = 0; i < areas_rep; i++) {

    res_ptr = new Resources(res_names[i],all_res_entry_loc[i],yields[i],energy_conv,patch_reps[i],loc_rep);

    all_res.push_back(res_ptr);
    // record association of between the area's locs and the area
    for(size_t l=0; l < res_ptr->locs.size(); l++) {
      lp = &(res_ptr->locs[l]);
      loc_to_res[lp] = res_ptr;
    }
    // record index of this Resource obj in all_res
    res_to_index[res_ptr] = i;
  }

  
  link_all_locations(n_ptr);


  return true;

}


// all location linking is also done, incl from initial locs in Resources
// objects back to relevant res entry location
// see LocKind enumeration
// the 'home' (HAB_ZONE) is linked to the 'hub' (NODE) : n_ptr
// the 'hub' is radially linked to the all the res entry entry locations (RES_ENTRY)
// the 'entry' location for a resource area links to its first 'patch' (PATCH) locations
// 'entry' locations not linked to each other

bool link_all_locations(LocPtr n_ptr) {

  LocPtr entry = NULL;
  // leave entries having no mutual links

  // set links between home and hub
  LocPtr h_ptr;
  h_ptr = all_home_loc[0];
  h_ptr->add_arc(NODE_L,n_ptr);
  n_ptr->add_arc(HAB_ZONE_L,h_ptr);

  // set links between hub and res entries
  for(size_t i = 0; i < areas_rep; i++) {
    entry = all_res_entry_loc[i];
    n_ptr->add_arc(RES_ENTRY_L,entry);
    entry->add_arc(NODE_L, n_ptr);
  }
  
  // don't have links between entries and (fst locs of) resources yet
  // so set those
  for(size_t i = 0; i < areas_rep; i++) {
    entry = all_res_entry_loc[i];
    entry->add_arc(RES_L, &(all_res[i]->locs[0]));
    all_res[i]->locs[0].add_arc(RES_ENTRY_L, entry);
  }

  setup_record << "********\n";
  setup_record << "GEOMETRY\n";
  setup_record << "********\n";

  h_ptr->show_links(setup_record);

  n_ptr->show_links(setup_record);

  
  for(size_t i = 0; i < areas_rep; i++) {

    entry = all_res_entry_loc[i];
    entry->show_links(setup_record);
  }

  for(size_t j= 0; j < areas_rep; j++) {

    all_res[j]->locs[0].show_links(setup_record);
  }

  

  return true;

}

// sets up the global pop variable

void set_up_population() {

  setup_record << "**********\n";
  setup_record << "POPULATION\n";
  setup_record << "**********\n";

  /******************************************************************/
  /* set here alternate to the default wait/rest  durations   */
  /* these will apply to *both* tribes via Person constr           */
  /* further tribe-specific customisations are done further below  */
  /* by altering Person:wait/rest on tribe members *after*   */
  /* construction (which perist via set_frm_parent)                */
  /******************************************************************/
  
  //EndWaitEvent::wait_duration_def = 2.0;
  //EndRestEvent::rest_duration_def = 2;



  string tribe_name;
  int tribe_size;

  tribe_name = "tribe1";
  //tribe_size = 3;
  //tribe_size = 9;
  tribe_size = 40;

  Population people(tribe_name,tribe_size);
  

  // get defaults from first person
  //people.population[0]->show_defaults(setup_record);

  setup_record << "have " << tribe_name;
  setup_record << " size " << tribe_size << " ";


  people.tribes.push_back('A');
  //people.egal_tribes.push_back('A');

  for(size_t i=0; i < people.population.size(); i++) {

    people.population[i]->type = people.tribes[0];
    

    people.population[i]->home_loc = all_home_loc[0];
    people.population[i]->at_home = true;
    people.population[i]->loc = people.population[i]->home_loc;

    people.population[i]->retains_info = true;
    people.population[i]->retains[WIPE] = true;
    people.population[i]->imparts_info = false;
    
    people.population[i]->todo_choice_method = "uniform";

    
  }

  if(tribe_size > 0) {
    setup_record << "CUSTOM: ";

    PerPtr p = people.population[0];
    setup_record << "type: " << p->type;

    if(people.egal_tribes.size()==1) { setup_record << " (egal) "; }

    setup_record << " " << p->info_type_to_string();

    setup_record << " hrate:" << p->hrate;
    setup_record << " todo_choice_method:" << p->todo_choice_method;

    if(p->has_todo_limit) { setup_record << " todo_limit: " << p->todo_limit; }
    setup_record << endl;
  }
  
  tribe_name = "tribe2";
  tribe_size = 40;
  //tribe_size = 3;
  //tribe_size = 9;
  setup_record << "have " << tribe_name;
  setup_record << " size " << tribe_size << " ";

  
  Population people_other(tribe_name,tribe_size);

  people_other.tribes.push_back('B');
  //people_other.egal_tribes.push_back('B');

  for(int i=0; i < people_other.population.size(); i++) {

    people_other.population[i]->type = people_other.tribes[0];
    people_other.population[i]->home_loc = all_home_loc[0];
    people_other.population[i]->at_home = true;
    people_other.population[i]->loc = people_other.population[i]->home_loc;

    people_other.population[i]->retains_info = true;
    people_other.population[i]->retains[WIPE] = true;
    people_other.population[i]->imparts_info = true;
    people_other.population[i]->imparts[WIPE] = true;

    people_other.population[i]->todo_choice_method = "uniform";

  }

  if(tribe_size > 0) {
    setup_record << "CUSTOM: ";
    PerPtr p = people_other.population[0];

    setup_record << "type:" << p->type;
    
    if(people_other.egal_tribes.size()==1) { setup_record << " (egal) "; }

    setup_record << " " << p->info_type_to_string();
    
    setup_record << " hrate:" << p->hrate;

    setup_record << " todo_choice_method:" << p->todo_choice_method;
    

    if(p->has_todo_limit) { setup_record << " todo_limit: " << p->todo_limit; }
    setup_record << endl;
  }

  pop = people;

  pop.add(people_other);
  
  pop.order_type = RANDOM;

}


