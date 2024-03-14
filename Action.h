#if !defined(POPACTION_H)
#define POPACTION_H

#include <string>
#include <list>
//#include "People.h" unneeded bc of forward decl
#include "Grid.h"
#include <vector>
using namespace std;

class Person; // forward decl

enum ActionKind {ROUTE, EXPLORE, EAT, HOMEREST, WAIT, START};

class Action {
 public:
  Action();
  virtual ~Action();
  ActionKind kind;
  virtual string tostring();
  void show();
  Person *p;  // concerning who
};

typedef Action * ActionPtr;

class RouteAction : public Action {
 public:
  RouteAction();
  ~RouteAction(); // prob overkill at moment
  size_t route_index; // where to reach in route when executed
  string tostring();
};

class ExploreAction : public Action {
 public:
  ExploreAction();
  ~ExploreAction(); // prob overkill at moment
  string tostring();
};

class EatAction : public Action {
 public:
  EatAction();
  ~EatAction(); // prob overkill at moment
  int units_frm_patch; // the units p is supposed now to have eaten (incl half-eaten)
  float gain; // energy gained 
  string tostring();
};

class HomeAction: public Action {
 public:
  HomeAction();
  ~HomeAction(); 
  // relevant location is p->home_loc
  string tostring();
};

class WaitAction: public Action {
 public:
  WaitAction();
  ~WaitAction(); 
  int waitTime;
  string tostring();
};

class ActionList {
 public:
  ActionList();
  void init_from_population(vector<Person *>); 
  void show();
  void show_first();
  void pop_first();
  ActionPtr get_first();
  void insert(ActionPtr a); // put into list according to its time
  void clear();
  bool is_empty();
  int num_actions();
  void process_next_action();
  
 private:
  list<ActionPtr> lp;
};

#endif
