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

  curiosity = 0.2;
  homeByTime = 20;
  
  init_energy = 5.0;
  current_energy = init_energy;
  
  max_energy = 7.0;
  sleepEnergyLoss = 2.0;
  moveCost = 0.1; //Todo fiddle with
  max_daily_eat = 5.0;
  hrate = 5; //Todo fiddle with
  
  eaten_today = 0.0;
  repro_age_start = 200;
  repro_age_end = 450;
  num_offspring = 0;
  fam_plan.planned_offspring = 0;
  speed = 1.0;

  homeByTime = 20;
  atResource = false;
  isHome = false;
  hasBeenEating = false;
  isHeadingHome = false;
  
  prevAction = NULL;
  knownResources;
  energyExploreAbove = 2.5;

  eating_patch = NULL;

  loc = NULL;
  
  route_index = 0;
  home_loc = NULL;

  mind.who = this;

  area_gains.clear_area_gains();
  
  homeTime = 0;
  clear_places_eaten();
  clear_places_explored();

  resEatenAt;
}

void Person::show_defaults(ostream& o) {
  o << "DEFAULTS: ";
  o << "expiry_age:" << expiry_age; 
  o << " init_energy:" << init_energy;
  o << " max_energy:" << max_energy;
  o << " sleep loss:" << sleepEnergyLoss;
  o << " move cost:" << moveCost;
  o << " max_daily_eat:" << max_daily_eat;
  o << " hrate:" << hrate;
  o << " repro_age_start:" << repro_age_start;
  o << " repro_age_end:" << repro_age_end;
  o << " speed:" << speed;
  o << endl;
}

//Closest resource to current location, which also lets the player get back home after.
int Person::closestViableResource() {
  int index = -1;
  int shortestRoute = 1000;
  for(int i = 0; i < knownResources.size(); i++)
  {
    vector<LocNode*> temp = mind.internalWorld.findPath(loc, knownResources[i]);
    if(!temp.empty())
    {
      if(temp.size() < shortestRoute)
      {
        int timeLeft = homeByTime - currentTic;
        vector<LocNode*> resToHome = mind.internalWorld.findPath(knownResources[i], home_loc);
        int timeToEat = timeLeft - (resToHome.size() + temp.size());
        //if((timeToEat * 0.5) > ((resToHome.size() + temp.size()) * moveCost)) ToDo edit 0.5 to be berry/bean ish cost
        if(timeToEat > 1)
        {
          shortestRoute = temp.size();
          index = i;
        }
      }
    }
  }
  return index;
}

