#include "Person.h"
#include "Resource.h"
#include "Util.h"
#include "Debug.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>

#if USE_QT
#include "QtVis/world_show.h"
#endif

using namespace std;

Population pop; // the global pop

int Person::person_count = -1;

Person::Person() {
  age=0;
  person_count++;
  identifier = person_count;
  type = 'A';
  expiry_age = 500;  // will not live beyod this age could die earlier

  curiosity = 60.0;
  homeByTime = 20;
  prevAction = START;
  energyExploreAbove = 10;
  
  init_energy = 50;
  current_energy = init_energy;
  
  max_energy = 70;
  sleepEnergyLoss = 15;
  moveCost = 2; //fiddle
  max_daily_eat = 35; //fiddle
  
  eaten_today = 0;
  repro_age_start = 200;
  repro_age_end = 450;
  num_offspring = 0;
  fam_plan.planned_offspring = -1;

  homeByTime = 20;
  atResource = false;
  isHome = false;
  hasBeenEating = false;
  isHeadingHome = false;

  eating_patch = NULL;

  loc = NULL;
  
  route_index = 0;
  home_loc = NULL;

  mind.who = this;
  
  homeTime = 0;
  clear_places_eaten();
  clear_places_explored();
}

void Person::show_defaults(ostream& o) {
  o << "DEFAULTS: ";
  o << "expiry_age:" << expiry_age; 
  o << " init_energy:" << init_energy;
  o << " max_energy:" << max_energy;
  o << " sleep loss:" << sleepEnergyLoss;
  o << " move cost:" << moveCost;
  o << " max_daily_eat:" << max_daily_eat;
  o << " repro_age_start:" << repro_age_start;
  o << " repro_age_end:" << repro_age_end;
  o << endl;
}

void Person::update_places_explored(LocNode* l) {
  if(mind.internalWorld.getNode(l->x, l->y)->type == UNKNOWN)
  {
    mind.internalWorld.getNode(l->x, l->y)->type = l->type;
    mind.numUnknown = mind.numUnknown - 1;
    num_places_explored = num_places_explored + 1;

    //cout << "Explored: " << l->x << " " << l->y << " is " << l->type << endl;

    if(l->type == RESOURCE)
      mind.addNewResToMind(l);
  }
}

void Person::show_route() {
  for(size_t i = 0; i < route.size(); i++) {
    cout << route[i]->tostring();
    if(i != (route.size()-1)) { cout << " -- "; }
  }
  cout << endl;
}

string Person::route_tostring() {
  string s = "";
 for(size_t i = 0; i < route.size(); i++) {
   s += route[i]->tostring();
    if(i != (route.size()-1)) { s += " -- "; }
  }
 return s;
}

