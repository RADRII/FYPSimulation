#if !defined(POPLOC_H)
#define POPLOC_H
#include <string>
#include <vector>
#include "Resource.h"
using namespace std;

enum NodeKind {HAB_ZONE, RESOURCE, OBSTACLE, EMPTY, UNKNOWN, START};

class LocNode {
 public:
  int x, y;
  int occupancy;
  NodeKind type;
  LocNode* parent;
  ResPtr resourceObject;
  LocNode(int x, int y, NodeKind type);
  string tostring();
  bool equals(LocNode*);
};

class LocGrid {
 public:
  int size;
  std::vector<std::vector<LocNode>> nodes;
  LocGrid(int s, bool isPerson);
  LocGrid();
  LocNode* getNode(int x, int y);
  vector <LocNode*> getUnexploredNeighbors(LocNode* loc);
  void resetParents();
  std::vector<LocNode*> findPath(LocNode* startNode, LocNode* endNode);
  std::vector<LocNode*> findPathClosestUnexplored(LocNode* startNode);
};


// globals vars

extern vector<LocNode*> all_home_loc;
extern vector<LocNode*> all_resource_loc;
extern vector<LocNode*> all_obstacle_loc;

bool res_frm_loc(LocNode* l, ResPtr& r);

#endif
