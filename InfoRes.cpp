#include "InfoRes.h"
#include "Util.h"
#include <iostream>
using namespace std;

InfoRes::InfoRes()
{
    knownResIndex = -1;
    isWipeout = false;
    isNotNormal = false;
    wipeOutOrig = SENSES;
    till_non_zero = -1;
    till_normal = -1; 
    originalSize = -1;
    plentyOrig = SENSES;
    till_non_plenty = -1;
}

void InfoRes::dailyDateCheck(int date)
{
  if(isWipeout && date >= till_non_zero)
  {
    isWipeout = false;
    isNotNormal = true;
  }
  else if(isNotNormal && date >= till_normal)
  {
    isNotNormal = false;
    till_non_zero = -1;
    till_normal = -1;
  }
  else if(isPlenty && date >= till_non_plenty)
  {
    isPlenty = false;
    till_non_plenty = -1;
    known_total_of_patches.resize(originalSize);
  }
}

string InfoRes::tostring() {
  string s = "";
  if(isWipeout == true)
  {
    s = "WIPEOUT: ";
    s += " till non_zero:"; s += to_string(till_non_zero);
    s += " till normal:";  s += to_string(till_normal);
    if(wipeOutOrig == SENSES) { s += " orig:senses"; }
    else { s += " orig:talking"; }
  }
  else if(isPlenty == true)
  {
    s = "PLENTY: ";
    s += " til normal:";  s += to_string(till_non_plenty);
    if(plentyOrig == SENSES) { s += " orig:senses"; }
    else { s += " orig:talking"; }
  }

  return s;
}







