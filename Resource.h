#if !defined(RESOURCE_h)
#define RESOURCE_h

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <utility>
#include "Location.h"
using namespace std;

// part of a CropPatch
class AgeBand {
public:
  int amount;

  int ripen;
  // day on which first ripened,  counting from beginning of its 'season'
  // eg. this part of a CropPatch whose 'profile' is length 7
  // 'ripen' must be in 0 ... 6

};

class Person; // forward declaration

/*****************************************************************************************/
/* could be imagined to be a bush with berries on it, or some ground with beans growing  */
/* treated as all at a single location. In the eating process such a single CropPatch is */
/* monopolised by a single Person at a time.                                             */
/* a Resources area will essentially be a collection of these                            */
/*****************************************************************************************/
class CropPatch {
 public:
  CropPatch();
  CropPatch(int day);

  string name; // nuts, berries, etc

  int patch_yield; // total that this patch will yield
    
  vector<float> profile;
  int lasts;
  int season_progress;
  int abs_init_day;
  int next_init_day;
  int period; 
  char sym; // how to display eg. '*' or 'o'
  Location pos;

  
  // 'profile' represents a growing 'season'
  // different parts of the patch become edible in stages
  // profile is a vector such that profile[n] = amount of patch_yield
  // that ripens at the n-th stage 
  // eg. {0.05, 0.1, 0.2, 0.3, 0.2, 0.1, 0.05}
  // in this case there would be 7 stages
  
  // 'lasts': num of days that a given part remains edible
  // combining above profile with lasts==4 gives the total number of days on which food
  // from this patch will be edible as 10
  //
  // 'season_progress': tracks how many days elapsed since first part of patch became edible
  // keeps incrementing whilst some part of patch is edible
  // rest of the time is -ve
  
  // 'period' process should repeat and this is num days between repeats
  // eg between successive instances of profile[0] amount of the crop become first edible
  
  // 'next_init_day': whilst food available from patch this is the day when that started
  // whilst no food edible from the patch this gives day when the first part of the patch
  // becomes available 
  // 
  // 'abs_init_day' day when it kicks off for every first time


  deque<AgeBand> bands;

  // it is handy to sort what is currently edible into 'age' bands
  // arranged in a queue, oldest at front, youngest at back, of length <= 'lasts'
  // should disappears from (front of) this after been around for 'lasts' days
  // eg. 
  // (oldest at left)
  // 1(5) , max: 5 (+5)
  // 2(5) 1(10)  , max: 15 (+10)
  // 3(5) 2(10) 1(20) , max: 35 (+20)
  // 4(5) 3(10) 2(20) 1(30)  , max: 65 (+30)
  // 4(10) 3(20) 2(30) 1(20) , max: 80 (-5 +20)
  // 4(20) 3(30) 2(20) 1(10) , max: 80 (-10 +10) 
  // 4(30) 3(20) 2(10) 1(5) , max: 65 (-20 +5)
  // 4(20) 3(10) 2(5) , max: 35 (-30)
  // 4(10) 3(5) , max: 15 (-20)
  // 4(5) , max: 5 (-10)  
  // but is *not* handy to actually store the 'age' in the bands
  // eg. 'age' at front has to go up as stuff queued at back
  // eg. 'age' of a band has to go up as it moves foward
  // also not possible to deduce 'age' from position in queue 
  // eg. at start front of queue = 1, at end front of queue = 4
  // also do not really need to know 'age'
  // so instead a band stores when it ripened (position in profile)
  // ==> and deduce 'age' from season_progress - b.ripen
  
  int get_age_of_band(AgeBand& b);
  int get_total(); // max edible at current point: sum over bands

  void show_bands(); // shows amounts in bands and a total eg. 3(11) 2(15) 1(15) 41
  void show_bars(); // shows repeats of the sym to indicate the total


  int update_at_date(int); // update the bands at given day, inserting new if any,
  // deleting inedible due to age if any, returns amount of update

  /* above relate to biological growth, following relate to eating processes */
  
  float energy_conv; // to convert from units of this crop to energy units
                     // so potentially diff crops can provide more energy
                     // (potentially later have graduated levels of ripening and so of energy yield)
 
  bool being_eaten;  // switched on and off according to whether or not a Person is currently
  // consuming food at this patch; see Population::update_by_move_and_feed

  Person *eater; // the person doing the eating
  
  // Person::eat_frm_dry_run() carefully determines number of units which a Person is going to eat,
  // this function then implements whole state change in one go
  void remove_units(int units); 

