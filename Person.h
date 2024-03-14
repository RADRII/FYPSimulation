#if !defined(PERSON_h)
#define PERSON_h

#include <string>
#include <vector>
#include "Resource.h"
#include "AreaGain.h"
#include "Action.h"
#include "Knowledge.h"
using namespace std;

class FamilyPlan {
 public:
  FamilyPlan();
  int whose_plan;
  int planned_offspring;
  void choose_planned_offspring();

  int birth_age_index;
  vector<int> birth_ages;
  void choose_birth_ages(int age, int maxReproAge);
  
  int next_birth_age;
  void set_plan(int age, int who, int maxReproAge);
  void show();
};

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

  //For action planning
  float curiosity;
  int homeByTime;

  // ENERGY
  int init_energy; // 'energy' at birth

  int max_energy; // how much can energy can be stored up at most

  int sleepEnergyLoss; // how much energy is lost between days

  int moveCost; //how much energy does moving cost;
  
  int max_daily_eat; // cannot cram more food in (as energy) in a single day than this in
  // NB: max_energy could be 15 and this could be 5 so *less*
  // --> would take >1 day to reach max

  // REPRO
  int repro_age_start;
  int repro_age_end;

  // MOVEMENT
  int speed; //typically always 1, might remove
  
  /*******************************/
  /* end of FIXED attributes     */
  /*******************************/
  
  int age; // current 'age': the number of simulation 'days' they have been alive

  // relating to ENERGY in and out 
  
  int current_energy; // 'energy', based on food eaten; assume alive unless this 0
  
  int eaten_today; // amount (as energy) added so far during a day

  int eat_from_dry_run(CropPatch& c);
  // calc what energy would happen if eating up to relevant energy limits from patch c
  // -- will not cause current_energy to grow too large if added
  //  -- will not be more than max_daily_eat

  bool hasMaxEnergy();
  // checks whether Person has reached relevant energy limits (max_energy or max_daily_eat)
  // and so will stop seeking food

  bool hasBeenEating; // indicates that Person is actively consuming food from a particular patch
                   
  CropPatch *eating_patch; // the patch they are eating from
                           // NB: this should be consistent with the patch's being_eaten
  

  // relating to REPRO 

  int num_offspring;
  void set_frm_parent(Person *parent);
  FamilyPlan fam_plan;
  
  // relating to Location

  LocNode* loc; // where the Person is

  /**********************************************************/
  /* following relating to routing */
  /**********************************************************/
  bool isHeadingHome;
  vector<LocNode*> route;
  bool clear_route();
  size_t route_index;
  void show_route();
  string route_tostring();

  /**********************************************************/
  /* following relating to planning actions */
  /**********************************************************/
  ActionKind prevAction;
  bool atResource;
  bool isHome;
  int currentTic;
  vector<LocNode*> knownResources;
  double energyExploreAbove;
  double exploreBoundary;

  ActionPtr getNextAction(bool failedEat);
  bool setResourceRoute();

  /**********************************************************/
  /* following relating to updating knowledge */
  /**********************************************************/
  Knowledge mind;
  LocNode* home_loc;
  bool getRoute(vector<LocNode*> internalRoute);
  

  /*************************************/
  /* relating to display of attributes */
  /*************************************/
  string toid();
  void show();
  void show_var_energy(); // called by show for 'variable' energy attributes

  string info_type_to_string();
  void info_type_show();

  // used for stats and knowledge
  AreaGains area_gains;
  
  // used only for compiling stats
  vector<ResPtr> resEatenAt;
  void update_places_eaten(ResPtr);
  void clear_places_eaten();
  int num_places_eaten;
  void update_places_explored(LocNode* l);
  void clear_places_explored();
  int num_places_explored;
  int homeTime;
  
 private:
  void show_home_time();
  void show_num_places();


  int eat_from(CropPatch& c);
  // determine energy update via consuming food from c
  // note: does not update the Person 
  // note: though this updates c, it is only called by eat_from_dry_run(..)
  // on a *copy* of an actual CropPatch
  // updates of actual CropPatchs done by CropPatch::remove_units(..) 
};

// bool compare_person(const Person& p1, const Person& p2);
bool compare_person(Person *p1, Person *p2);

class Population {
 public:
  Population();
  Population(string id, int size);
  void add(Population& p);
  string id;

  int currentTic;
  int hbt;

  void updatePeopleTic(int tic);
  void resetDayBools();

  vector<Person *> population;
  bool update(int date); // top-level 'update' function which calls series of further update_... functions
                         // to eg. let die those too old, those with not enough energy
                         //        expend energy and try to eat
                         //        add new population members


  void update_by_cull(int& deaths_age, int& deaths_starve, int& deaths_strand); // age, expend energy, then cull,
  //TODO add update by wipeouts back in (after add communication)
  
  int update_by_repro(); // add new population members, set num to number born
  
  void update_by_action(int date, int tic);

  void RouteAction_proc(RouteAction *route_ptr, int tic);
  void EatAction_proc(EatAction *eat_ptr,ActionList& list, int& date, int tic);
  void ExploreAction_proc(ExploreAction *explore_ptr,ActionList& list, int tic);
  void HomeAction_proc(HomeAction *home_ptr, int tic);
  void WaitAction_proc(WaitAction *wait_ptr, int tic);

  void update_by_redistrib(); // make all equal
  
  void show();
  void zero_eaten_today();
  int get_total();

  vector<Tribe> tribes;
  
  bool compare_by_index(const int&p1, const int& p2);
  void show_occupancy();
  void show_occupants(LocNode* l);

  void collect_subtype(char type, vector<PerPtr>& sub_pop);

  float get_mean_energy(char type);
  float get_mean_eaten(char type);

  int max_places_eaten = 0;
  int max_places_explored;
};

extern Population pop;
extern LocGrid world;

#endif
