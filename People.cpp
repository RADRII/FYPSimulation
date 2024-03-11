#include "Resource.h"
#include "People.h"
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
  
  init_energy = 5.0;
  current_energy = init_energy;
  
  max_energy = 7.0;
  daily_use = 2.0; 
  //daily_use = 0.0; // could use to make eating redundant
  max_daily_eat = 3.5;
  hrate = 0.11;
  
  eaten_today = 0.0;
  //repro_age_start = 350;
  repro_age_start = 200;
  repro_age_end = 450;
  num_offspring = 0;
  fam_plan.planned_offspring = 0;
  speed = 2.0;

  at_a_resource = false;
  has_todo_limit = false;
  todo_limit = 0;

  has_home_time_limit = false;
  home_time_max = 0;
  
  //at_a_patch = false;

  eating_patch = NULL;

  
  loc = NULL;
  todo_index = NO_TODO;
  todo_sched.clear(); 

  todo_choice_method = "uniform";
  
  route_index = 0;
  home_loc = NULL;
  at_home = false;
  
  wait = EndWaitEvent::wait_duration_def;
  
  retains_info = false;
  retains[WIPE] = false;

  imparts_info = false;
  imparts[WIPE] = false;

  mind.who = this;

  area_gains.clear_area_gains();
  
  home_time = 0.0;
  clear_places_eaten();
  clear_places_explored();

}

void Person::show_defaults(ostream& o) {
  o << "DEFAULTS: ";
  o << "expiry_age:" << expiry_age; 
  o << " init_energy:" << init_energy;
  o << " max_energy:" << max_energy;
  o << " daily_use:" << daily_use;
  o << " max_daily_eat:" << max_daily_eat;
  o << " hrate:" << hrate;
  o << " repro_age_start:" << repro_age_start;
  o << " repro_age_end:" << repro_age_end;
  // fam_plan stuff?
  o << " speed:" << speed;
  o << " wait:" << EndWaitEvent::wait_duration_def;
    
  o << " retains_info:" << retains_info;
  if(retains_info) {
    o << " ";
    o << "wipe("   << retains[WIPE]   << ")";
  }
  o << " imparts_info:" << imparts_info;
  if(imparts_info) {
    o << " ";
    o << "wipe("   << imparts[WIPE]    << ")";
  }

  o << endl;

}

const size_t Person::NO_TODO = numeric_limits<size_t>::max();

// give a person their major sequence of locations they wish to get to
// primarily the resource entries. could also include the final home location
// after these but possibly should skip that as it is not really any kind of choice
/* if person has neg info about a Resoures area exclude its entry from the entries being considered */
/* randomly pick a first from those being considered, then add the rest */
bool Person::set_todo_sched() {

  todo_sched.clear();
  LocNode* start_loc = NULL;
  vector<LocNode*> to_consider;

  #if DEBUG
  //if(type == 'B') {
  db(toid()); db(" knowledge\n ");
  db(mind.tostring()); db("\n");
  //}
  #endif
  
  // set to_consider to include (entries to) only to wanted areas
  // defined unwanted as having wipeout info on it with till_non_zero > 0
  for(size_t i=0; i < all_res.size(); i++) {
    ResPtr r = all_res[i];
    //if(knows_about(r)) {
    if(retains[WIPE] && mind.has_wipeout_info_about(r)) {
      WipeoutInfo wi;
      for(size_t w=0; w < mind.wipeouts.size(); w++) {
        if(mind.wipeouts[w].res_to_ignore == r) {
          wi = mind.wipeouts[w];
        }
      }
      if(wi.till_non_zero <= 0) {
        to_consider.push_back(world.getNode(r->x, r->y));
      }
      else {
        #if DEBUG
        db(r->id); db(" excluded due to "); db(wi.tostring()); db("\n");
        #endif
      }
    }
    else {
      to_consider.push_back(world.getNode(r->x, r->y));
    }
  } // should have made a subset of the entry locations

  //Shuffle randomly to_consider into todo
  for(int i = 0; i < to_consider.size(); i++)
  {
    int rand_index = rand() % to_consider.size();
    todo_sched.push_back(to_consider[rand_index]);
    to_consider.erase(to_consider.begin()+rand_index);
  }

  // check if have too many
  if(has_todo_limit && todo_sched.size() > todo_limit) {
    todo_sched.resize(todo_limit);
  }
  
  todo_index = 0;
  return true;  
}

string Person::todo_sched_tostring() {

  string s = "";
  for(size_t i = 0; i < todo_sched.size(); i++) {
    s += todo_sched[i]->tostring();
    if(i != (todo_sched.size()-1)) { s += " > "; }
  }
  return s;

}

bool Person::get_nxt_frm_todo_sched(LocNode* res_entry_loc, size_t& nxt_todo) {
  if(todo_index != todo_sched.size()-1) {
    nxt_todo = todo_index+1;
    res_entry_loc = todo_sched[nxt_todo];
    return true;
  }
  else {
    return false;
  }
}