//Way to complicated to explain the entire decision tree here but
//Person will decide on their next action depending mostly on:
//Their previous action + the time before they have to be home
ActionPtr Person::getNextAction(bool failedEat)
{
  //Set prev equal to the last actions kind, if the last action was null (meaning start of day, its set to START)
  ActionKind prev = prevAction;

  if(prev == HOMEREST)
  {
    if(loc->type != HAB_ZONE)
      cout << "Warning: somehow resting not at home" << endl;

    HomeAction *next = new HomeAction;
    next->p = this;
    return next;
  }
  if(prev == ROUTE && !failedEat)
  {
    //If in middle of a prev route, continue
    if(route_index < route.size())
    {
      RouteAction *next = new RouteAction;
      next->p = this;
      next->route_index = route_index;
      return next;
    }

    //reset route and routeindex
    route.clear();
    route_index = -1;

    //routed home, time to rest
    if(loc->type == HAB_ZONE)
    {
      HomeAction *next = new HomeAction;
      next->p = this;
      return next;
    }
    //routed to resource, try to eat
    else if(loc->type == RESOURCE)
    {
      loc->resourceObject->numHeading = loc->resourceObject->numHeading - 1;

      EatAction *next = new EatAction;
      next->p = this;
      return next;
    }
    cout << "Warning: Did not route to home or resource." << endl;
  }
  else if(prev == EAT && !failedEat)
  {
    //future adri, dont need to check if their at home already, if they were eating they can't be
    //if no time left, go home
    vector<LocNode*> pathHome = mind.internalWorld.findPath(loc, home_loc);
    int timeHome = pathHome.size();

    if((homeByTime - currentTic) <= timeHome)
    {
      isHeadingHome = true;
      route = pathHome;
      route_index = 0;
      
      RouteAction *next = new RouteAction;
      next->p = this;
      next->route_index = route_index;
      return next;
    }
    //else if not full try to eat again
    else if(!hasMaxEnergy())
    {
      EatAction *next = new EatAction;
      next->p = this;
      return next;
    }
    //explore if energy after going home right now is above a certain bounday
    //plus random chance with curiosity
    //plus is there anything to explore
    else if(
      !mind.internalWorld.findPathClosestUnexplored(loc).empty() &&
      current_energy - (timeHome * moveCost) - sleepEnergyLoss > energyExploreAbove &&
      (1+ (rand() % 100)) < curiosity)
    {
      ExploreAction *next = new ExploreAction;
      next->p = this;
      return next;
    }
    //else head home
    else
    {
      isHeadingHome = true;
      route = pathHome;
      route_index = 0;
      
      RouteAction *next = new RouteAction;
      next->p = this;
      next->route_index = route_index;
      return next;
    }
  }

  //if no time left, go home
  vector<LocNode*> pathHome = mind.internalWorld.findPath(loc, home_loc);
  int timeHome = pathHome.size();
  if(timeHome == 0) 
  {
    timeHome = timeHome + 1; //needed so people don't leave home at last tic
  }

  if((homeByTime - currentTic) <= timeHome)
  {
    //if already somehow home, then rest
    if(isHome)
    {
      if(loc->type != HAB_ZONE)
        cout << "Warning: somehow resting not at home" << endl;

      HomeAction *next = new HomeAction;
      next->p = this;
      return next;
    }
    
    isHeadingHome = true;
    route = pathHome;
    route_index = 0;
    
    RouteAction *next = new RouteAction;
    next->p = this;
    next->route_index = route_index;
    return next;
  }

  if(failedEat)
  {
    if(loc->type != RESOURCE)
      cout << "Warning: tried to eat at non resource somehow" << endl;
    
    //if not too many people waiting, wait
    //can use resource number not mind numbers as person is physically there
    if(loc->resourceObject->numWaiters < loc->resourceObject->getNumViablePatches())
    {
      WaitAction *next = new WaitAction;
      next->p = this;
      debug_record << identifier << " (retry) decided to " << WAIT << endl; 
      return next;
    }
  }

  //Finally either route to a resource or explore
  //Route to a random (viable) resource if not full
  //Explore if not
  if(!mind.knownResources.empty() && eaten_today < max_daily_eat)
  {
    //Check if currently on a resource, try to eat if haven't failed already
    if(loc->type == RESOURCE && !failedEat)
    {
      EatAction *next = new EatAction;
      next->p = this;
      return next;
    }

    bool wasSet = setResourceRoute();

    if(wasSet)
    {
      RouteAction *next = new RouteAction;
      next->p = this;
      next->route_index = route_index;

      if(failedEat)
        debug_record << identifier << " (retry) decided to " << ROUTE << endl; 
      return next;
    }
  }

  //If have been exploring, while knowing of a resource then either keep exploring or go home
  if(prev == EXPLORE && !mind.knownResources.empty())
  {
    //Should keep exploring?
    vector<LocNode*> pathHome = mind.internalWorld.findPath(loc, home_loc);
    int tHome = pathHome.size();
    if(
      !mind.internalWorld.findPathClosestUnexplored(loc).empty() &&
      current_energy - (tHome * moveCost) - sleepEnergyLoss > energyExploreAbove &&
      (1+ (rand() % 100)) < curiosity)
    {
      ExploreAction *next = new ExploreAction;
      next->p = this;
      return next;
    }

    //if already somehow home, then rest
    if(isHome)
    {
      if(loc->type != HAB_ZONE)
        cout << "Warning: somehow resting not at home" << endl;

      HomeAction *next = new HomeAction;
      next->p = this;
      return next;
    }

    //Head home if not
    isHeadingHome = true;
    route = pathHome;
    route_index = 0;
    
    RouteAction *next = new RouteAction;
    next->p = this;
    next->route_index = route_index;
    return next;
  }

  //Finally just explore
  ExploreAction *next = new ExploreAction;
  next->p = this;

  if(failedEat)
    debug_record << identifier << " decided to " << EXPLORE << endl; 
  return next;
}

