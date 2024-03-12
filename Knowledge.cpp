#include "Knowledge.h"
#include "Person.h"
#include "Debug.h"

Knowledge::Knowledge() {
  internalWorld = LocGrid(gridSize, true);
  internalWorld.getNode(gridSize/2, gridSize/2)->type = HAB_ZONE;
}

//todo
string Knowledge::tostring() {
  string s = "";
  return s;
}

