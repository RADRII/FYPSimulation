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

//takes in communication and updates internal knowledge
void Knowledge::receiveCommunication(int index, InfoRes* info)
{
  resInfo[index]->known_total_of_patches.clear();
  for(int i = 0; i < info->known_total_of_patches.size(); i++)
  {
    resInfo[index]->known_total_of_patches.push_back(info->known_total_of_patches[i]);
  }

  if(info->isWipeout)
  {
    resInfo[index]->isWipeout = true;
    resInfo[index]->till_non_zero = info->till_non_zero;
    resInfo[index]->till_normal = info->till_normal;
    resInfo[index]->wipeOutOrig = TALKING;
  }
  else if(info->isPlenty)
  {
    resInfo[index]->isPlenty = true;
    resInfo[index]->till_non_plenty = info->till_non_plenty;
    resInfo[index]->originalSize = info->originalSize;
    resInfo[index]->plentyOrig = TALKING;
  }

  return;
}

//takes in communication and updates internal knowledge
void Knowledge::receiveCommunication(LocNode* res, InfoRes* newInfo)
{
  knownResources.push_back(res);

  //Create info about res from taken info
  InfoRes* info = new InfoRes();
  info->knownResIndex = knownResources.size() - 1;
  info->originalSize = 4; //TODO this is hardcoded because I can't be bothered right now. Figure out a smarter way.

  //setup info
  for(int i = 0; i < newInfo->known_total_of_patches.size(); i++)
  {
    info->known_total_of_patches.push_back(newInfo->known_total_of_patches[i]);
  }

  if(newInfo->isWipeout)
  {
    info->isWipeout = true;
    info->till_non_zero = newInfo->till_non_zero;
    info->till_normal = newInfo->till_normal;
    info->wipeOutOrig = TALKING;
  }
  else if(info->isPlenty)
  {
    info->isPlenty = true;
    info->till_non_plenty = newInfo->till_non_plenty;
    info->plentyOrig = TALKING;
  }

  // Add to info vector
  resInfo.push_back(info);

  //update internal world
  internalWorld.getNode(res->x, res->y)->type = RESOURCE;
  numUnknown = numUnknown - 1;

  return;
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