bool Person::set_route(LocNode* fst, LocNode* lst) {
  route.clear();
  route_index = 0;
  
  vector<LocNode*> temp = mind.internalWorld.findPath(fst, lst);
  if(temp.empty())
  {
    route_index = -1;
    return false;
  }
  else
  {
    route = mind.internalWorld.findPath(fst, lst);
  }

  if(route.back()->type == HAB_ZONE)
    isHeadingHome = true;

  return true;
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

// amout returned as 'gained' can assume
//  -- will not cause current_energy to grow too large if added
//  -- will not be more than max_daily_eat
// NB: this is not called directly but via eat_from_dry_run(..) passing a copy
// of a real crop patch
//User eats one berry
float Person::eat_from(CropPatch& c, float& handling, int& units_frm_patch){
  #if DEBUG
  db(identifier); db(type); db(" frm "); db(c.pos.tostring()); db(c.sym); db("\n");
  #endif
  
  float gained = 0.0;
  if(max_daily_eat > eaten_today && max_energy > current_energy && c.get_total() > 1)
  {
    return c.energy_conv;
  }

  return 0;
}

// calc what would happen in call to eat_frm(c,..) but make no updates to c
float Person::eat_from_dry_run(CropPatch& c, float& handling, int& units_frm_patch) {
  CropPatch c_cpy;
  c_cpy = c;
  return eat_from(c_cpy, handling, units_frm_patch);
}

bool Person::hasMaxEnergy() {
  if((eaten_today == max_daily_eat) ||  (current_energy == max_energy)) {
    return true;
  }
  else {
    return false;
  }
}

// check if others are eating at the same location
bool Person::others_at_loc(vector<PerPtr>& others) {
  return loc->resourceObject->being_eaten_patches_at_location();
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
  area_gains.show_area_gains();
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
void Person::update_places_explored(LocNode* l) {
  if(mind.internalWorld.getNode(l->x, l->y)->type == UNKNOWN)
  {
    mind.internalWorld.getNode(l->x, l->y)->type = l->type;
    num_places_explored++;
  }
}

/* need just for stats */
//doesnt reset each day, grows larger with time
void Person::clear_places_explored() {
  num_places_explored = 0;
}

void FamilyPlan::set_plan(int age, int who) {
  whose_plan = who;
  choose_planned_offspring();

  if(planned_offspring > 0) {
    choose_wait_first();
    if(planned_offspring > 1) {
      wait_next.resize(planned_offspring - 1);
      choose_wait_next();
    }
    next_birth_age = age + wait_first;

  }
}

void FamilyPlan::choose_wait_first() {
  // wait_first = gsl_rng_uniform_int(r_global, 10);
  wait_first = gsl_rng_uniform_int(r_global, 123);
  //    wait_first = gsl_rng_uniform_int(r_global, 50);

}

void FamilyPlan::choose_wait_next() {
  for(int i=0; i < planned_offspring-1; i++) {
    // wait_next[i] = gsl_rng_uniform_int(r_global, 10);
    wait_next[i] = gsl_rng_uniform_int(r_global, 123);
    // wait_next[i] = gsl_rng_uniform_int(r_global, 50);
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

  //cout << poss_offspring << endl;
}

FamilyPlan::FamilyPlan(){}

FamilyPlan::FamilyPlan(int age, int whose, int pl, int first, vector<int> nxt) {
  whose_plan = whose;
  planned_offspring = pl;

  wait_first = first;
  wait_next = nxt; 
    
  next_birth_age = age + wait_first;

}

void FamilyPlan::show() {
  db("person "); db(whose_plan); db(" ");
  db("pl:"); db(planned_offspring);
  if(planned_offspring > 0) {
    db(" fst:"); db(wait_first);
  }
  if(planned_offspring > 1) {
    db(" nxt(");
    for(size_t i = 0; i < wait_next.size(); i++) {
      db(wait_next[i]); db(" ");
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
  for(int i=0; i < size; i++) {
    

    Person *p = new Person;

    p->age = ((float)i/size) * 500; // unif distrib over ages ?
    if(p->age > 350) { p->num_offspring = 1; }
    population.push_back(p);

  }

  int homeByTime = 20;
  currentTic = 0;
}

void Population::zero_eaten_today() {
  vector<Person *>::iterator p;
  p = population.begin();
  while(p != population.end()) {
    (*p)->eaten_today = 0.0; 
    p++;
  }
}

void Population::resetDayBools() {
  vector<Person *>::iterator p;
  p = population.begin();
  while(p != population.end()) {
    (*p)->isHeadingHome = false;
    (*p)->hasBeenEating = false;
    (*p)->isHome = false;
    (*p)->prevAction = nullptr;

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

  /******************************************************/
  /* remove those too old, those who aren't home at night, those with not enough energy */
  /******************************************************/
  update_by_cull(deaths_age, deaths_starve, deaths_strand);
  r_line.DEATHS_AGE = deaths_age;
  r_line.DEATHS_STARVE = deaths_starve;
  r_line.DEATHS_STARVE = deaths_strand;
  
  #if DEBUG
  db("> age cull, expend nrg "); show(); db("\n");
  #endif
  
  if(population.size() == 0) { // extinction
    r_line.BIRTHS = 0;
    r_line.POP = 0;
    r_line.TYPEA = 0;
    r_line.TYPEB = 0;
    r_line.A_EN = 0;
    r_line.B_EN = 0;
    r_line.A_EATEN = 0;
    r_line.B_EATEN = 0;
    r_line.MAX_NUM_PLACES_EATEN = 0;
    r_line.MAX_NUM_PLACES_EXPLORED = 0;
    // no other person related updates are possible so return
    return true;
  }

  // from here assume non-zero population and further updates to calculate

  /****************************************************************/
  /* execute day's worth of moving about to find and consume food */
  /****************************************************************/
  //db_level = 0;
  for(int tic = 0; tic < homeByTime; tic++)
  {
    pop.currentTic = tic;
    updatePeopleTic(tic);
    update_by_action(date, tic);
  }

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

  /************************************************/
  /* do updates of population due to reproduction */
  /************************************************/
  int num_births = 0;
  update_by_repro(num_births); 
  r_line.BIRTHS = num_births;

  r_line.POP = get_total();

  r_line.A_EN = get_mean_energy('A'); // includes new births in denom
  r_line.B_EN = get_mean_energy('B'); // ditto
  r_line.A_EATEN = get_mean_eaten('A'); // includes new births in denom
  r_line.B_EATEN = get_mean_eaten('B'); // ditto
  
  /***************************************************************************************************/
  /* do knowledge update by having people talk to each other based on what they learned while eating */
  /***************************************************************************************************/
  //db_level = 0;
  //TODO

  //db_level = 1;
  
  int numA = 0;
  int numB = 0;
  for(size_t i=0; i < population.size(); i++)  {
    if(population[i]->type == 'A') {
      numA++;
    }
    else {
      numB++;
    }
  }

  r_line.TYPEA = numA;
  r_line.TYPEB = numB;

  //reset peoples daily bools
  resetDayBools();
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
    else if((*p)->current_energy < 0.0) {
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

    if(a->kind == EAT) 
    { 
      EatAction *arr_ptr;	
      arr_ptr = (EatAction *)a;
      EatAction_proc(arr_ptr,actList,tic,date);
      delete a;
      continue;
    }

    if(a->kind == EXPLORE) 
    { 
      ExploreAction *arr_ptr;	
      arr_ptr = (ExploreAction *)a;
      ExploreAction_proc(arr_ptr,actList,tic);
      delete a;
      continue;
    }

    if(a->kind == HOMEREST) 
    { 
      HomeAction *arr_ptr;	
      arr_ptr = (HomeAction *)a;
      HomeAction_proc(arr_ptr,tic);
      delete a;
      continue;
    }

    if(a->kind == WAIT) 
    { 
      WaitAction *arr_ptr;	
      arr_ptr = (WaitAction *)a;
      WaitAction_proc(arr_ptr,tic);
      delete a;
      continue;
    }
  }

  r_line.MAX_NUM_PLACES_EATEN = max_places_eaten;
  r_line.MAX_NUM_PLACES_EXPLORED = max_places_explored;
}

void Population::RouteAction_proc(RouteAction *route_ptr, int tic)
{
  //Get person related to action and set it as their prev action
  PerPtr p;
  p = route_ptr->p;
  p->prevAction = route_ptr;
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
  p->route_index = p->route_index++;

  //Update persons loc bools
  if(p->loc->type == HAB_ZONE && route_ptr->route_index + 1 == p->route.size())
  {
    p->isHome = true;
    if(p->num_places_eaten > max_places_eaten) 
      max_places_eaten = p->num_places_eaten;
    if(p->num_places_explored > max_places_explored)
	    max_places_explored = p->num_places_explored;
  }

  //Todo update energy

  if(p->loc->type == RESOURCE && route_ptr->route_index + 1 == p->route.size())
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
  p->prevAction = expl_ptr;
  p->hasBeenEating = false;

  //Get list of unexplored neigbors
  vector<LocNode*> potentials = p->mind.internalWorld.getUnexploredNeighbors(p->loc);

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
  //if togo is an obstacle update knowledge and requeue explore
  if(world.getNode(x,y)->type == OBSTACLE)
  {
    p->mind.internalWorld.getNode(x, y)->type = OBSTACLE;
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

  //Todo update energy

  //Update mind
  p->mind.internalWorld.getNode(x, y)->type = p->loc->type;

  //update loc bools
  if(p->loc->type == RESOURCE)
    p->atResource = true;
  else p->atResource = false;

  return;
}

//Does nothing but update prev, person just waits at home
void Population::HomeAction_proc(HomeAction *home_ptr, int tic)
{
  //Get person related to action and set it as their prev action
  PerPtr p;
  p = home_ptr->p;
  p->prevAction = home_ptr;

  return;
}

//Does nothing but update prev, person is waiting at resource
void Population::WaitAction_proc(WaitAction *wait_ptr, int tic)
{
  //Get person related to action and set it as their prev action
  PerPtr p;
  p = wait_ptr->p;
  p->prevAction = wait_ptr;
  p->hasBeenEating = false;

  return;
}

//Check if eating is possible, if so go ahead with gains of energy
//If not get person
void Population::EatAction_proc(EatAction *eat_ptr, ActionList& list, int &date, int tic)
{
  //Get person related to action and set it as their prev action
  PerPtr p;
  p = eat_ptr->p;

  // get all patches at x (i) not empty (ii) not occupied (ie. being_eaten is set true there)
  bool have_patches_here = false;
  vector<int> patches;
  have_patches_here = p->loc->resourceObject->patches_at_location(patches);

  //not possible to eat, have person redecide
  if(!have_patches_here) 
  {
    p->getNextAction(true);
    return;
  } 
  //Else eat :)
  else
  {
    //Set prev action
    p->prevAction = eat_ptr;
    p->hasBeenEating = true;

    // get a random one
    int patch_index = p->loc->resourceObject->choose_rand_patch(patches);
    CropPatch& c = p->loc->resourceObject->resources[patch_index];
    // set the patch as being eaten
    c.being_eaten = true;
    // set the eater of the patch
    c.eater = p;
    // set person to state of having a patch
    //p->at_a_patch = true;
    p->eating_patch = &(p->loc->resourceObject->resources[patch_index]);

    // TEMP: for sake of later stats gathering
    p->update_places_eaten(p->loc->resourceObject);
	
    //  make p eat max poss from patch, calculating handling time 'handled'
    float gain;
    float handled;
    int units_frm_patch;
      
    //
    gain = p->eat_from_dry_run(c,handled,units_frm_patch); // calc but dont do update

    // update person's energy attributes from this
    p->eaten_today += eat_ptr->gain;
    p->current_energy += eat_ptr->gain;
    // also update person's AreaGain info
    p->area_gains.increment_an_area_gain(p->loc->resourceObject, eat_ptr->gain);
    // update patch	
    p->eating_patch->remove_units(eat_ptr->units_frm_patch);
  } // end have_patches_here
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

void Population::aggregate_area_gains(char type,AreaGains &agg) {
  vector<PerPtr> sub_pop;
  collect_subtype(type, sub_pop);
  
  agg.clear_area_gains();
  agg.collective = true;
  
    /*****************************************/
    /* make running the sum over all in sub_pop */
    /*****************************************/
    map<ResPtr,AreaGain>::iterator agg_itr;

    for(size_t i=0; i < sub_pop.size(); i++) {
      PerPtr p = sub_pop[i];
      map<ResPtr,AreaGain>::const_iterator pag_itr;
      for(pag_itr = p->area_gains.area_gains.begin();pag_itr != p->area_gains.area_gains.end(); pag_itr++) {
	agg_itr = agg.area_gains.find(pag_itr->first);
	if(agg_itr == agg.area_gains.end()) {
          agg.set(pag_itr->first,pag_itr->second);
	  agg.area_gains[pag_itr->first].collective = true; 
	}
	else {
	  agg_itr->second.gain += pag_itr->second.gain;
	  agg_itr->second.time_in_area += pag_itr->second.time_in_area;
	  agg_itr->second.count += 1;
	}
      
      } // end loop over p's gains

      
    } // end loop over sub_pop

  /*********************************/
  /* calculate the gain_per_person */
  /*********************************/  
  for(agg_itr = agg.area_gains.begin(); agg_itr != agg.area_gains.end(); agg_itr++) {
    AreaGain& ag = agg_itr->second;
    if(ag.count != 0) {
      ag.gain_per_person = ag.gain/ag.count;
    }
    else {
      ag.gain_per_person = 0;
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

void Population::clear_all_area_gains() {
 PerPtr p;
  for(size_t i=0; i < population.size(); i++) {
    p = population[i];
    p->area_gains.clear_area_gains();
  }

}

void Population::update_by_repro(int& num_births) { // add new population members


  // NOTE: according to docs, when a vector internally does a reallocation
  // (which you see when val frm capacity() changes) *all* iterators can become
  // invalid ie. the iterators do not transparently update to reflect the hidden
  // storage changes
  // --> cant use iterators on resizing vectors 

  // code uses index not iterator


  for(size_t i=0; i < population.size(); i++) {

    Person* p = population[i];
    FamilyPlan& plan = population[i]->fam_plan;
    if(p->age < p->repro_age_start-1) {
    }
    else if (p->age == p->repro_age_start - 1) {
      plan.set_plan(p->age + 1,p->identifier);
      #if DEBUG
      db("plan "); db(p->type); db(": "); plan.show();
      #endif
    }
    else if ((p->age >= p->repro_age_start) && (p->age <= p->repro_age_end)
	     && (plan.planned_offspring > 0) && (plan.next_birth_age == p->age)) {

      p->num_offspring++;
      if(plan.planned_offspring > 1 && plan.planned_offspring > p->num_offspring ) {
        plan.next_birth_age += plan.wait_next[p->num_offspring - 1];
	plan.next_birth_age++; // so if wait_next is 0, this is the next day
      }

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


  // // below is the code for doing it with an iterator
  // // throws seg error due to iterator invalidation when vector grows
  // vector<Person>::iterator p;
  // p = population.begin(); // go to start again
  // while(p != population.end()) {
  //   if(!(p->have_reproduced) && (p->age >= p->repro_age_start) && (p->age <= p->repro_age_end)) {
  //     Person child;
  //     population.push_back(child);
  //       cout << "capacity after push back: " << population.capacity() << endl;
  //     p->have_reproduced = true;
  //   }
  //   p++;
  // }
  

}

void Person::set_frm_parent(Person *p) {
  // share characteristics with parent
  type = p->type;
  max_energy = p->max_energy;
  max_daily_eat = p->max_daily_eat;
  init_energy =  p->init_energy;
  speed = p->speed;

  sleepEnergyLoss = p->sleepEnergyLoss;
  moveCost = p->moveCost;
  homeByTime = p->homeByTime;
  curiosity = p->curiosity;
  
  current_energy = init_energy;
  hrate = p->hrate;
  home_loc = p->home_loc;
  loc = home_loc;
}

bool compare_person(Person *p1, Person *p2) {
  if(p1->type < p2->type) { return true; }
  else if(p1->type == p2->type && p1->age > p2->age) { return true; }
  else {return false;}

}

string Person::info_type_to_string() {
  string s = "";
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