//Tries to find a viable known resource to start a route too
//returns true if one is found and route is set
//false otherwise
bool Person::setResourceRoute()
{
  //remove non viable resources
  vector<LocNode*> viable;
  for(int i = 0; i < mind.knownResources.size(); i++)
  {
    vector<LocNode*> potential = mind.internalWorld.findPath(loc, mind.knownResources[i]);
    vector<LocNode*> backHome = mind.internalWorld.findPath(mind.knownResources[i], home_loc);
    
    //viable  if possible to get there in time and
    // if not current location and
    // if not still zero from wipeout and
    // 50% if still not normal from wipeout
    //on tic 0 (when their all at home) it also checks if there aren't too many people there/going there already 
    //it doesnt make sense for people to know that information afterwards afterwards
    if(homeByTime - currentTic > potential.size() + backHome.size() && 
      !mind.knownResources[i]->equals(loc) &&
      !mind.resInfo[i]->isWipeout)
    {
      if(mind.resInfo[i]->isNotNormal)
      {
        if(rand() % 100 > 50)
          continue;
      }
      if(currentTic == 0)
      {
        if(mind.knownResources[i]->resourceObject->getNumPersonsInterestedInResource() < mind.knownResources[i]->resourceObject->resources.size())
          viable.push_back(mind.knownResources[i]);
      }
      else if(mind.resInfo[i]->isPlenty)
      {
        //add in twice if the resource is in plenty mode
        //to make it more likely to be picked
        viable.push_back(mind.knownResources[i]);
        viable.push_back(mind.knownResources[i]);
      }
      else
      {
        viable.push_back(mind.knownResources[i]);
      }
    }
  }

  if(viable.empty())
    return false;
  
  //Pick randomly
  int randIndex = rand() % viable.size();
  route = mind.internalWorld.findPath(loc, viable[randIndex]);;
  route_index = 0;

  viable[randIndex]->resourceObject->numHeading = viable[randIndex]->resourceObject->numHeading + 1;
  return true;
}

// amout returned as 'gained' can assume
//  -- will not cause current_energy to grow too large if added
//  -- will not be more than max_daily_eat
// NB: this is not called directly but via eat_from_dry_run(..) passing a copy
// of a real crop patch
//User eats one berry
int Person::eat_from(CropPatch& c){
  #if DEBUG
  db(identifier); db(type); db(" frm "); db(c.name); db(c.sym); db("\n");
  #endif

  if(max_daily_eat > eaten_today && max_energy > current_energy && c.get_total() > 0)
  {
    //Get smaller of the two leftovers
    int smallerMax;
    if(max_daily_eat - eaten_today < max_energy - current_energy)
      smallerMax = max_daily_eat - eaten_today;
    else
      smallerMax = max_energy - current_energy;

    //return smaller gain if gain is bigger than the smaller leftover
    //aka dont overfill
    if(smallerMax < c.energy_conv)
    {
      return smallerMax;
    }
    return c.energy_conv;
  }

  return 0;
}

// calc what would happen in call to eat_frm(c,..) but make no updates to c
int Person::eat_from_dry_run(CropPatch& c) {
  CropPatch c_cpy;
  c_cpy = c;
  return eat_from(c_cpy);
}

bool Person::hasMaxEnergy() {
  if((eaten_today >= max_daily_eat) ||  (current_energy >= max_energy)) {
    return true;
  }
  else {
    return false;
  }
}

string Person::toid() {
  string s = "";
  s += to_string(identifier);
  s += type;
  return s;
}

void Person::show(){
  db("person: "); db(toid()); //db(identifier); db(type);
  db(" age: "); db(age);
  db(" rep: "); (num_offspring > 0 ? db(1) : db(0)); 
  db(" Esleep: "); db(sleepEnergyLoss);
  db(" mxEn: "); db(max_energy);
  db(" mxEat: "); db(max_daily_eat);
  show_var_energy();
  show_home_time();
  show_num_places();
  //  cout << " Rep: " << have_reproduced;
}

void Person::show_var_energy() {
  db(" En:"); db(current_energy);
  db(" Eaten: "); db(eaten_today);
}

void Person::show_home_time() {
  db(" Hm:"); db(homeTime);
}

void Person::show_num_places() {
  db(" Ple:"); db(num_places_eaten);
  db(" Plx:"); db(num_places_explored);

}

