#if !defined(PEOPLE_h)
#define PEOPLE_h

#include <string>
#include <vector>
#include "Resource.h"
#include "AreaGain.h"
#include "Event.h"
#include "Knowledge.h"
using namespace std;



class FamilyPlan {
 public:
  FamilyPlan();
  FamilyPlan(int age, int whose, int pl, int first, vector<int> nxt);
  int whose_plan;
  int planned_offspring;
  void choose_planned_offspring();
  int wait_first;
  void choose_wait_first();
  vector<int> wait_next;
  void choose_wait_next();
  
  int next_birth_age;
  void set_plan(int age, int who);
  void show();


};

enum MoveState { UNDEF, LEAVING_AREA, EXPLORING_AREA};

enum Activity { EATING, RESTING };

typedef char Tribe;

class Person;
typedef Person * PerPtr;

class Person {
 public:
  Person();
  void show_defaults(ostream& o);
  // shows the default values on important persistent characteristics set by above constructor
  // only makes sense to run before any 'customisation'
  
  static int person_count; // this increments on each creation of a Person
  int identifier; // this set to val of person_count on creation of a Person
  // so is unique int identifier of the person

  /********************************************************************/
  /* FIXED attributes ie. set at birth and never changed              */
  /* some or all of these would switched to make a customised Person  */
  /********************************************************************/
  
  char type; // used a symbol to distinguish different 'types' of people
  int expiry_age; // will not live beyond this age; could die earlier
  float curiosity;

  // ENERGY
  float init_energy; // 'energy' at birth
  
  float daily_use; // energy 'out' in a single day (some basic metabolism, some foraging)
                   // withoug adding to current_energy by eating this decrement will lead
                   // to death

  float max_energy; // how much can energy can be stored up at most
  
  float max_daily_eat; // cannot cram more food in (as energy) in a single day than this in
  // NB: max_energy could be 15 and this could be 5 so *less*
  // --> would take >1 day to reach max

  float hrate; // time to 'handle' one unit from a CropPatch; this makes feeding take time
  // even once food has been located. could be though to represent time to collect and
  // consume a unit of food
  // assume same time-units as speed
  // so hrate * units comparable to speed * dist

  // REPRO
  int repro_age_start;
  int repro_age_end;

  // MOVEMENT
  float speed; // rate at which distance is covered

  // size of todo_sched
  bool has_todo_limit;
  size_t todo_limit;

  bool has_home_time_limit;
  float home_time_max;

  float rest; // how long will rest at hub location  
  float wait; // how long will wait at last possible location of an area
  float mtime; // time it takes to say something
  float talk_cost; // cost of saying something
  
  /*******************************/
  /* end of FIXED attributes     */
  /*******************************/
  
  int age; // current 'age': the number of simulation 'days' they have been alive

  

  // relating to ENERGY in and out 
  
  float current_energy; // 'energy', based on food eaten; assume alive unless this 0
  
  float eaten_today; // amount (as energy) added so far during a day

  float eat_from_dry_run(CropPatch& c, float& handling, int& units_frm_patch);
  // calc what energy would happen if eating up to relevant energy limits from patch c
  // -- will not cause current_energy to grow too large if added
  //  -- will not be more than max_daily_eat
  // sets handling to time which would be taken for units consumed in generating this gain
  // sets units_frm_patch to num of units which would be consumed in generating this gain

  
  bool will_stop();
  // checks whether Person has reached relevant energy limits (max_energy or max_daily_eat)
  // and so will stop seeking food


  bool at_a_patch; // indicates that Person is actively consuming food from a particular patch
                   
  CropPatch *eating_patch; // the patch they are eating from
                           // NB: this should be consistent with the patch's being_eaten
  



  
  // if eating at a location find if there are others also there
  bool others_at_loc(vector<PerPtr>& others);

  
  // relating to REPRO 

  int num_offspring;
  void set_frm_parent(Person *parent);
  FamilyPlan fam_plan;
  
  // relating to Location

  LocNode* loc; // where the Person is

  // relating to NAVIGATION
  MoveState move_state;
  bool heading_home();
  
  /**************************************************************/
  /* following relate to navigation  OUTSIDE a Resources object */
  /**************************************************************/
  size_t route_index;
  bool set_route(LocNode* fst, LocNode* lst);
  vector<LocNode*> route;
  string route_tostring();
  void show_route();
  void set_route_loc(); // uses route_index (bad name as its set the loc frm the route)

  
  /**********************************************************/
  /* following relating to planning ie. creating todo_sched */
  /**********************************************************/
  bool at_a_resource;

  vector<LocNode*> todo_sched; 
  size_t todo_index;
  bool set_todo_sched();
  
