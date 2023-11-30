#include "Event.h"
#include "People.h"
#include "Util.h"
#include "Debug.h"
#include <iostream>

Event::Event() {
  t = 0.0; 
  st = 0.0;
}

Event::~Event() {}

string Event::tostring() {
  string s = "junk";
  return s;
}

void Event::show() {
  db(tostring()); db("\n"); 
}


EndStageEvent::EndStageEvent() {
  kind = END_STAGE;
  t = 0.0;
  st = 0.0;
  p = NULL;
  // ? default val for route_index
}

EndStageEvent::~EndStageEvent() {};

string EndStageEvent::tostring() {
  string s = "(";
  s += f_to_s(t);
  s += " per:";
  //  s += to_string(p->identifier);
  s += p->toid();
  s += " end-stage ";
  s += p->route[route_index]->tostring();
  s += ")"; 
  return s;

}


EndRestEvent::EndRestEvent() {
  kind = END_REST;
  t = 0.0;
  st = 0.0;
  p = NULL;
}

EndRestEvent::~EndRestEvent() {} 

string EndRestEvent::tostring() {
  string s = "(";
  s += f_to_s(t);
  s += " per:";
  //  s += to_string(p->identifier);
  s += p->toid();
  s += " end-rest";
  s += ")"; 
  return s;

}

float EndRestEvent::rest_duration_def = 4.0;
//float EndRestEvent::rest_duration_def = 0.1;
//float EndRestEvent::rest_duration_def = 8.0;

ArriveEvent::ArriveEvent() {
  kind = ARRIVE;
  t = 0.0;
  st = 0.0;
  p = NULL;
}

ArriveEvent::~ArriveEvent() {};

string ArriveEvent::tostring() {
  string s = "(";
  s += f_to_s(t);
  s += " per:";
  //  s += to_string(p->identifier);
  s += p->toid();
  s += " reach ";
  s += p->visit_sched[vis_index]->tostring();
  s += ")"; 
  return s;

}

EndEatEvent::~EndEatEvent(){};

EndEatEvent::EndEatEvent() {
  kind = END_EAT;
  t = 0.0;
  st = 0.0;
  p = NULL;
}

// temp made identical to ArriveEvent to replicated current code
string EndEatEvent::tostring() {
   string s = "(";
  //s += to_string(tick);
  s += f_to_s(t);
  s += " per:";
  //  s += to_string(p->identifier);
  s += p->toid();
  s += " ate ";
  s += f_to_s(gain);
  s += " [";
  s += to_string(units_frm_patch);
  s += "]";
  //  s += p->visit_sched[vis_index]->tostring();
  s += ")"; 
  return s;
}


ReachHome::ReachHome() {
  kind = REACH_HOME;
  t = 0.0;
  st = 0.0;
  p = NULL;
}

ReachHome::~ReachHome() {};

string ReachHome::tostring() {
  string s = "(";
  s += f_to_s(t);
  s += " per:";
  s += p->toid();
  s += " ";
  s += p->home_loc->id;
  s += "< )"; 
  return s;

}

EndWaitEvent::EndWaitEvent() {
  kind = END_WAIT;
  t = 0.0;
  st = 0.0;
  p = NULL;

}

EndWaitEvent::~EndWaitEvent() {}

string EndWaitEvent::tostring() {
  string s = "(";
  s += f_to_s(t);
  s += " per:";
  //  s += to_string(p->identifier);
  s += p->toid();
  s += " end-wait";
  s += ")"; 
  return s;

}

Person *p;  // concerning who

float EndWaitEvent::wait_duration_def = 1.0;






EventLoop::EventLoop() {};

// no longer needed?
// EventLoop::EventLoop(vector<Location> *the_locs) {
//   locs = the_locs;
//   //  home_loc.x = 0.0;
//   // home_loc.id = "home";
// }

void EventLoop::init_from_population(vector<Person *> persons) {

  // insert into event loop in random order
  // will still be intrinsically ordered by times 

  int *people_ordering;
  int people_ordering_size = persons.size();
  people_ordering = new int[people_ordering_size];
  for(int o=0; o < people_ordering_size; o++) {
    people_ordering[o] = o;
  }
  gsl_ran_shuffle(r_global, people_ordering, people_ordering_size, sizeof(int));
  
  // for(int i=0; i < persons.size(); i++) {
  for(int o=0; o < people_ordering_size; o++) {
    int i = people_ordering[o];
    LocPtr fst_loc = persons[i]->route[0];
    LocPtr nxt_loc = persons[i]->route[1];
    EndStageEvent *fst = new EndStageEvent;
    fst->p = persons[i];
    fst->route_index = 1;  
    fst->t = persons[i]->get_trav_time(fst_loc, nxt_loc);
    fst->st = 0.0;
    insert(fst); // automatically casts to base-class ptr ?
    

  }
  delete [] people_ordering;
}


void EventLoop::show() {
  list<EventPtr>::iterator it;
  for(it = lp.begin(); it != lp.end(); it++) {
    db((*it)->tostring()); db(" ");
  }
  db("\n");
}

void EventLoop::show_first() { 
  list<EventPtr>::iterator it = lp.begin();
  db((*it)->tostring()); db("\n");
}

EventPtr EventLoop::get_first() { 
  return lp.front();
}

void EventLoop::pop_first() { 
  lp.pop_front();
}

// 3 into {} --> 3
// 3c into 3a 3b 4 ...
// --> 3a 3b 3c 4 ...
// 4c into 3a 3b 4a 4b ...
// --> 3a 3b 4a 4b 4c ..
// 5 into 3a 3b 4a 4b
// --> 3a 3b 4a 4b 5
// linked list 'insert' goes *before* the iterator
void EventLoop::insert(EventPtr e) {

  float time = e->t;
  bool inserted = false;
  list<EventPtr>::const_iterator it;
  for(it = lp.begin(); it != lp.end(); it++) {
    if(time < (*it)->t) {
      lp.insert(it,e);
      inserted = true;
      break;
    }
  }
  if(!inserted) {
    lp.insert(it,e); // it reached end
  }
    
}

void EventLoop::clear() {
  lp.clear();
}


bool EventLoop::is_empty() {
  return (lp.size() == 0); 
}

int EventLoop::num_events() {
  return lp.size();
}





