#include "Knowledge.h"
#include "Person.h"
#include "Debug.h"
#include <iostream>

using namespace std;

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
  info->knownResIndex = knownResources.size() - 1;
  info->originalSize = 4; //TODO this is hardcoded because I can't be bothered right now. Figure out a smarter way.

  // Add to info vector
  resInfo.push_back(info);

  //Call update info res
  updateInfoRes(res);
}

//todo
void Knowledge::updateInfoRes(LocNode* res)
{
  InfoRes* info = resInfo[getInfoIndex(res)];

  //clear and add current totals for patches
  info->known_total_of_patches.clear();
  for(int i = 0; i < res->resourceObject->resources.size(); i++)
  {
    info->known_total_of_patches.push_back(res->resourceObject->resources[i].get_total());
  }

  //plenty check
  if(res->resourceObject->in_plenty && !info->isPlenty)
  {
    info->isPlenty = true;
    info->till_non_plenty = res->resourceObject->normal_after_plenty;
    info->plentyOrig = SENSES;
  }

  //wipeout check
  if(res->resourceObject->in_wipeout && !info->isWipeout)
  {
    info->isWipeout = true;
    info->till_non_zero = res->resourceObject->non_zero_after_wipeout;
    info->till_normal = res->resourceObject->normal_after_wipeout;
    info->wipeOutOrig = SENSES;
  }
}

//return index of knowledge that it wants info on
//-1 if there is nothing to be shared
int Knowledge::needsKnowledgeOn(LocNode* res, bool wipeout, bool plenty)
{
  for(int i = 0; i < knownResources.size(); i++)
  {
    if(knownResources[i]->equals(res))
    {
      if((wipeout && !resInfo[i]->isWipeout) || (plenty && !resInfo[i]->isPlenty))
      {
        return i;
      }
    }
  }

  return -1;
}

int Knowledge::getInfoIndex(LocNode* res)
{
  for(int i = 0; i < knownResources.size(); i++)
  {
    if(knownResources[i]->equals(res))
      return i;
  }

  cout << "Warning: Somehow trying to update non-known resource." << endl;
  return -1;
}

void Knowledge::dailyBoolUpdate(int date)
{
  for(int i = 0; i < resInfo.size(); i++)
  {
    resInfo[i]->dailyDateCheck(date);
  }
}

string Knowledge::tostring() 
{
  string s = "";
  for(int x = 0; x < gridSize; x++)
  {
    for(int y = 0; y < gridSize; y++)
    {
      s += to_string(x) + " " + to_string(y);
      s += " type:" + to_string(internalWorld.getNode(x,y)->type) + " | ";
    }
    s += "\n";
  }
  for(int i = 0; i < resInfo.size(); i++)
  {
    if(resInfo[i]->isWipeout)
    {
      s += "WIPEOUT " + to_string(knownResources[i]->x) + " " + to_string(knownResources[i]->y) + "\n";
    }
    if(resInfo[i]->isPlenty)
    {
      s += "PLENTY " + to_string(knownResources[i]->x) + " " + to_string(knownResources[i]->y) + "\n";
    }
  }
  return s;
}