/* need just for stats */
void Person::update_places_eaten(ResPtr r) {
  for(int i = 0; i < resEatenAt.size(); i++)
  {
    if(resEatenAt[i]->equals(r))
      return;
  }
  resEatenAt.push_back(r);
  num_places_eaten++;
  return;
}

/* need just for stats */
void Person::clear_places_eaten() {
  resEatenAt.clear();
  num_places_eaten = 0;
}

/* need just for stats */
//doesnt reset each day, grows larger with time
void Person::clear_places_explored() {
  num_places_explored = 0;
}

void FamilyPlan::set_plan(int age, int who, int maxReproAge) {
  whose_plan = who;
  choose_planned_offspring();
  birth_age_index = 0;

  if(planned_offspring > 0) 
  {
    choose_birth_ages(age, maxReproAge);
    next_birth_age = birth_ages[birth_age_index];
  }
}

//Decide the ages a person will have a child
//The years they have left are split into planned offspring number of equal sized bands 
//randomly decide a number 
void FamilyPlan::choose_birth_ages(int age, int maxReproAge)
{
  int bandSize = (maxReproAge - age) / planned_offspring;

  for(int i = 0; i < planned_offspring; i++)
  {
    int randInt = gsl_rng_uniform_int(r_global, bandSize);
    //cout << "Rand: " << randInt << endl;
    int birthAge = randInt + (age + (bandSize * i));

    birth_ages.push_back(birthAge);
  }
}

void FamilyPlan::choose_planned_offspring() {
  const double p[] = { 0.05, 0.5, 0.45}; // expect 1.4 offspring
  //const double p[] = { 0.00, 0.05, 0.05, 0.99}; // expect 1.4 offspring

  size_t K = 3;
  //size_t K = 4;
  gsl_ran_discrete_t *init;
  init = gsl_ran_discrete_preproc(K, p);

  
  planned_offspring = gsl_ran_discrete(r_global, init);

  //cout << planned_offspring << endl;
}

FamilyPlan::FamilyPlan(){}

void FamilyPlan::show() {
  db("person "); db(whose_plan); db(" ");
  db("pl:"); db(planned_offspring);
  if(planned_offspring > 0) {
    db(" fst:"); db(next_birth_age);
  }
  if(planned_offspring > 1) {
    db(" nxt(");
    for(size_t i = 1; i < birth_ages.size(); i++) {
      db(birth_ages[i]); db(" ");
    }
    db(")");
  }
  if(planned_offspring > 0) {
    db(" next_ba:"); db(next_birth_age);
  }
  db("\n");
}


Population::Population() {}

Population::Population(string name, int size){ 

  id = name;
  hbt = 20;
  max_places_explored = 0;
  for(int i=0; i < size; i++) {
    

    Person *p = new Person;

    p->age = ((float)i/size) * 500; // unif distrib over ages ?
    if(p->age > 350) { p->num_offspring = 1; }
    p->homeByTime = hbt;
    population.push_back(p);

  }
  currentTic = 0;
}

void Population::zero_eaten_today() {
  vector<Person *>::iterator p;
  p = population.begin();
  while(p != population.end()) {
    (*p)->eaten_today = 0; 
    p++;
  }
}

void Population::resetDayBools(int date) {
  vector<Person *>::iterator p;
  p = population.begin();
  while(p != population.end()) {
    (*p)->isHeadingHome = false;
    (*p)->hasBeenEating = false;
    (*p)->isHome = false;
    (*p)->prevAction = START;
    (*p)->mind.dailyBoolUpdate(date);
    p++;
  }
}

void Population::updatePeopleTic(int tic)
{
  vector<Person *>::iterator p;
  p = population.begin();
  while(p != population.end()) {
    (*p)->currentTic = tic; 

    if((*p)->eating_patch != NULL)
    {
      (*p)->eating_patch->being_eaten = false;
      (*p)->eating_patch->eater = NULL;
      (*p)->eating_patch = NULL;
    }

    p++;
  }

  for(int i = 0; i < all_res.size(); i++)
  {
    all_res[i]->numWaiters = 0;
  }
}

void sanity_check_area_gains(vector<PerPtr> &population);

