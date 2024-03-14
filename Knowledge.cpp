#include "Knowledge.h"
#include "Person.h"
#include "Debug.h"

Knowledge::Knowledge() 
{
  internalWorld = LocGrid(gridSize, true);
  internalWorld.getNode(gridSize/2, gridSize/2)->type = HAB_ZONE;
  numUnknown = 48;
}

void Knowledge::addNewResToMind(LocNode* res)
{
  //Add to knownres
  knownResources.push_back(res);

  //Create info about res
  InfoRes* info = new InfoRes();
  info->numPatches = res->resourceObject->resources.size();
  //add current totals for patches
  for(int i = 0; i < res->resourceObject->resources.size(); i++)
  {
    info->known_total_of_patches.push_back(res->resourceObject->resources[i].get_total());
  }

  // Add to info vector
  resInfo.push_back(info);
}

//todo
void Knowledge::updateInfoRes(LocNode* res)
{

}

//todo
string Knowledge::tostring() 
{
  string s = "";
  return s;
}

