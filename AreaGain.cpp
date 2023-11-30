#include "AreaGain.h"
#include "Util.h"
#include "Debug.h"
#include <iostream>
using namespace std;

AreaGain::AreaGain() {

  gain = 0.0;
  time_in_area = 0.0; 
  time_entered = 0.0;
  time_left = 0.0;

  collective = false;
  count = 0;
  gain_per_person = 0.0;
  res_ptr = NULL;

}

AreaGain::AreaGain(ResPtr r) {

  gain = 0.0;
  time_in_area = 0.0; 
  time_entered = 0.0;
  time_left = 0.0;
  collective = false;  
  count = 0;
  gain_per_person = 0.0;
  res_ptr = r;

}

void AreaGain::zero() {

  gain = 0.0; 
  time_in_area = 0.0; 
  time_entered = 0.0;
  time_left = 0.0;
  count = 0;

};

string AreaGain::tostring() {
  if(res_ptr == NULL) {
    return "NotInit";
  }
  string s;
  s = res_ptr->id;
  //s += "["; s += to_string(res_to_index[res_ptr]); s +=  "]";
  s += "(g:";
  s += f_to_s(gain);
  s += ",d:";
  s += f_to_s(time_in_area);
  if(collective) {
    s += ",c:";
    s += to_string(count);
    s += ",m:";
    s += f_to_s(gain_per_person);
  }
  s += ")";
  return s;
}

void AreaGain::show() {
  cout << tostring() << endl;
}

AreaGains::AreaGains() {
  collective = false;
}

void AreaGains::set_an_area_entry_time(ResPtr r_ptr, float time_entered){
  // assumes no info this area yet
  // would need to reconsider if can re-enter
  AreaGain g(r_ptr);
  g.time_entered = time_entered;
  g.count = 1;
  area_gains[r_ptr] = g;

}

void AreaGains::increment_an_area_gain(ResPtr r_ptr, float gain){
  map<ResPtr, AreaGain>::iterator gains_itr;
  gains_itr = area_gains.find(r_ptr);
  if(gains_itr == area_gains.end()) {
    AreaGain g(r_ptr);
    g.gain = gain;
    area_gains[r_ptr] = g;
  }
  else {
   gains_itr->second.gain += gain;
  }

}

void AreaGains::set_an_area_duration(ResPtr r_ptr, float time_left){
  // really ought to have info for this area already
  map<ResPtr, AreaGain>::iterator gains_itr;
  gains_itr = area_gains.find(r_ptr);
  if(gains_itr != area_gains.end()) {
    gains_itr->second.time_left = time_left;
    gains_itr->second.time_in_area = time_left - (gains_itr->second.time_entered);
  }
  else {
    db("really ought to have info for this area already\n");
    cout << "really ought to have info for this area already\n";   
  }

}

bool AreaGains::has_this_area(ResPtr r_ptr){
  map<ResPtr, AreaGain>::const_iterator gains_itr;
  gains_itr = area_gains.find(r_ptr);
  if(gains_itr != area_gains.end()) {
    return true;
  }
  else {
    return false; 
  }

}

void AreaGains::clear_area_gains(){
  area_gains.clear();
}


bool AreaGains::has_info() {
  if(area_gains.size() != 0) {
    return true;
  }
  else {
    return false;
  }

}


void AreaGains::show_area_gains() {
  map<ResPtr,AreaGain>::const_iterator ag_itr;
  for(ag_itr = area_gains.begin(); ag_itr != area_gains.end(); ag_itr++) {
    AreaGain ag;
    ag = ag_itr->second;
    db(" "); db(ag.tostring()); db(" ");
  }
 
}

bool AreaGains::get(ResPtr r_ptr, AreaGain &ag) {
  map<ResPtr, AreaGain>::const_iterator gains_itr;
  gains_itr = area_gains.find(r_ptr);
  if(gains_itr != area_gains.end()) {
    ag = gains_itr->second;
    return true;
  }
  else {
    return false; 
  }

}

void AreaGains::set(ResPtr r_ptr, AreaGain ag) {
  area_gains[r_ptr] = ag;

}

bool AreaGains::get_gain(ResPtr r_ptr, float &g) {
  map<ResPtr, AreaGain>::const_iterator gains_itr;
  gains_itr = area_gains.find(r_ptr);
  if(gains_itr != area_gains.end()) {
    g = gains_itr->second.gain;
    return true;
  }
  else {
    return false; 
  }


}

bool AreaGains::get_gain_per_person(ResPtr r_ptr, float &g) {
  map<ResPtr, AreaGain>::const_iterator gains_itr;
  gains_itr = area_gains.find(r_ptr);
  if(gains_itr != area_gains.end()) {
    g = gains_itr->second.gain_per_person;
    return true;
  }
  else {
    return false; 
  }


}

bool AreaGains::get_count(ResPtr r_ptr, int &c) {
  map<ResPtr, AreaGain>::const_iterator gains_itr;
  gains_itr = area_gains.find(r_ptr);
  if(gains_itr != area_gains.end()) {
    c = gains_itr->second.count;
    return true;
  }
  else {
    return false; 
  }



}


string AreaGains::tostring() {
  string s = "";
  map<ResPtr,AreaGain>::const_iterator ag_itr;
  for(ag_itr = area_gains.begin(); ag_itr != area_gains.end(); ag_itr++) {
    AreaGain ag;
    ag = ag_itr->second;
    s += ag.tostring();
    s += " "; 
  }
  return s;

}