// top-level 'update' function which calls series of further update_... functions
// to eg. let die those too old, those with not enough energy
//        expend energy and try to eat
//        add new population members
// NOTE 15/8/2019: given this a date parameter this to allow the Resource expiry info
// which is expressed as absolute time to be transformed to 'count-down' times for Persons
bool Population::update(int date){
  //db_level = 0;
  bool updated = true;
 
  int deaths_age = 0;
  int deaths_starve = 0;
  int deaths_strand = 0;

  //reset peoples daily bools
  resetDayBools(date);
  
  #if DEBUG
  db("> age cull, expend nrg "); show(); db("\n");
  #endif
  
  if(population.size() == 0) { // extinction
    r_line.BIRTHS = 0;
    r_line.POP = 0;
    r_line.A_EN = 0;
    r_line.A_EATEN = 0;
    r_line.MAX_NUM_PLACES_EATEN = 0;
    // no other person related updates are possible so return
    return true;
  }

  // from here assume non-zero population and further updates to calculate

  /****************************************************************/
  /* execute day's worth of moving about to find and consume food */
  /****************************************************************/
  //db_level = 0;
  for(int tic = 0; tic < hbt; tic++)
  {
    //cout << "Tic: " << tic << endl;
    debug_record << "TIC TIC TIC TIC TIC TIC" << endl;
    pop.currentTic = tic;
    updatePeopleTic(tic);
    update_by_action(date, tic);
  }
  debug_record << date << " TOC TOC TOC TOC TOC TOC" << endl;
  #if DEBUG
  db("------------------------\n");
  db("> feeding\n ");
  show(); db("\n");

  for(size_t i = 0; i < all_res.size(); i++) {
    db(all_res[i]->tostring());
    //all_res[i]->show_bands();
    //db_level = 0;
    //all_res[i]->show_bars();
    // for brief output
    all_res[i]->show_total();
    db("\n");
    //db_level = 1;
  }
  #endif

  /***************************************************************************************************/
  /* do knowledge update by having people talk to each other based on what they learned while eating */
  /***************************************************************************************************/
  //db_level = 0;
  //TODO

  /************************************************/
  /* do updates of population due to reproduction */
  /************************************************/
  int num_births = update_by_repro(); 
  r_line.BIRTHS = num_births;

  r_line.A_EN = get_mean_energy('A'); // includes new births in denom
  r_line.A_EATEN = get_mean_eaten('A'); // includes new births in denom

  //Check for new maxs (stats)
  for(size_t i=0; i < population.size(); i++)  
  {
    if(population[i]->num_places_eaten > max_places_eaten) 
      max_places_eaten = population[i]->num_places_eaten;
    if(population[i]->num_places_explored > max_places_explored)
	    max_places_explored = population[i]->num_places_explored;
  }

  /******************************************************/
  /* remove those too old, those who aren't home at night, those with not enough energy */
  /******************************************************/
  update_by_cull(deaths_age, deaths_starve, deaths_strand);
  r_line.DEATHS_AGE = deaths_age;
  r_line.DEATHS_STARVE = deaths_starve;
  r_line.DEATHS_STRANDED = deaths_strand;

  r_line.POP = get_total();

  return updated;
}


void Population::update_by_cull(int& deaths_age, int& deaths_starve, int& deaths_strand) { // age, expend energy, then cull, set nums to amounts culled

  vector<Person *>::iterator p;

  // let die those too old, those with not enough energy
  p = population.begin();
  while(p != population.end()) {
    //    cout << "considering "; p->show(); db("\n");
    (*p)->age = ((*p)->age + 1);
    (*p)->current_energy -= (*p)->sleepEnergyLoss;

    if((*p)->age >= (*p)->expiry_age) {
      #if DEBUG
      db((*p)->type); db(" DEATH (age)\n");
      #endif
      delete *p;
      p = population.erase(p);
      deaths_age++; 
    }
    else if((*p)->current_energy < 0) {
      #if DEBUG
      db((*p)->type); db(" DEATH (starve)\n");
      #endif
      write_starvation_stats_line(starvation_stats,*p);
      delete *p;
      p = population.erase(p);
      deaths_starve++; 
    }
    else if((*p)->loc->type != HAB_ZONE) {
      #if DEBUG
      db((*p)->type); db(" DEATH (strand)\n");
      #endif
      write_stranding_stats_line(stranding_stats,*p);
      delete *p;
      p = population.erase(p);
      deaths_strand++;
    }
    else {    
      p++; // update the iterator
    }

  }
  #if DEBUG
  db("finished culling\n");
  #endif

}

