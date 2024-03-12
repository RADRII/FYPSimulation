#include "Action.h"
#include "Person.h"
#include "Util.h"
#include "Debug.h"
#include <iostream>

Action::Action() {
}

Action::~Action() {}

string Action::tostring() {
  string s = "junk";
  return s;
}

void Action::show() {
  db(tostring()); db("\n"); 
}


RouteAction::RouteAction() {
  kind = ROUTE;
  p = NULL;
  route_index = 0;
}

RouteAction::~RouteAction() {};

string RouteAction::tostring() {
  string s = "Route-";
  s += f_to_s(route_index);
  s += " for:";
  s += p->toid();
  s += ".";
  return s;
}

ExploreAction::ExploreAction() {
  kind = EXPLORE;
  p = NULL;
}

ExploreAction::~ExploreAction() {};

string ExploreAction::tostring() {
  string s = "Explore";
  s += " for:";
  s += p->toid();
  s += ".";
  return s;
}

EatAction::~EatAction(){};

EatAction::EatAction() {
  kind = EAT;
  p = NULL;
}

// temp made identical to ArriveEvent to replicated current code
string EatAction::tostring() {
  string s = "Eat";
  s += " for:";
  s += p->toid();
  s += ", ate ";
  s += f_to_s(gain);
  s += " [";
  s += to_string(units_frm_patch);
  s += "]";
  s += "."; 
  return s;
}


HomeAction::HomeAction() {
  kind = HOMEREST;
  p = NULL;
}

HomeAction::~HomeAction() {};

string HomeAction::tostring() {
  string s = "HomeRest";
  s += " for:";
  s += p->toid();
  s += ".";
  return s;
}

WaitAction::WaitAction() {
  kind = WAIT;
  p = NULL;
}

WaitAction::~WaitAction() {};

string WaitAction::tostring() {
  string s = "Wait";
  s += " for:";
  s += p->toid();
  s += "[";
  s += f_to_s(waitTime);
  s += "]";
  s += ".";
  return s;
}

Person *p;  // concerning who

ActionList::ActionList() {};

void ActionList::init_from_population(vector<Person *> persons) {
  // insert into action loop in semi-random order
  // all eats before anything else.

  //Create randomly shuffled mask for persons
  int *people_ordering;
  int people_ordering_size = persons.size();
  people_ordering = new int[people_ordering_size];
  for(int o=0; o < people_ordering_size; o++) {
    people_ordering[o] = o;
  }
  gsl_ran_shuffle(r_global, people_ordering, people_ordering_size, sizeof(int));

  //Go through persons randomly adding actions to list
  for(int o = 0; o < persons.size(); o++)
  {
    int i = people_ordering[o];

    ActionPtr action = persons[i]->getNextAction(false);
    if(action->kind == EAT && persons[i]->hasBeenEating)
    {
        lp.push_front(action);
    }
    else
    {
        lp.push_back(action);
    }
    
    string s = "";
    if(persons[i]->isHeadingHome) s = "(H)";
    debug_record << persons[i]->identifier << " decided to " << action->kind << s << endl;
  }
}

void ActionList::show() {
  list<ActionPtr>::iterator it;
  for(it = lp.begin(); it != lp.end(); it++) {
    db((*it)->tostring()); db(" ");
  }
  db("\n");
}

void ActionList::show_first() { 
  list<ActionPtr>::iterator it = lp.begin();
  db((*it)->tostring()); db("\n");
}

ActionPtr ActionList::get_first() { 
  return lp.front();
}

void ActionList::pop_first() { 
  lp.pop_front();
}

void ActionList::insert(ActionPtr a) {
   lp.push_back(a); 
}

void ActionList::clear() {
  lp.clear();
}


bool ActionList::is_empty() {
  return (lp.size() == 0); 
}

int ActionList::num_actions() {
  return lp.size();
}

