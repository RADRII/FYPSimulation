#if !defined(KNOWLEDGE_h)
#define KNOWLEDGE_h

// things a Person may come to know which can be used to influence choices
#include "Grid.h"
#include "InfoRes.h"
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
  int numUnknown;

  /**********************/
  /* Knowledge on each Resource */
  /**********************/
  

  string tostring();

};

#endif