//Creates action list, one action from each person
//Runs through list, action by action handled in a proc function.
//Every person gets one action per tic, unless their first actions failed (failed eat)
//Then they can queue a different one
void Population::update_by_action(int date, int tic) {
  //Initialize action list
  ActionList actList;
  actList.init_from_population(pop.population);
  
  //Complete each action in list
  while(!actList.is_empty())
  {
    ActionPtr a = actList.get_first(); // = (t, p, x) [time, person, loc]
    actList.pop_first();

    if(a->kind == ROUTE) 
    { 
      RouteAction *arr_ptr;	
      arr_ptr = (RouteAction *)a;
      RouteAction_proc(arr_ptr,tic);
      delete a;
      continue;
    }

    else if(a->kind == EAT) 
    { 
      EatAction *arr_ptr;	
      arr_ptr = (EatAction *)a;
      EatAction_proc(arr_ptr,actList,tic,date);
      delete a;
      continue;
    }

    else if(a->kind == EXPLORE) 
    { 
      ExploreAction *arr_ptr;	
      arr_ptr = (ExploreAction *)a;
      ExploreAction_proc(arr_ptr,actList,tic);
      delete a;
      continue;
    }

    else if(a->kind == HOMEREST) 
    { 
      HomeAction *arr_ptr;	
      arr_ptr = (HomeAction *)a;
      HomeAction_proc(arr_ptr,tic);
      delete a;
      continue;
    }

    else if(a->kind == WAIT) 
    { 
      WaitAction *arr_ptr;	
      arr_ptr = (WaitAction *)a;
      WaitAction_proc(arr_ptr,tic);
      delete a;
      continue;
    }
    cout << "Warning: not a valid action" << endl;
  }

  r_line.MAX_NUM_PLACES_EATEN = max_places_eaten;
  r_line.MAX_NUM_PLACES_EXPLORED = max_places_explored;
}

void Population::RouteAction_proc(RouteAction *route_ptr, int tic)
{
  //Get person related to action and set it as their prev action
  PerPtr p;
  p = route_ptr->p;
  p->prevAction = ROUTE;
  p->hasBeenEating = false;

  //update persons prevloc and newlocs occupancy
  LocNode* innerLoc = p->route[route_ptr->route_index];
  p->loc->occupancy = p->loc->occupancy - 1;
  innerLoc->occupancy = innerLoc->occupancy + 1;

  //Update persons location
  int x = innerLoc->x;
  int y = innerLoc->y;
  p->loc = world.getNode(x,y);

  //Update persons route pointer
  p->route_index = p->route_index + 1;

  //Remove energy for moving
  p->current_energy = p->current_energy - 1;

  //Update persons loc bools
  if(p->loc->type == HAB_ZONE && p->route_index >= p->route.size())
    p->isHome = true;
  else
    p->isHome = false;

  if(p->loc->type == RESOURCE && p->route_index >= p->route.size())
    p->atResource = true;
  else p->atResource = false;
  return;
}

