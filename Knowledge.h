#if !defined(KNOWLEDGE_h)
#define KNOWLEDGE_h

// things a Person may come to know which can be used to influence choices
#include "Grid.h"
#include "InfoRes.h"
#include <set>

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
  int numUnknown;

  /**********************/
  /* Knowledge on each Resource */
  /**********************/
  std::vector<LocNode*> knownResources;
  std::vector<InfoRes*> resInfo; //shares index with knownresources, so info at index 0 will be for the resource at index 0 of knownresources
  void addNewResToMind(LocNode* res);
  void updateInfoRes(LocNode* res);
  int getInfoIndex(LocNode* res);

  /**********************/
  /* Communication */
  /**********************/
  int needsKnowledgeOn(LocNode* res, bool wipeout, bool plenty);
  void receiveCommunication(int index, InfoRes* info);

  /**********************/
  /* Wipeout and Plentys */
  /**********************/
  void dailyBoolUpdate(int date);

  string tostring();

};

#endif
