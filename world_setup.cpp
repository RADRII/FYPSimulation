#include "Resource.h"
#include "People.h"
#include "Util.h"
#include "Debug.h"
#include "Grid.h"
#include <cmath>
#include <iostream>

//Variables to fiddle with:
int numResources = 25;
int numObstacles = 5;
int gridSize = 7;

bool setup_all_resources_and_locations();
void set_up_population();

LocGrid world = LocGrid(gridSize);
vector<LocNode*> all_home_loc;
vector<LocNode*> all_resource_loc;

// this makes global all_home_loc contain pointers to new home locations
// and makes global all_res_entry_loc contains pointers to new res entry locations
// this makes the global all_res contains pointers to new Resources object
// the Resources creation itself makes the Location objects within each Resources object
// have to make entry locs first, with specific geometry, to pass to Resources contstructor
// and get the contained locations in their turn in the right places
bool setup_all_resources_and_locations() {
  setup_record << "**************\n";
  setup_record << "GEOMETRY\n";
  setup_record << "**************\n";

  //Setup Empty Grid
  setup_record << "Grid size " << gridSize << endl;

  //Add Home
  world.getNode(gridSize/2, gridSize/2)->type = HAB_ZONE;
  all_home_loc.push_back(world.getNode(gridSize/2, gridSize/2));
  setup_record << "Home Location: X " << gridSize/2 << ", Y " << gridSize/2 << endl;

  //Add Obstacles Areas
  int success = 0;
  while(success < numObstacles)
  {
    int xr = rand() % gridSize;
    int yr = rand() % gridSize;

    if(world.getNode(xr, yr)->type == EMPTY)
    {
      world.getNode(xr, yr)->type = OBSTACLE;
      vector<LocNode*> all_obstacle_loc;
      all_obstacle_loc.push_back(world.getNode(xr, yr));
      setup_record << "Obstacle: X " << xr << ", Y " << yr << endl;
      success++;
    }
  }

  //Add Resources Areas
  success = 0;

  while(success < numResources)
  {
    int xr = rand() % gridSize;
    int yr = rand() % gridSize;

    if(world.getNode(xr, yr)->type == EMPTY)
    {
      world.getNode(xr, yr)->type = RESOURCE;
      all_resource_loc.push_back(world.getNode(xr, yr));
      setup_record << "Resource: X " << xr << ", Y " << yr << endl;
      success++;
    }
  }

  setup_record << "**************\n";
  setup_record << "RESOURCE AREAS\n";
  setup_record << "**************\n";

  int yield = 100;

  vector<int> patch_reps;
  int top_patch_rep = 2;
  int decline = 0;

  for(size_t i = 0; i < numResources; i++) {
     int this_patch_rep;
     this_patch_rep = top_patch_rep - (i * decline);
     patch_reps.push_back(this_patch_rep);
  }

  float energy_conv;
  energy_conv = 0.25; 
  
  for(size_t i = 0; i < numResources; i++) {

    ResPtr res_ptr = new Resources(i,all_resource_loc[i]->x,all_resource_loc[i]->y,yield,energy_conv,patch_reps[i]);

    all_res.push_back(res_ptr);
    // record association of between the resorce and the nodeloc
    all_resource_loc[i]->resourceObject = res_ptr;
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
  tribe_size = 10;
  //tribe_size = 9;

  Population people(tribe_name,tribe_size);
  

  // get defaults from first person
  people.population[0]->show_defaults(setup_record);

  setup_record << "have " << tribe_name;
  setup_record << " size " << tribe_size << " ";


  people.tribes.push_back('A');
  //people.egal_tribes.push_back('A');

  for(size_t i=0; i < people.population.size(); i++) {

    people.population[i]->type = people.tribes[0];
    

    people.population[i]->home_loc = all_home_loc[0];
    people.population[i]->at_home = true;
    people.population[i]->loc = people.population[i]->home_loc;

    
  }

  if(tribe_size > 0) {
    setup_record << "CUSTOM: ";

    PerPtr p = people.population[0];
    setup_record << "type: " << p->type;
    setup_record << "type:" << p->type;
    setup_record << " speed:" << p->speed;

    setup_record << endl;
  }
  
  tribe_name = "tribe2";
  tribe_size = 10;

  setup_record << "have " << tribe_name;
  setup_record << " size " << tribe_size << " ";

  
  Population people_other(tribe_name,tribe_size);

  people_other.tribes.push_back('B');
  //people_other.egal_tribes.push_back('B');

  // doing 'customisation' of these; makes these faster, calls them all of 'B' type
  for(int i=0; i < people_other.population.size(); i++) {

    people_other.population[i]->type = people_other.tribes[0];
    people_other.population[i]->home_loc = all_home_loc[0];
    people_other.population[i]->at_home = true;
    people_other.population[i]->loc = people_other.population[i]->home_loc;
    people_other.population[i]->speed = 2.0;
    // all else left to default
  }

  if(tribe_size > 0) {
    setup_record << "CUSTOM: ";
    PerPtr p = people_other.population[0];

    setup_record << "type: " << p->type;
    setup_record << " speed" << p->speed;
    setup_record << endl;

  }

  pop = people;

  pop.add(people_other);
  
  pop.order_type = RANDOM;

}


