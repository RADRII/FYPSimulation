#include "Resource.h"
#include "Person.h"
#include "Util.h"
#include "Debug.h"
#include "Setups.h"
#include "Grid.h"
#include <cmath>
#include <iostream>

//Variables to fiddle with:
int numResources = 25;
int numObstacles = 5;
int gridSize = 7;

bool setup_all_resources_and_locations(Setups* setup);
void set_up_population(Setups* s);

LocGrid world = LocGrid(gridSize, false);
vector<LocNode*> all_home_loc;
vector<LocNode*> all_resource_loc;

// this makes global all_home_loc contain pointers to new home locations
// and makes global all_res_entry_loc contains pointers to new res entry locations
// this makes the global all_res contains pointers to new Resources object
// the Resources creation itself makes the Location objects within each Resources object
// have to make entry locs first, with specific geometry, to pass to Resources contstructor
// and get the contained locations in their turn in the right places
bool setup_all_resources_and_locations(Setups* setup) {
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

  int yield = 70;

  vector<int> patch_reps;
  int top_patch_rep = 2;
  int decline = 0;

  for(size_t i = 0; i < numResources; i++) {
     int this_patch_rep;
     this_patch_rep = top_patch_rep - (i * decline);
     patch_reps.push_back(this_patch_rep);
  }

  int energy_conv;
  energy_conv = 6; 
  
  for(int i = 0; i < numResources; i++) {

    ResPtr res_ptr = new Resources(i,all_resource_loc[i]->x,all_resource_loc[i]->y,yield,energy_conv,patch_reps[i]);
    res_ptr->p_wipeout = setup->percentW;
    res_ptr->p_plenty = setup->percentP;

    all_res.push_back(res_ptr);
    // record association of between the resorce and the nodeloc
    all_resource_loc[i]->resourceObject = res_ptr;
  }

  return true;

}

// sets up the global pop variable

void set_up_population(Setups* setup)
{
  setup_record << "**********\n";
  setup_record << "POPULATION\n";
  setup_record << "**********\n";

  //3 different types of people
  //A - does not communicate
  //B - Only communicates positive information
  //C - Communicates everything

  //Setup A
  string tribe_name;
  int tribe_size;

  tribe_name = "noComm";
  tribe_size = setup->numA; //fiddle

  Population people(tribe_name,tribe_size);
  
  // get defaults from first person
  if(tribe_size != 0)
  {
    people.population[0]->show_defaults(setup_record);
    setup_record << "have " << tribe_name;
    setup_record << " size " << tribe_size << " \n";
    people.tribes.push_back('A');
  }

  for(size_t i=0; i < people.population.size(); i++) {

    people.population[i]->type = people.tribes[0];
    
    people.population[i]->home_loc = all_home_loc[0];
    people.population[i]->loc = people.population[i]->home_loc;
    all_home_loc[0]->occupancy = all_home_loc[0]->occupancy + 1;

    //Tribe specific setups
    people.population[i]->willCommunicate = false;
    people.population[i]->energyExploreAbove = 20;
  }

  //Setup B
  tribe_name = "OnlyPos";
  tribe_size = setup->numB; //fiddle

  Population peopleB(tribe_name,tribe_size);

  if(tribe_size != 0)
  {
    setup_record << "have " << tribe_name;
    setup_record << " size " << tribe_size << " \n";
    peopleB.tribes.push_back('B');
  }


  for(size_t i=0; i < peopleB.population.size(); i++) {

    peopleB.population[i]->type = peopleB.tribes[0];
    peopleB.population[i]->home_loc = all_home_loc[0];
    peopleB.population[i]->loc = peopleB.population[i]->home_loc;
    all_home_loc[0]->occupancy = all_home_loc[0]->occupancy + 1;

    //Tribe specific setups
    peopleB.population[i]->willCommunicate = true;
    peopleB.population[i]->onlyPos = true;
    peopleB.population[i]->energyExploreAbove = 25;
    peopleB.population[i]->communicateAboveEnergy = 20;
  }

  //Setup C
  tribe_name = "CommAll";
  tribe_size = setup->numC; //fiddle

  Population peopleC(tribe_name,tribe_size);

  if(tribe_size > 0)
  {
    setup_record << "have " << tribe_name;
    setup_record << " size " << tribe_size << " \n";
    peopleC.tribes.push_back('C');
  }

  for(size_t i=0; i < peopleC.population.size(); i++) {

    peopleC.population[i]->type = peopleC.tribes[0];
    
    peopleC.population[i]->home_loc = all_home_loc[0];
    peopleC.population[i]->loc = peopleC.population[i]->home_loc;
    all_home_loc[0]->occupancy = all_home_loc[0]->occupancy + 1;

    //Tribe specific setups
    peopleC.population[i]->willCommunicate = true;
    peopleC.population[i]->onlyPos = true;
    peopleC.population[i]->energyExploreAbove = 25;
    peopleC.population[i]->communicateAboveEnergy = 20;
  }

  //Condense all tribes into one pop
  if(setup->numB != 0)
    people.add(peopleB);
  if(setup->numC != 0)
    people.add(peopleC);

  if(tribe_size > 0) {
    setup_record << "CUSTOM: ";

    setup_record << "type: " << 'A';
    setup_record << "type:" << 'B';
    setup_record << "type:" << 'C';

    setup_record << endl;
  }

  pop = people;
}


