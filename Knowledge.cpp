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
  info->numPatches = res->resourceObject->resources.size();
  info->knownResIndex = knownResources.size() - 1;

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
  InfoRes* info = resInfo[getInfoIndex(res)];

  //add current totals for patches
  for(int i = 0; i < res->resourceObject->resources.size(); i++)
  {
    info->known_total_of_patches[i] = res->resourceObject->resources[i].get_total();
  }

  if(res->resourceObject->in_wipeout && !info->isWipeout)
  {
    info->isWipeout = true;
    info->till_non_zero = res->resourceObject->non_zero_after_wipeout;
    info->till_normal = res->resourceObject->normal_after_wipeout;
  }
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

