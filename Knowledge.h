#if !defined(KNOWLEDGE_h)
#define KNOWLEDGE_h

// things a Person may come to know which can be used to influence choices
#include "Grid.h"
#include "InfoTypes.h"
#include <set>
//#include "Resource.h"

class Person;
extern int gridSize;

class Knowledge {
 public:

  Person *who;
  Knowledge();
  
  /**********************/
  /* Location Based knowledge */
  /**********************/
  LocGrid internalWorld;
  std::vector<LocNode*> knownResources;

  /**********************/
  /* episodic knowledge */
  /**********************/
  
  /* can includes things which should update mechanically day by day
   * like amount of time remaining till something is due to become true
   */
  void update_time_dependent_info();

  bool has_time_dependent_info();

  /**************************/
  /* pertaining to wipeouts */
  /**************************/
  
  // roughly list of pairs (r,d) where r is a Resources area, d date until which should be ignored
  vector<WipeoutInfo> wipeouts;

  bool has_wipeout_info_about(ResPtr r); 
  
  // suppose detect_a_wipeout(..) only called if has_wipeout_info_about(..) gives false
  // so first figure out there has been wipeout, but don't want to keep learning next day and the next
  // till_non_zero_after_wipeout is num days till become non-zero after wipeout
  // till_normal_after_wipeout is num days till completely normal after wipeout

  void detect_a_wipeout(ResPtr rptr,   int till_non_zero_after_wipeout, int till_normal_after_wipeout); // adds to wipeouts

  bool has_this_wipeout(WipeoutInfo& wi);

  string tostring();

};

#endif
