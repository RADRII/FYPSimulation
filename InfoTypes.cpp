#include "InfoTypes.h"
#include "Util.h"
#include <iostream>
using namespace std;

WipeoutInfo::WipeoutInfo(){
  res_to_ignore = NULL;
  till_non_zero = -1;
  till_normal = -1;
  impart_today = false;
  orig = SENSES;
}

string WipeoutInfo::tostring() {
  string s = "";
  if(res_to_ignore == NULL) {
    s = "not valid";
  }
  else {
   s += res_to_ignore->id;
   s += " till non_zero:"; s += to_string(till_non_zero);
   s += " till normal:";  s += to_string(till_normal);
   if(orig == SENSES) { s += " orig:senses"; }
   else { s += " orig:talking"; }

  }

  return s;
}