// use nature of last on route to determine if heading  home
bool Person::heading_home() {
  if(route.back()->type == HAB_ZONE) {
    return true;
  }
  else {
    return false;
  }
}

bool Person::set_route(LocNode* fst, LocNode* lst) {
  route.clear();
  route_index = 0;
  
  route = mind.internalWorld.findPath(fst, lst);

  return false;
}

void Person::set_route_loc() {
  loc = route[route_index];
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
// sets handling to time taken for units consumed in generating this gain
// sets units_frm_patch to num of units consumed in generating this gain
// NB: this is not called directly but via eat_from_dry_run(..) passing a copy
// of a real crop patch

float Person::eat_from(CropPatch& c, float& handling, int& units_frm_patch){ // consume food if possible from c
  #if DEBUG
  db(identifier); db(type); db(" frm "); db(c.pos.tostring()); db(c.sym); db("\n");
  #endif
  
  float gained = 0.0;
  handling = 0.0;
  units_frm_patch = 0;

  // how far from hitting max daily food consumption?
  float m_eat = max_daily_eat - eaten_today;

  // how far from hitting max energy?
  float m_en = max_energy - current_energy;

  // if m is the smallest of  these margins
  // m is the max that can still be eaten

  float m = ((m_en < m_eat) ? m_en : m_eat);

  // assume eat like this
  // if m = 3.5 and next band has 5 units (conv rate = 1)
  // will take 4 units, consuming half of 4th unit, so half is wasted
  // bef:  ooooo  
  // aft:  o
  //

  int m_int = (int) ( m / (c.energy_conv) ); 
  //float m_rem = fmod(m, c.energy_conv);
  float m_rem = m - (m_int * c.energy_conv);

  int units_avail;
  int units_sought;

  (m_rem > 0 ? units_sought = m_int + 1 : units_sought = m_int);

  units_avail = c.get_total();

  if(units_avail >= units_sought) { // eating will end in this patch
    units_frm_patch = units_sought;
    gained = m;
    handling = (m_int * hrate);
    // note this treatment of handling excluding poss partial unit
    // is carry over from prev vers
  }
  else {
    units_frm_patch = units_avail;
    gained = (units_frm_patch * c.energy_conv);
    handling = (units_frm_patch * hrate);
  }
  
  // so either current patch now exhausted
  // or have maxed out either current_energy or eaten_today
  
  //cout << "person " << identifier << ": ate" << gained << endl;
  //show_var_energy(); cout << endl;
  return gained;

}

// calc what would happen in call to eat_frm(c,..) but make no updates to c
float Person::eat_from_dry_run(CropPatch& c, float& handling, int& units_frm_patch) {
  CropPatch c_cpy;
  c_cpy = c;
  return eat_from(c_cpy, handling, units_frm_patch);
}


bool Person::will_stop() {
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
  db(" Eout: "); db(daily_use);
  db(" mxEn: "); db(max_energy);
  db(" mxEat: "); db(max_daily_eat);
  if(has_todo_limit) { db(" mxTodo: ") ; db((int)todo_limit) ; }
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
  db(" Hm:"); db(home_time);
}

void Person::show_num_places() {
  db(" Ple:"); db(num_places_eaten);
  db(" Plx:"); db(num_places_explored);

}

/* need just for stats */
void Person::update_places_eaten(ResPtr r) {
  map<ResPtr, bool>::const_iterator m_itr;
  m_itr = where_eaten.find(r);
  if(m_itr == where_eaten.end()) {
    where_eaten[r] = true;
    num_places_eaten++;
  }
  // else do nothing
}

/* need just for stats */
void Person::clear_places_eaten() {
  where_eaten.clear();
  num_places_eaten = 0;
}

/* need just for stats */
void Person::update_places_explored(ResPtr r) {
  map<ResPtr, bool>::const_iterator m_itr;
  m_itr = where_explored.find(r);
  if(m_itr == where_explored.end()) {
    where_explored[r] = true;
    num_places_explored++;
  }
  // else do nothing
}

/* need just for stats */
void Person::clear_places_explored() {
  where_explored.clear();
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

  //  order_type = PLAIN;
  // order_type = RANDOM;
  order_type = BY_FITNESS;

}

void Population::add(Population& other) {

 population.insert(population.end(), other.population.begin(), other.population.end());
  // bring tribes info from other
 tribes.insert(tribes.end(),other.tribes.begin(), other.tribes.end());
  // bring egal_tribes info from people_other
 egal_tribes.insert(egal_tribes.end(),other.egal_tribes.begin(), other.egal_tribes.end());
}

void Population::zero_eaten_today() {
  vector<Person *>::iterator p;
  p = population.begin();
  while(p != population.end()) {
    (*p)->eaten_today = 0.0; 
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

  /******************************************************/
  /* remove those too old, those with not enough energy */
  /******************************************************/
  update_by_cull(deaths_age, deaths_starve);
  r_line.DEATHS_AGE = deaths_age;
  r_line.DEATHS_STARVE = deaths_starve;
  
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
    r_line.HOMETIME_MAX_LIVING = 0;
    r_line.HOMETIME_MAX_DEAD = 0;
    r_line.HOMETIME_MAX = 0;
    r_line.TWOPLACETIME_MAX = 0;
    r_line.MAX_NUM_PLACES_EATEN = 0;
    r_line.MAX_NUM_PLACES_EXPLORED = 0;
    // no other person related updates are possible so return
    return true;
  }

  // from here assume non-zero population and further updates to calculate

  /*************************************************************************/
  /* make sure anyone's 'count down' knowledge on wipeouts has got updated */
  /* also anyone's personal gain info from day before is cleared                    */
  /*************************************************************************/
  //db_level= 0;
  update_time_dependent_knowledge();
  //db_level = 1;



  /****************************************************************/
  /* execute day's worth of moving about to find and consume food */
  /****************************************************************/
  //db_level = 0;
  update_by_move_and_feed(date);
  //db_level = 1;
  cout << "IF YOURE READING THIS IT DIDNT BREAK" << endl;
  // TEMP
  // to verify area_gains code working 
  // sanity_check_area_gains(population);

  if(pop.egal_tribes.size()>0) { update_by_redistrib(); }


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
  update_by_communication();

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

  

  // if(pop.egal_tribes.size()>0) { update_by_redistrib(); }

  //qt_show_occupancy();
  return updated;
  
  
}


void Population::update_by_cull(int& deaths_age, int& deaths_starve) { // age, expend energy, then cull, set nums to amounts culled

  vector<Person *>::iterator p;
    

  // let die those too old, those with not enough energy
  p = population.begin();
  while(p != population.end()) {
    //    cout << "considering "; p->show(); db("\n");
    (*p)->age = ((*p)->age + 1);
    (*p)->current_energy -= (*p)->daily_use;
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
    else {    
      p++; // update the iterator
    }

  }
  #if DEBUG
  db("finished culling\n");
  #endif

}

void Population::update_time_dependent_knowledge() {
  PerPtr p;
  for(size_t i=0; i < population.size(); i++) {
    p = population[i];
    if(p->retains_info && p->mind.has_time_dependent_info()) {
      #if DEBUG
      db(p->toid()); db(" time update knowledge\n");
      #endif
      p->mind.update_time_dependent_info(); // concerns Wipeouts, does output of any deletions
      #if DEBUG
      //if(p->type=='B') {
      if(p->mind.has_time_dependent_info()) {
	db(p->mind.tostring()); db("\n");
      }
      //}
      #endif
    }
  }

}


// [] set up everyone's todo_sched (entries to Resources objects)
// [] set up everyone's route to give all a route to first off todo_sched
//    sets each persons loc to first in route
// [] sets up and starts to process the event loop
//    5 kinds of events involved  
//    EndStageEvent, EndRestEvent, ArriveEvent, EndEatEvent, EndWaitEvent
//    init state of loop has for each person their EndStage events for 1st stage of their route
//
//    then repeatedly first event removed and handled according to kind via
//    EndStageEvent_proc, EndRestEvent_proc, ArriveEvent_proc, EndEatEvent_proc, EndWaitEvent_proc
void Population::update_by_move_and_feed(int date) {

  //db_level = 1;
  pop.set_all_todo();
  //db_level = 0;

  // gives everyone a route, also sets at_home, at_a_resource

  pop.set_all_routes();
  //db_level = 1;
  //db_level = 0;

  EventLoop loop;

  // init event loop : for each person their end-stage events at first part of their route
  loop.init_from_population(pop.population);
  #if DEBUG
  db("loc occupancy\n"); show_occupancy();
  #endif

  while(!loop.is_empty()) {

    #if DEBUG2
    //db("num events: "); db(loop.num_events()); db("\n");
    #endif
    #if DEBUG
    loop.show();
    #endif
    
    EventPtr e = loop.get_first(); // = (t, p, x) [time, person, loc]
    loop.pop_first();

    time_reached = e->t; // need to eventually catch final time reached 
    /**********************************/
    /*  case EndStage                 */
    /**********************************/
    if(e->kind == END_STAGE) {
      EndStageEvent *stg_ptr;
      stg_ptr = (EndStageEvent *)e;
      EndStageEvent_proc(stg_ptr,loop);
      delete e;
      continue;
    }

    /**********************************/
    /*  case Arrive v EndEat          */
    /**********************************/
    if((e->kind == ARRIVE) || (e->kind == END_EAT)) { 
      
      if(e->kind == ARRIVE) { 
        ArriveEvent *arr_ptr;	
        arr_ptr = (ArriveEvent *)e;
        ArriveEvent_proc(arr_ptr,loop,date);
        delete e;
        continue;
      }
   
      if(e->kind == END_EAT) {     // 'release' the patch which was being eaten, if any
        EndEatEvent *eat_ptr;
        eat_ptr =  (EndEatEvent *)e;
        EndEatEvent_proc(eat_ptr,loop);
        delete e;
        continue;
      }

     } // ARRIVE v END_EAT


    /****************************/
    /* EndWait                  */
    /****************************/
    if(e->kind == END_WAIT) {
      EndWaitEvent *wait_ptr;
      wait_ptr = (EndWaitEvent *)e;
      EndWaitEvent_proc(wait_ptr,loop);
      delete e;
      continue;
    }
    
    
    
  } // end event loop
  

  r_line.HOMETIME_MAX_LIVING = hml;
  r_line.HOMETIME_MAX_DEAD = hmd;
  r_line.HOMETIME_MAX = time_reached;
  r_line.TWOPLACETIME_MAX = max_time_for_two;
  r_line.MAX_NUM_PLACES_EATEN = max_places_eaten;
  r_line.MAX_NUM_PLACES_EXPLORED = max_places_explored;
}



//  roughly, where the EndStage event e = (t, p, x)
//    if not at end of route
//      CHANGES due to e:  update location of p from x (i.e. route_index)
//      if at 'hub': INSERT due to e: post an EndRest e'
//      else 
//       INSERT due to e: fresh EndStage event e' for next on route 
//
//    if at end of route && its a res entry
//      CHANGES due to e: set the person's res_ptr, visit_sched, revisit_sched
//                      : set person's entry time for relevant AreaGain
//      INSERTS due to e: fresh ArriveEvent (for 1st loc of resource)
//
//    if at end of route && its a home
//      CHANGES due to e: set at home
//      INSERTS due to e: none
void Population::EndStageEvent_proc(EndStageEvent *stg_ptr,EventLoop& loop) {
  PerPtr p;
  p = stg_ptr->p;
  LocNode* cur_loc;
  /*********************/
  /* not at route end  */
  /*********************/
  if(stg_ptr->route_index != (p->route.size()-1)) { 
    // CHANGES
    p->route_index = stg_ptr->route_index;
    p->set_route_loc();

    // if heading out of an area update gains-related vars for that area
    // just for book-keeping 
    if(p->move_state == LEAVING_AREA) {
      p->move_state = UNDEF;
      p->area_gains.set_an_area_duration(p->loc->resourceObject, stg_ptr->t);
    }
    
    cur_loc = p->loc;
    LocNode* prev_loc;
    prev_loc = p->route[p->route_index-1];

    // move on
    float start_t = stg_ptr->t; 
    LocNode* nxt_loc = p->route[p->route_index+1];
    EndStageEvent *nxt = new EndStageEvent;
    nxt->p = p;
    nxt->route_index = p->route_index+1;
    nxt->t = start_t + 1;
    nxt->st = start_t;
    #if DEBUG
        db(p->toid()); db(" plan:"); db(cur_loc->tostring()); db(" --> "); db(nxt_loc->tostring()); db("\n");
    #endif
    loop.insert(nxt);
  }
  /*********************/
  /* at route end      */
  /*********************/
  else { // either RES_ENTRY or HAB_ZONE
    // CHANGE
    p->route_index = stg_ptr->route_index;
    p->set_route_loc();
    cur_loc = p->loc;
    if(cur_loc->type == RESOURCE) { 
      // FURTHER CHANGES 
      p->at_a_resource = true;

      // set person's area entry time to this arrival time at the entry 
      // not doing this in ArriveEvent at first loc
      // cos can re-arrive at first loc of area several times
      // just for book-keeping
      p->move_state = EXPLORING_AREA; 	  
      p->area_gains.set_an_area_entry_time(cur_loc->resourceObject,stg_ptr->t);
	  
      // INSERTS
      ArriveEvent *fst = new ArriveEvent;
      fst->p = stg_ptr->p;
      fst->t = stg_ptr->t + 1;
      fst->st = stg_ptr->t;
	  
      #if DEBUG
            db(p->toid()); db(" plan:"); db(cur_loc->tostring()); db(" --> "); db(nxt_loc->tostring()); db("\n");
      #endif
      loop.insert(fst);
      //delete e;
    }
    else if(cur_loc->type == HAB_ZONE) { 
      // FURTHER CHANGES
      p->at_home = true;
      #if DEBUG
            db(p->toid()); db(" reached home:"); db("( "); db(p->home_loc->tostring()); db(")\n");
      #endif

      // lines below only for tracking whats going on, don't influence anything
      p->home_time = time_reached;
      if((time_reached > hml) && (p->current_energy >= p->daily_use) && (p->eaten_today > 0)) {
	      hml = time_reached;
      }
      if((time_reached > hmd) && (p->current_energy < p->daily_use)) {
	      hmd = time_reached;
      }

      if((time_reached > max_time_for_two) && (p->num_places_eaten == 2)) {
	      max_time_for_two = time_reached;
      }

      if(p->num_places_eaten > max_places_eaten) {
	      max_places_eaten = p->num_places_eaten;
      }

      if(p->num_places_explored > max_places_explored) {
	max_places_explored = p->num_places_explored;
      }

      // INSERTS: none
      //delete e;
    }
  }
  #if DEBUG
    db("loc occupancy\n"); show_occupancy();
  #endif
}

// roughly if Arrive  e = (t, p, x) 
//    CHANGES due to e:
//       update location of p from x (i.e. vis_index)
//       if at last loc of area possibly learn wipeout info from area
//    then call attemptEat_proc(..) to see if can eat here
void Population::ArriveEvent_proc(ArriveEvent *arr_ptr,EventLoop& loop, int& date) {
  // CHANGES Arrive
  // update location of person from event
  PerPtr p;	
  p = arr_ptr->p;
  
  if(p->at_home) { p->at_home = false;}

  // see if can eat here, leading to possible inserts
  attemptEat_proc(arr_ptr,loop);
}

// roughly if EndEat  e = (t, p, [+g,-u])
// CHANGES due to e:
//   update energy and crops from [+g,-u]
//   cancel all indicators on for p for eating 
//                           and on any patch where p was eating
// INSERTS due to e:
//   if(p would not like to eat more) {
//      set route to home, create EndStage event e' and insert e' }
//      make fresh EndStage e' for heading home
//      insert e' }
//   else { call attemptEat_proc(..)
void Population::EndEatEvent_proc(EndEatEvent *eat_ptr,EventLoop& loop) {


  PerPtr p;
  p = eat_ptr->p;
     // CHANGES EndEat
  // 'release' the patch which was being eaten, if any
  // if(p->at_a_patch) { // surely must be?
#if DEBUG
    db("ate: "); db(eat_ptr->gain); db(" ["); db(eat_ptr->units_frm_patch); db("]\n");
#endif
    // update person's energy attributes from this
    p->eaten_today += eat_ptr->gain;
    p->current_energy += eat_ptr->gain;
    // also update person's AreaGain info
    p->area_gains.increment_an_area_gain(p->loc->resourceObject, eat_ptr->gain);
    // update patch	
    p->eating_patch->remove_units(eat_ptr->units_frm_patch);
    p->eating_patch->being_eaten = false;
    p->eating_patch->eater = NULL;
#if DEBUG
    db("releasing "); db(p->eating_patch->sym); db(p->eating_patch->pos.tostring()); db("\n");
#endif
    // return person to state of having no patch
    //p->at_a_patch = false;
    p->eating_patch = NULL;
 
    //}
    
  // INSERTS
  if(p->will_stop()) {
#if DEBUG
    db(p->toid()); db(" finished\n");
    db("loc occupancy\n"); show_occupancy();
#endif
    LocNode* cur_loc = p->loc;
    p->set_route(cur_loc,p->home_loc); // should check
    EndStageEvent *nxt = new EndStageEvent;
    nxt->p = p;
    nxt->route_index = 1;
    nxt->t = eat_ptr->t + 1;
    p->move_state = LEAVING_AREA; // relying on fact that route[1] will be res entry and trigger area-change update
#if DEBUG
    db(p->toid()); db(" plan:"); db(cur_loc->tostring()); db(" --> "); db(p->route[1]->tostring()); db("\n");
#endif
    loop.insert(nxt);

  }
  else { // p would like to eat more

    attemptEat_proc(eat_ptr,loop); // poss further inserts

  }


}




// roughly, where event e = (t, p) and x is p's location
// [] if (p cannot eat at x [all patches empty or occupied]) {
//       if p has nowhere else to go in the area and some patches are occupied
//           make EndWait event e' and insert e'
//       else call attemptMoveOn_proc(..) to move elsewhere in the area
//     }
// [] if (p could eat at x) {
//      where patch c = random from all non-empty and unnoccuped at x
//        set all indicators on for p for eating 
//            and on the chosen patch
//        calc [+g,-u] for p if eat max poss from patch recording gain & handling h
//        make EndEatEvent e'(t+h, p, [+g,-u]) so projected energy/crop update
//        insert e'
//   }
void Population::attemptEat_proc(Event *e,EventLoop& loop) {
  // should arrive here if had ARRIVE or END_EAT or END_WAIT and p would like to eat more
  PerPtr p;
  ArriveEvent *arr_ptr;
  if(e->kind == ARRIVE) {
    arr_ptr = (ArriveEvent *)e;
    p = arr_ptr->p;
  }
  EndEatEvent *eat_ptr;
  if(e->kind == END_EAT) { 
    eat_ptr =  (EndEatEvent *)e;
    p = eat_ptr->p;
  }
  EndWaitEvent *wait_ptr;
  if(e->kind == END_WAIT) { 
    wait_ptr =  (EndWaitEvent *)e;
    p = wait_ptr->p;
  }
  
  // get all patches at x (i) not empty (ii) not occupied (ie. being_eaten is set true there)
  bool have_patches_here = false;
  vector<int> patches;
  have_patches_here = p->loc->resourceObject->patches_at_location(patches);


  if(!have_patches_here) {
    // so none non-empty or unoccupied so
    // if  at last poss location and some occupied patches there
    //   post EndWait event
    if(p->loc->resourceObject->being_eaten_patches_at_location()) {
      wait_ptr = new EndWaitEvent;
      wait_ptr->p = p;
      //wait_ptr->t = e->t + EndWaitEvent::wait_duration_def;
      wait_ptr->t = e->t + p->wait;
      // #if DEBUG1
      // db(p->toid()); db(" waits at "), db(p->loc->tostring()), db("\n");
      // #endif
      loop.insert(wait_ptr);
    }
    else {
    // otherwise try to post event for moving to next place
    // note if at final part of vis_sched involves use revisit_sched and basically swapping
    // revisit_sched for visit_sched: see get_nxt_location for details
    attemptMoveOn_proc(p, e,  loop);
    // Note passing e just because need to access e->t to insert further events
    }
  } 

  
  if(have_patches_here) {
    // so some non-empty and unoccupied
    
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
#if DEBUG
    db("hiding "); db(p->eating_patch->sym); db(p->eating_patch->pos.tostring()); db("\n");
#endif

    // TEMP: for sake of later stats gathering
    p->update_places_eaten(p->loc->resourceObject);
	
    //  make p eat max poss from patch, calculating handling time 'handled'
    float gain;
    float handled;
    int units_frm_patch;
      
    //
    gain = p->eat_from_dry_run(c,handled,units_frm_patch); // calc but dont do update

    
    // insert (t+h, p, x) where h is the handling time, same loc
    EndEatEvent *ltr = new EndEatEvent;
    ltr->t = e->t + handled;
    ltr->p = p;
    //ltr->vis_index = p->vis_index; // no mvmt scheduled
    ltr->units_frm_patch = units_frm_patch;
    ltr->gain = gain;
    

    loop.insert(ltr);

	

#if DEBUG
    db("loc occupancy\n"); show_occupancy();
#endif


  } // end have_patches_here


}

// called from attemptEat_proc(.. e ..)
//  INSERTS due to e
//     if(there is further loc x' in current Resources) {
//       make Arrive event e' (t',p, x')  where x' is next place, t' is time given speed of p
//       insert e' 
//     }
//     else if(not at end of todo_sched) {
//             set route to reach nxt frm todo_sched, create EndStage event e' for
//             first stage of that route and insert e'}
//     else {  set route to reach home, create EndStage event e' for
//             first stage of that route and insert e'}
//     }
void Population::attemptMoveOn_proc(Person *p, Event *e, EventLoop& loop) {

 
  // so none non-empty or unoccupied so
  // try to post event for moving to next place
  // note if at final part of vis_sched involves use revisit_sched and basically swapping
  // revisit_sched for visit_sched: see get_nxt_location for details 
  bool found_next = false;
  LocNode* nxt_loc; int nxt_index;
  LocNode* cur_loc = p->loc;
  size_t nxt_todo_index;

  if(p->get_nxt_frm_todo_sched(nxt_loc,nxt_todo_index)) {
    // moving on to whole other Resources ara
    p->todo_index = nxt_todo_index;
    p->set_route(cur_loc,nxt_loc);
    EndStageEvent *nxt = new EndStageEvent;
    nxt->p = p;
    nxt->route_index = 1;
    nxt->t = e->t + 1;
    p->move_state = LEAVING_AREA; // relying on fact that route[1] will be res entry and trigger area-change update
    #if DEBUG
        db(p->toid()); db(" to nxt todo:"); db(to_string(p->todo_sched[p->todo_index]));
        db(" route"); db(p->route_tostring()); db("\n");
        db("loc occupancy\n"); show_occupancy();
    #endif
    loop.insert(nxt);
  }
  else { // going to head home
    // note might get here with only patches still occupied in last destination
    // so really should wait and not go home
    // not sure how to deal with this properly for the moment
    p->set_route(cur_loc,p->home_loc); // should check
    EndStageEvent *nxt = new EndStageEvent;
    nxt->p = p;
    nxt->route_index = 1;
    nxt->t = e->t + 1;
    p->move_state = LEAVING_AREA; // relying on fact that route[1] will be res entry and trigger area-change update	  	  #if DEBUG
#if DEBUG
    db(p->toid()); db(" plan (no choice):"); db(cur_loc->tostring()); db(" --> "); db(p->home_loc->tostring()); db("\n");
    db("loc occupancy\n"); show_occupancy();
#endif
    loop.insert(nxt);
  }
}

// this event posted from earlier call to attemptEat_proc cos at last possibility
// within an area and had to wait because patches occupied
// --> calls attemptEat_proc(..) again
void Population::EndWaitEvent_proc(EndWaitEvent *wait_ptr,EventLoop& loop) {
#if DEBUG
  db("processing Wait event"); db(wait_ptr->tostring()); db("\n");
#endif
  
  attemptEat_proc(wait_ptr,loop);
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

bool Population::collect_talkers(vector<PerPtr>& sub_pop) {
  sub_pop.clear();
  for(size_t i=0; i < population.size(); i++) {
    PerPtr p = population[i];
    if(p->imparts_info) {
      sub_pop.push_back(p);
    }
  }
  if(sub_pop.size() > 0) {
    return true;
  }
  else {
    return false;
  }
}

void Population::update_by_communication() {
  vector<PerPtr> talkers;
  bool have_talkers = false;
  have_talkers = collect_talkers(talkers);
  
  if(have_talkers) {
    for(size_t i=0; i < (talkers.size()-1); i++) {
      PerPtr p1 = talkers[i];
      for(size_t j=i+1; j < talkers.size(); j++) {
	PerPtr p2 = talkers[j];
	confer(p1,p2);
      }
    }
  
    for(size_t i=0; i < (talkers.size()-1); i++) {
      PerPtr p1 = talkers[i];
      for(size_t k=0; k < p1->mind.wipeouts.size(); k++) {
	p1->mind.wipeouts[k].impart_today = false;
      }
    }


  } // end if talkers branch
  else {

  }
}

void Population::confer(PerPtr p1, PerPtr p2) {
  
  for(size_t i=0; i < p1->mind.wipeouts.size(); i++) {
    WipeoutInfo w1 = p1->mind.wipeouts[i];
    if(w1.impart_today && !(p2->mind.has_this_wipeout(w1))) {
       WipeoutInfo w2;
       w2 = w1;
       w2.impart_today = false;
       w2.orig = TALKING;
       p2->mind.wipeouts.push_back(w2);
       #if DEBUG
       db(p1->toid()); db(" -> "); db(p2->toid()); db(w2.tostring()); db("\n");
       #endif
    }
  }

  for(size_t i=0; i < p2->mind.wipeouts.size(); i++) {
    WipeoutInfo w2 = p2->mind.wipeouts[i];
    if(w2.impart_today && !(p1->mind.has_this_wipeout(w2))) {
       WipeoutInfo w1;
       w1 = w2;
       w1.impart_today = false;
       w1.orig = TALKING;
       p1->mind.wipeouts.push_back(w1);
       #if DEBUG
       db(p2->toid()); db(" -> "); db(p1->toid()); db(w1.tostring()); db("\n");
       #endif
    }
  }

  

}


void Population::aggregate_area_gains(char type,AreaGains &agg, string what_to_note) {
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
	if((what_to_note == "first_sched_gains") && (p->todo_sched[0]->resourceObject != pag_itr->first)) {
	// this area not relevant as not first on p's sched
	continue;
	}
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

void Population::aggregate_sched_gains(char type,vector<float> &sched_gains) {
  vector<PerPtr> sub_pop;
  collect_subtype(type, sub_pop);
  
  sched_gains.clear();
  sched_gains.resize(all_res.size());
  for(size_t i=0; i < sched_gains.size(); i++) {
    sched_gains[i] = 0;
  }

  for(size_t i=0; i < sub_pop.size(); i++) {
    PerPtr p = sub_pop[i];
    for(size_t si = 0; si < p->todo_sched.size(); si++) {
      ResPtr r = p->todo_sched[si]->resourceObject;
      float g = 0;
      if(p->area_gains.get_gain(r,g)) {
        sched_gains[si] += g;
      }
    } // end loop over p's todo_sched
  } // end loop over sub_pop

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




void Population::set_all_todo() {
  PerPtr p;
  
  for(size_t i=0; i < population.size(); i++) {
    p = population[i];
    //db_level = 1;
    if(p->set_todo_sched()) {
      #if DEBUG
      db(p->toid()); db(" sched:"); db(p->todo_sched_tostring()); db("\n");
      #endif
    }
    p->move_state = UNDEF;
    //db_level = 0;
  }
}

// this presupposes set_all_todo has run
void Population::set_all_routes() {
  LocNode* fst;
  LocNode* lst;
  PerPtr p;
  
  for(size_t i=0; i < population.size(); i++) {
    p = population[i];
    p->at_home = true;  // poss redundant?
    p->at_a_resource = false; // poss redundant?
    p->loc = p->home_loc; // poss redundant?
    fst = p->loc; 
    lst = p->todo_sched[0]; 
    if(p->set_route(fst,lst)) {
      #if DEBUG
      db(p->toid()); db(" route:"); db(p->route_tostring()); db("\n");
      #endif
    }
    else { /* some error */ }

  }

}


void Population::clear_all_area_gains() {
 PerPtr p;
  for(size_t i=0; i < population.size(); i++) {
    p = population[i];
    p->area_gains.clear_area_gains();
  }

}


// make all equal
void Population::update_by_redistrib() {
  for(int t=0; t < egal_tribes.size(); t++) {
    vector<PerPtr> egal_pop;
    collect_subtype(egal_tribes[t], egal_pop);
    double total_en = 0.0;
    vector<PerPtr>::iterator p;
    p = egal_pop.begin(); 
    while(p != egal_pop.end()) {
      total_en += (*p)->current_energy;
      p++;
    }
    double shared_en;
    shared_en = total_en/egal_pop.size();
    p = egal_pop.begin(); // go to start again
    while(p != egal_pop.end()) {
      (*p)->current_energy = shared_en;
      p++;
    }
    
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
  daily_use = p->daily_use;
  max_energy = p->max_energy;
  max_daily_eat = p->max_daily_eat;
  init_energy =  p->init_energy;
  speed = p->speed;
  has_todo_limit = p->has_todo_limit;
  todo_limit = p->todo_limit;
  has_home_time_limit = p->has_home_time_limit;
  home_time_max = p->home_time_max;
  
  current_energy = init_energy;
  hrate = p->hrate; //
  home_loc = p->home_loc;
  at_home = p->at_home;
  loc = home_loc;
  retains_info = p->retains_info;
  retains[WIPE] = p->retains[WIPE];
  imparts_info = p->imparts_info;
  imparts[WIPE] = p->imparts[WIPE];

  wait  = p->wait;
  rest  = p->rest;
  
  todo_choice_method = p->todo_choice_method;

}

// bool compare_person(const Person& p1, const Person& p2) {
//   if(p1.type < p2.type) { return true; }
//   else if(p1.type == p2.type && p1.age > p2.age) { return true; }
//   else {return false;}

// }

bool compare_person(Person *p1, Person *p2) {
  if(p1->type < p2->type) { return true; }
  else if(p1->type == p2->type && p1->age > p2->age) { return true; }
  else {return false;}

}

string Person::info_type_to_string() {
  string s = "";
  s += "retains_info:"; s += to_string(retains_info);
  if(retains_info) {
    s += " about(/";
    if(retains[WIPE]) { s += "wipe/"; }
    s += ")";
  }
  
  s += " imparts_info:"; s += to_string(imparts_info);
  if(imparts_info) {
    s += " about(/";
    if(imparts[WIPE]) { s += "wipe/"; }
    s += ")";
  }
    
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

void Population::calc_res_occupancy() {

  for(size_t r=0; r < all_resource_loc.size(); r++) {
    all_resource_loc[r]->occupancy = 0;
  }

  
  Person *p;
  LocNode* l;
 
  for(size_t i=0; i < population.size(); i++) {
    p = population[i];
    l = p->loc;
    if(l->type == RESOURCE) { // p at home has at least vis_index == -1
      l->occupancy++;
    }
 
  }
}

void Population::calc_home_occupancy() {
  Person *p;
  LocNode* l;

  for(size_t i=0; i < all_home_loc.size(); i++) {
    l = all_home_loc[i];
    l->occupancy = 0;
    LocNode* n_ptr;
    n_ptr->occupancy = 0;
  }
    
  for(size_t i=0; i < population.size(); i++) {
    p = population[i];
    l = p->loc;
    if(l->type == HAB_ZONE) { 
      l->occupancy++;
    }
    
  }

}

void Population::qt_show_crops() {
#if USE_QT
  //  Population *p = NULL;
  // Resources *r = NULL;
  if(WorldShow::res_level != 0) { return; }

  Population *p = this;
  // this now a global var
  // vector<ResPtr> all_res;
  // all_res.push_back(&res);
  // all_res.push_back(&res2);
  
  //Resources *r = &res;
  WorldShow window(p); 
  window.exec();
  //cout << "finished in qt_show_occupancy res_level:";
  //cout << WorldShow::res_level << endl;


#endif

}




void Population::qt_show_occupancy() {
#if USE_QT
  //  Population *p = NULL;
  // Resources *r = NULL;
  if(WorldShow::res_level != 1) { return; }


  calc_home_occupancy();
  calc_res_occupancy();
  calc_res_entry_occupancy();
  
  Population *p = this;
  // this now a global var
  // vector<ResPtr> all_res;
  // all_res.push_back(&res);
  // all_res.push_back(&res2);
  
  //Resources *r = &res;
  WorldShow window(p); 
  window.exec();
  //cout << "finished in qt_show_occupancy res_level:";
  //cout << WorldShow::res_level << endl;
#endif
}

int Population::get_total() {
  return population.size();

}

bool Population::compare_by_index(const int& p1_index, const int& p2_index) {
  return compare_person(population[p1_index], population[p2_index]);

}