  bool reset_todo_sched();   
  static const size_t NO_TODO;
  string todo_sched_tostring();
  bool get_nxt_frm_todo_sched(LocNode* res_entry_loc, size_t& nxt_todo);

  string todo_choice_method;

  /*******************************************************/
  /* relating to what a Person might know and talk about */
  /*******************************************************/
  bool retains_info;
  bool retains[1];
  Knowledge mind;
  bool imparts_info;
  bool imparts[1];


  string info_type_to_string();
  void info_type_show();
  
  bool at_home;
  LocNode* home_loc;

  /*************************************/
  /* relating to display of attributes */
  /*************************************/
  string toid();
  void show();
  void show_var_energy(); // called by show for 'variable' energy attributes

  // used for stats and knowledge
  AreaGains area_gains;
  
  // used only for compiling stats
  float home_time;
  void update_places_eaten(ResPtr);
  void clear_places_eaten();
  int num_places_eaten;
  void update_places_explored(ResPtr);
  void clear_places_explored();
  int num_places_explored;  


  
 private:
  void show_home_time();
  //void show_num_places_eaten();
  void show_num_places();
  //void show_num_places_explored();

  
  map<ResPtr,bool> where_eaten;
  map<ResPtr,bool> where_explored;



  
  float eat_from(CropPatch& c, float& handling, int& units_frm_patch);
  // determine energy update via consuming food from c
  // note: does not update the Person 
  // note: though this updates c, it is only called by eat_from_dry_run(..)
  // on a *copy* of an actual CropPatch
  // updates of actual CropPatchs done by CropPatch::remove_units(..) 
};




// bool compare_person(const Person& p1, const Person& p2);
bool compare_person(Person *p1, Person *p2);

enum PopOrderType { PLAIN, RANDOM, BY_FITNESS };

class Population {
 public:
  Population();
  Population(string id, int size);
  void add(Population& p);
  
  string id;

  vector<Person *> population;
  bool update(int date); // top-level 'update' function which calls series of further update_... functions
                         // to eg. let die those too old, those with not enough energy
                         //        expend energy and try to eat
                         //        add new population members


  void update_by_cull(int& deaths_age, int& deaths_starve); // age, expend energy, then cull,
  // settings the inputs to the amounts culled
  void update_time_dependent_knowledge();
  

  void update_by_repro(int& num); // add new population members, set num to number born
  void update_by_move_and_feed(int date);

  void EndStageEvent_proc(EndStageEvent *stage_ptr,EventLoop& loop);
  void ArriveEvent_proc(ArriveEvent *arr_ptr,EventLoop& loop, int& date);
  void EndEatEvent_proc(EndEatEvent *eat_ptr,EventLoop& loop);
  void EndWaitEvent_proc(EndWaitEvent *wait_ptr,EventLoop& loop);

  void attemptEat_proc(Event *e,EventLoop& loop);

  void attemptMoveOn_proc(Person *p, Event *e, EventLoop& loop);

  

  
  void update_by_redistrib(); // make all equal
  
  void update_by_communication();
  void confer(PerPtr p1, PerPtr p2);
  
  void show();
  void zero_eaten_today();
  int get_total();

  void set_all_todo();
  void set_all_routes();
  
  // jun8 2020: clears objective gains, not knowledge  
  void clear_all_area_gains();

  // bool egalitarian; // true means all have same energy default false
  vector<Tribe> tribes;
  vector<Tribe> egal_tribes; // members of these types have same energy as each other 
  
  PopOrderType order_type;
  
  bool compare_by_index(const int&p1, const int& p2);
  void show_occupancy();
  void calc_res_occupancy();
  void calc_home_occupancy();
  void calc_res_entry_occupancy(); // TODO
  void show_occupants(LocNode* l, ResPtr r_ptr);

  void qt_show_crops();
  void qt_show_occupancy();

  void collect_subtype(char type, vector<PerPtr>& sub_pop);
  bool collect_talkers(vector<PerPtr>& sub_pop);
  void aggregate_area_gains(char type,AreaGains &agg, string what_to_note);
  void aggregate_sched_gains(char type,vector<float> &sched_gains);
  float get_mean_energy(char type);
  float get_mean_eaten(char type);
  
   private:

  // vars used in update_by_move_and_feed for tracking things
  // don't actually influence how event processing unfolds

  float time_reached = 0.0;
  float hml = 0.0; // home-time max living
  float hmd = 0.0; // home-time max dying
  // TEMP
  float max_time_for_two = 0;
  int max_places_eaten = 0;
  int max_places_explored = 0;
  
};

extern Population pop;


#endif
