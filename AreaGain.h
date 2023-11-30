#if !defined(AREA_GAIN_h)
#define AREA_GAIN_h

#include "Resource.h"

class AreaGain { // record for one area of what was eaten and how long it took, could be personal or collective

 public:
  AreaGain();
  AreaGain(ResPtr r);
  
  ResPtr res_ptr; // which Resources area 

  float gain; // how much energy gained from this area
  float time_in_area; // how long in area
  void zero(); // set all to zero
  float time_entered;
  float time_left;
  int count; // how many people were involved
  float gain_per_person;
  bool collective;
  string tostring();
  void show();
  // possibly add show methods
  
};

class AreaGains { // record for all areas of what was eaten and how long it took, personal or collective
  
 public:
  AreaGains();
  void set_an_area_entry_time(ResPtr r_ptr, float entry_time);
  void increment_an_area_gain(ResPtr r_ptr, float gain); 
  void set_an_area_duration(ResPtr r_ptr, float end_time);
  bool has_this_area(ResPtr r_ptr);
  void clear_area_gains();
  void show_area_gains();
  bool get(ResPtr r_ptr, AreaGain &ag);
  void set(ResPtr r_ptr, AreaGain g);
  bool get_gain(ResPtr r_ptr, float &g);
  bool get_count(ResPtr r_ptr, int &g);
  bool get_gain_per_person(ResPtr r_ptr, float &g);
  bool collective;
  bool has_info();
  string tostring();
  map<ResPtr,AreaGain> area_gains;
  
 private:



};




#endif