//Picks random unexplored direction or tries to go to closest unexplored location
void Population::ExploreAction_proc(ExploreAction *expl_ptr,ActionList& list, int tic)
{
  //Get person related to action and set it as their prev action
  PerPtr p;
  p = expl_ptr->p;
  p->prevAction = EXPLORE;
  p->hasBeenEating = false;

  //Check if should go home
  //Aka if moving in a direction away from home would kill the person
  //While also creating a list of neighbors that are unknown
  vector<LocNode*> potentials;
  vector<LocNode*> neighbors = p->mind.internalWorld.getNeighbors(p->loc);

  bool canGetHome = false;
  for(int i = 0; i < neighbors.size(); i++)
  {
    vector<LocNode*> pathHome = p->mind.internalWorld.findPath(neighbors[i], p->home_loc);
    int timeHome = pathHome.size();

    if(timeHome == 0 && !(neighbors[i]->equals(p->home_loc)))
      cout << "Warning: No route found." << endl;

    if((p->homeByTime - currentTic - 1) > timeHome) //-1 because it will be from the next tic
    {
      canGetHome = true;
      if(neighbors[i]->type == UNKNOWN)
      {
        potentials.push_back(neighbors[i]);
      }
    }
  }
  //if we can't get home from any neighbor we need to go home now.
  if(canGetHome == false)
  {
    if(p->loc->type == HAB_ZONE)
    {
      debug_record << p->identifier << " (retry) decided to " << HOMEREST << endl;

      HomeAction *retry = new HomeAction;
      retry->p = p;
      list.insert(retry);
      return;
    }

    debug_record << p->identifier << " (retry) decided to " << ROUTE << endl;

    p->isHeadingHome = true;
    p->route = p->mind.internalWorld.findPath(p->loc, p->home_loc);
    p->route_index = 0;
    
    RouteAction *retry = new RouteAction;
    retry->p = p;
    retry->route_index = p->route_index;

    list.insert(retry);
    return;
  }
  //We can get home for sure now
  //choose where to go, either immediete unexplored neighbor or moving towards closest unexplored
  LocNode* toGo;
  if(!potentials.empty())
  {
    int randIndex = rand() % potentials.size();
    toGo = potentials[randIndex];
  }
  else
  {
    vector<LocNode*> pathToClosestUnknown = p->mind.internalWorld.findPathClosestUnexplored(p->loc);
    toGo = pathToClosestUnknown[0];
  }

  int x = toGo->x;
  int y = toGo->y;

  //Update mind
  p->update_places_explored(world.getNode(x,y));

  //if togo is an obstacle update knowledge and requeue explore
  if(world.getNode(x,y)->type == OBSTACLE)
  {
    ExploreAction *retry = new ExploreAction;
    retry->p = p;
    list.insert(retry);
    return;
  }

  //update persons prevloc and newlocs occupancy
  p->loc->occupancy = p->loc->occupancy - 1;
  toGo->occupancy = toGo->occupancy + 1;

  //Update persons location
  p->loc = world.getNode(x,y);

  //Remove energy for moving
  p->current_energy = p->current_energy - p->moveCost;

  //update loc bools
  if(p->loc->type == RESOURCE)
  {
    p->atResource = true;
    p->isHome = false;
  }
  else if (p->loc->type == HAB_ZONE)
  {
    p->atResource = false;
    p->isHome = true;
  }
  else
  {
    p->atResource = false;
    p->isHome = false;
  }

  return;
}

//Does nothing but update prev, person just waits at home
void Population::HomeAction_proc(HomeAction *home_ptr, int tic)
{
  //Get person related to action and set it as their prev action
  PerPtr p;
  p = home_ptr->p;
  p->prevAction = HOMEREST;

  return;
}

//Does nothing but update prev, person is waiting at resource
void Population::WaitAction_proc(WaitAction *wait_ptr, int tic)
{
  //Get person related to action and set it as their prev action
  PerPtr p;
  p = wait_ptr->p;
  p->prevAction = WAIT;
  p->hasBeenEating = false;

  p->loc->resourceObject->numWaiters = p->loc->resourceObject->numWaiters + 1;
  return;
}

//Check if eating is possible, if so go ahead with gains of energy
//If not get person
void Population::EatAction_proc(EatAction *eat_ptr, ActionList& list, int &date, int tic)
{
  //Get person related to action and set it as their prev action
  PerPtr p;
  p = eat_ptr->p;

  //Update knowledge on that resource
  p->mind.updateInfoRes(p->loc);

  // get all patches at x (i) not empty (ii) not occupied (ie. being_eaten is set true there)
  int cropIndex = p->loc->resourceObject->get_available_patch();

  //not possible to eat, have person redecide
  if(cropIndex == -1) 
  {
    list.insert(p->getNextAction(true));
    return;
  } 
  //Else eat :)
  else
  {
    //Set prev action
    p->prevAction = EAT;
    p->hasBeenEating = true;

    // get a random available patch
    CropPatch& c = p->loc->resourceObject->resources[cropIndex];

    // set the patch as being eaten
    c.being_eaten = true;

    // set the eater of the patch
    c.eater = p;
    // set person to state of having a patch
    p->eating_patch = &(c);

    // for sake of later stats gathering
    p->update_places_eaten(p->loc->resourceObject);
	
    int gain;
      
    //
    gain = p->eat_from_dry_run(c); // calc but dont do update

    // update person's energy attributes from this
    p->eaten_today = p->eaten_today + gain;
    p->current_energy = p->current_energy + gain;
    // update patch	
    p->eating_patch->remove_units(1);

    p->resEatenAt.push_back(p->loc->resourceObject);
  }
}

void Population::collect_subtype(char type, vector<PerPtr>& sub_pop) {
  sub_pop.clear();
  for(size_t i=0; i < population.size(); i++) {
    PerPtr p = population[i];
    if(p->type == type) {
      sub_pop.push_back(p);
    }
  }
}