  bool wipeout_at_date(int d, int& normal_after_wipeout, int& non_zero_after_wipeout);
  // a sudden catastrophic failure (fire, mudslide, ...) causing patch to flat-line its output
  // from d to end of its season. will start up again when next season is due
  // non_zero_after_wipeout set to be first day this patch will once more provide non-zero units
  // normal_after_wipeout set to be first day on which effect of the wipeout on this patch disappears
  // ie. end of its 'season'
  // empties bands, sets other indicators as if season over and waiting to restart
  // returns true if this 'wipeout' actually changes things
  // eg. if in the ordinary course of things there is an attempt to wipeout on one of the patch's
  // 'dormant' days after last day of possible food availability and before first day of food avail
  // then it does not really do anything
  // eg. if already been wiped-out at some non-dormant day, that will have prematurely switched it to
  // a dormant date, and again nothing really happens
  // NB: currently calling of this is via a Resources area which contains the CropPatchs
 
  
};

/****************************************************************/
/* this is to represent a distinct 'area' containing CropPatchs */
/* in a succession of nearby locations belonging to the area    */
/* foraging is at one level a matter of travel between Resources areas */
/* at a lower level is a matter of travel between CropPatchs in an area */
/************************************************************************/
// collection of CropPatches
class Resources {
public:
  Resources();
  //Resources(string name, LocPtr res_entry, int patch_yield);
  //Resources(string name, LocPtr res_entry, int patch_yield, int patch_rep);
  //Resources(string name, LocPtr res_entry, int patch_yield, int patch_rep, int loc_rep);
  Resources(string name, LocPtr res_entry, int patch_yield, float energy_conv, int patch_rep, int loc_rep);
  string id;
  string tostring();
  
  vector<CropPatch> resources;
  vector<Location> locs; // list the locations of all the CropPatch's in the resources vec
                         // note its non-repeating, tho several patches can share a location
  void set_locs(); // sets up the above

  // these run functions of same name over its CropPatch's
  void show_bands(); 
  void show_bars();
  
  int get_total(); // total via function of same name over all the units of the contained CropPatches

  int get_total(Location l); // total from the patches at location
  void show_total();
  
  int update_at_date(int); // does update by function of same name over all the contained CropPatches
  // returns the total amount of update
  
  void  wipeout_at_date(int d);   // does 'wipeout' on all contained CropPatches
  // this is a catastropic failure of a whole area, eg a fire or flash-flood that
  // sweeps thru whole area
  int non_zero_after_wipeout; // after a wipeout the first day non-zero food again
  int normal_after_wipeout; // after a wipeout the first day normal food amount again

  bool in_wipeout; // true if after a wipeout and not reached day of 'normal_after_wipeout'
  double p_wipeout; // 'daily' prob of the area having a wipeout ie. running wipeout_at_date(..)

  /***************************************/
  /* relating to EATING processes */
  /***************************************/
  
  // get non-empty patches which are also not being_eaten at the given location
  bool patches_at_location(vector<int>& patches, Location *pos);
  
  // pick a random one from given list of patches
  int choose_rand_patch(vector<int> &patches);
  
  // helper vars used by above (which uses GSL shuffle function which uses an array)
  int *crop_ordering; // pointer to array containing random shuffle of 0 .. N-1 
  int crop_ordering_size; // size of array of shufffled numbers
  
  bool being_eaten_patches_at_location(Location *pos);
  // checks whether any patches are being_eaten at given location
  // used to see the location should be added to someone's revisit list


  // the Resources constructor is passed an 'entry' location
  // this function gives that location 
  bool get_res_entry(LocPtr& e);

  // from a location to the indices of the CropPatchs in resources
  // having that location
  map<Location,vector<size_t>> loc_to_indices;
  void set_loc_to_indices();
  
  
private:
  // defaults relating to the patches which are contained
  string def_name_start;
  int def_patch_yield;
  int def_lasts;
  int def_abs_init_day;
  int def_period;
  vector<float> def_profile;
  char def_sym;
  float def_energy_conv;
  int def_loc_rep;
  int def_patch_rep;
  
};


void test_Resources();

typedef Resources * ResPtr;

// all the Resources areas
extern vector<ResPtr> all_res;

// map from a Location to the Resources area it is part of
// NB: not all Locations should map to a Resources area at all
// NB: currently code in 'world_setup.cpp' sets this map only on
// first of an area's Locations
extern map<LocPtr, ResPtr> loc_to_res;



bool res_frm_loc(LocPtr l, ResPtr& r);
void show_all_loc_to_res();

extern map<ResPtr,size_t> res_to_index;

#endif
