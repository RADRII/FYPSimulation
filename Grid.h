#if !defined(POPLOC_H)
#define POPLOC_H
#include <string>
#include <vector>
#include "Resource.h"
using namespace std;

enum NodeKind {HAB_ZONE, RESOURCE, OBSTACLE, EMPTY, UNKNOWN};

class LocNode {
 public:
  int x, y;
  int occupancy;
  NodeKind type;
  LocNode* parent;
  ResPtr resourceObject;
  LocNode(int x, int y, NodeKind type);
  string tostring();
};

class LocGrid {
 public:
  int size;
  std::vector<std::vector<LocNode>> nodes;
  LocGrid(int s, bool isPerson);
  LocGrid();
  LocNode* getNode(int x, int y);
  std::vector<LocNode*> findPath(LocNode* startNode, LocNode* endNode);
};


// globals vars

extern vector<LocNode*> all_home_loc;
extern vector<LocNode*> all_resource_loc;
extern vector<LocNode*> all_obstacle_loc;

bool res_frm_loc(LocNode* l, ResPtr& r);

#endif
