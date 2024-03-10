#if !defined(POPEVENT_H)
#define POPEVENT_H

#include <string>
#include <list>
//#include "People.h"
#include "Grid.h"
#include <vector>
using namespace std;

class Person; // forward decl

enum EventKind {END_STAGE, END_REST, ARRIVE, END_EAT, REACH_HOME, END_WAIT};

class Event {
 public:
  Event();
  virtual ~Event();
  float t;     // don't call it 'tick' cos can be any real number
  float st;    // when was this event posted
  EventKind kind;
  // needed for using st to t to represent start and end of a process
  // and doing a reschedule due to an interrupt
  virtual string tostring();
  void show();
};

typedef Event * EventPtr;

class EndStageEvent : public Event {
 public:
  EndStageEvent();
  ~EndStageEvent(); // prob overkill at moment
  Person *p;  // concerning who
  //Location *l; // where reached
  size_t route_index; // where to reach in route when executed
  string tostring();

};

class EndRestEvent : public Event {
 public:
  EndRestEvent();
  ~EndRestEvent(); // prob overkill at moment
  Person *p;  // concerning who
  string tostring();
  static float rest_duration_def;
};


class ArriveEvent : public Event {
 public:
  ArriveEvent();
  ~ArriveEvent(); // prob overkill at moment
  Person *p;  // concerning who
  //Location *l; // where reached
  int vis_index; // where to reach in visit_sched when executed
  string tostring();

};

class EndEatEvent : public Event {
 public:
  EndEatEvent();
  ~EndEatEvent(); // prob overkill at moment
  Person *p;  // concerning who (which patch accessible via this)
  int units_frm_patch; // the units p is supposed now to have eaten (incl half-eaten)
  float gain; // energy gained 
  string tostring();
};

class ReachHome: public Event {
 public:
  ReachHome();
  ~ReachHome(); 
  Person *p;  // concerning who
  // relevant location is p->home_loc
  string tostring();
};

class EndWaitEvent : public Event {
 public:
  EndWaitEvent();
  ~EndWaitEvent(); // prob overkill at moment
  Person *p;  // concerning who
  string tostring();
  static float wait_duration_def;
};

 

class EventLoop {
 public:
  EventLoop();
  // no longer used?
  //  EventLoop(vector<Location> *the_locs);
  void init_from_population(vector<Person *>); 
  void show();
  void show_first();
  void pop_first();
  EventPtr get_first();
  void insert(EventPtr e); // put into list according to its time
  void clear();
  bool is_empty();
  int num_events();
  void process_next_event();
  
  
 private:
  list<EventPtr> lp;
  //no longer used?
  //vector<Location> *locs;
  //Location home_loc;


};

#endif
