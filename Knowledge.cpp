#include "Knowledge.h"
#include "People.h"
#include "Debug.h"



Knowledge::Knowledge() {
  wipeouts.clear();

}



void Knowledge::update_time_dependent_info() {

  /***************************/
  /* concerning wipeout info */
  /***************************/
  
  // decrement all
  for(size_t i=0; i < wipeouts.size();  i++) {
    wipeouts[i].till_non_zero--;
    wipeouts[i].till_normal--;
  }

  // remove all with <= 0 
  vector<WipeoutInfo>::iterator w;
  w = wipeouts.begin();
  while(w != wipeouts.end()) {
    if(w->till_non_zero <= 0) {
      #if DEBUG
      db("losing "); db(w->tostring()); db("\n");
      #endif
      w = wipeouts.erase(w);
    }
    else {
      w++;
    }
  }

}

bool Knowledge::has_time_dependent_info() {
  if(wipeouts.size() != 0) { return true; }

  else { return false; }
}

bool Knowledge::has_wipeout_info_about(ResPtr r) {

  for(size_t i=0; i < wipeouts.size();  i++) {
    if(wipeouts[i].res_to_ignore == r) {
      return true;
    }
  }

  return false;
}

bool Knowledge::has_this_wipeout(WipeoutInfo& in) {

  for(size_t i=0; i < wipeouts.size(); i++) {
    WipeoutInfo wi = wipeouts[i];
    if((in.res_to_ignore == wi.res_to_ignore) &&
       (in.till_non_zero == wi.till_non_zero) && 
       (in.till_normal == wi.till_normal)) {
      return true;
    }
  }
  return false;
}

void Knowledge::detect_a_wipeout(ResPtr rptr, int till_non_zero_after_wipeout, int till_normal_after_wipeout) {
  WipeoutInfo wi;
  wi.res_to_ignore = rptr;
  // could set ignore_till to either supplied date
  wi.till_non_zero = till_non_zero_after_wipeout;
  wi.till_normal = till_normal_after_wipeout;
  if(who->imparts_info) { wi.impart_today = true; }
  wipeouts.push_back(wi);
  //db(who->toid()); db("learned "); db(wi.tostring()); db("\n");
  #if DEBUG
    //	      db_level = 1;
   db(who->toid()); db(" detected "); db(wi.tostring()); db("\n");
    //	      db_level = 0;
   #endif
}







string Knowledge::tostring() {
  string s = "";

  for(size_t i=0; i < wipeouts.size();  i++) {
    WipeoutInfo wi = wipeouts[i];
    s += wi.tostring(); 
    if(i != wipeouts.size()-1) { s += "\n"; }
  }

  return s;
}