float Population::get_mean_eaten(char type) {
  vector<PerPtr> sub_pop;
  collect_subtype(type, sub_pop);
  float m_eaten = 0;
  for(size_t i=0; i < sub_pop.size(); i++) {
    m_eaten += sub_pop[i]->eaten_today;
  }

  if(sub_pop.size() > 0) {
    return m_eaten/sub_pop.size();
  }
  else {
    return 0.0;
  }
  
}

float Population::get_mean_energy(char type) {
  vector<PerPtr> sub_pop;
  collect_subtype(type, sub_pop);
  float m_e = 0;
  for(size_t i=0; i < sub_pop.size(); i++) {
    m_e += sub_pop[i]->current_energy;
  }

  if(sub_pop.size() > 0) {
    return m_e/sub_pop.size();
  }
  else {
    return 0.0;
  }
  
}

// add new population members
int Population::update_by_repro() 
{
  // NOTE: according to docs, when a vector internally does a reallocation
  // (which you see when val frm capacity() changes) *all* iterators can become
  // invalid ie. the iterators do not transparently update to reflect the hidden
  // storage changes
  // --> cant use iterators on resizing vectors 
  // code uses index not iterator

  int num_births = 0;

  for(size_t i=0; i < population.size(); i++) {
    Person* p = population[i];
    FamilyPlan& plan = population[i]->fam_plan;

    if(p->age < p->repro_age_start-1) 
    {
      continue;
    }
    else if (p->age == p->repro_age_start - 1 || (p->fam_plan.planned_offspring == -1 && p->age < p->repro_age_end)) 
    {
      plan.set_plan(p->age + 1,p->identifier, p->repro_age_end);
      #if DEBUG
      db("plan "); db(p->type); db(": "); plan.show();
      #endif
    }
    else if ((p->age >= p->repro_age_start) && (p->age <= p->repro_age_end)
	     && (plan.planned_offspring > 0) && (plan.next_birth_age == p->age)) 
    {
      p->num_offspring = p->num_offspring + 1;
      p->fam_plan.birth_age_index = plan.birth_age_index + 1;

      //check if that was the last child or not before setting next birth age
      if(plan.birth_age_index < plan.birth_ages.size())
        plan.next_birth_age = plan.birth_ages[plan.birth_age_index];

      #if DEBUG
      db(p->identifier); db(p->type); db(" repro(1)\n");
      #endif
      
      Person* child = new Person;

      // share characteristics with parent
      child->set_frm_parent(p);

      population.push_back(child);
      num_births++;
    }
  }

  all_home_loc[0]->occupancy = all_home_loc[0]->occupancy + num_births;
  return num_births;
}

void Person::set_frm_parent(Person *p) {
  // share characteristics with parent
  type = p->type;
  max_energy = p->max_energy;
  max_daily_eat = p->max_daily_eat;
  init_energy =  p->init_energy;

  sleepEnergyLoss = p->sleepEnergyLoss;
  moveCost = p->moveCost;
  homeByTime = p->homeByTime;
  curiosity = p->curiosity;
  
  current_energy = init_energy;
  home_loc = p->home_loc;
  loc = home_loc;
}

bool compare_person(Person *p1, Person *p2) {
  if(p1->type < p2->type) { return true; }
  else if(p1->type == p2->type && p1->age > p2->age) { return true; }
  else {return false;}

}

string Person::info_type_to_string() {
  string s = to_string(identifier);
  return s;
}

void Person::info_type_show(){
  string s = info_type_to_string();
  db(s);
}

void Population::show(){
  db("pop size: "); db((int)population.size()); db("\n");
  
  if(population.size() > 0) {
    vector<Person *> pop_cpy;
    pop_cpy = population;
    sort(pop_cpy.begin(),pop_cpy.end(),compare_person);

    for(size_t i=0; i < pop_cpy.size(); i++) {
      pop_cpy[i]->show(); db("\n");
    }

  }
  
  // for(int i=0; i < population.size(); i++) {
  //   if(population[i].type == 'A') {
  //     population[i].show(); db("\n");
  //   }
  // }

  // for(int i=0; i < population.size(); i++) {
  //   if(population[i].type == 'B') {
  //     population[i].show(); db("\n");
  //   }
  // }


}

int Population::get_total() {
  return population.size();

}

bool Population::compare_by_index(const int& p1_index, const int& p2_index) {
  return compare_person(population[p1_index], population[p2_index]);

}


