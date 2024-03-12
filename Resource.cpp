#include "Resource.h"
#include "Util.h"
#include "Debug.h"
#include <iostream>

using namespace std;

CropPatch::CropPatch() {
  next_init_day = abs_init_day;
  bands.clear();
}

CropPatch::CropPatch(int day) {
  abs_init_day = day;
  next_init_day = abs_init_day;
  bands.clear();
  being_eaten = false;
  
}

// max edible at i after first day of season, bearing in mind how long things last
int CropPatch::get_total() {

  std::deque<AgeBand>::const_iterator it = bands.begin();

  int total = 0;
  AgeBand b;

  while (it != bands.end()) {
    b = *it;
    total += b.amount; 
    it++;
  }
  return total;
  
}

// update the bands at given day, inserting new if any, deleting inedible due to age if any
int CropPatch::update_at_date(int date) { 

  float incr = 0.0;
  AgeBand b;
  if(date < next_init_day) {
    return 0.0;
  }
  else if((date - next_init_day) < (profile.size() + lasts - 1)) { // days of non-zero
    int d;
    d = date - next_init_day;
    
    b.ripen = d;  b.amount = profile[d] * patch_yield;
  
    if(d == 0) { // first day of new growth
      bands.push_back(b);
      incr = b.amount;
      season_progress++;

    }
    else if(d > 0 && d < profile.size()) { // further days of new growth
      bands.push_back(b);
      incr = b.amount;

      if(bands.size() == (lasts + 1)) {
	bands.pop_front(); // drop last band cos must be too old
      }
      season_progress++;

    }
    else { // nothing new, just stuff getting older
      incr = 0.0;
      // note this check against lasts not lasts+1 works cos not yet incremented season_progress
      if(!bands.empty() && (get_age_of_band(bands.front()) == (lasts))) {
	bands.pop_front();
      }
      season_progress++;
    }
  } 
  else { // thats a 'crop' over
    while(!bands.empty()) {
      bands.pop_front();
    }
    next_init_day += period;
    season_progress = -1;
    incr = 0.0; 

  }


  return incr;

}

// to flat-line its output from d to end of its season
bool CropPatch::wipeout_at_date(int date, int& normal_after_wipeout, int& non_zero_after_wipeout) {

  if(date < next_init_day) { // 'dormant' and so cannot be wiped out
    // leave normal_after_wipeout, non_zero_after_wipeout unchanged
    non_zero_after_wipeout = next_init_day;
    return false;
  }
  else if((date - next_init_day) < (profile.size() + lasts - 1)) { // days  of non-zero
    while(!bands.empty()) {
      bands.pop_front();
    }
    // as update not called yet, season_progress not incremented
    // also profile.size() + lasts is 1 more than entire number of days food avail
    normal_after_wipeout = date + (profile.size() + (lasts-1)) - (season_progress+1);
    next_init_day += period;
    non_zero_after_wipeout = next_init_day;
    season_progress = -1;
    return true;
  }
  else { // could be first day of normal dormant period; until update_at_date(..) has run
         // necessary updates to next_init_day, season_progress wont have happened

    non_zero_after_wipeout = next_init_day + period;
    return false;
  }

}

// complementary to Person::eat_frm_dry_run which carefully determines number of units which
// a Person is going to eat, incrementally taking into account their energy maxima
// this function blindly does a removal the given number of units
void CropPatch::remove_units(int units) {
  int rem_units;
  rem_units = units;
  int units_frm_band;
  deque<AgeBand>::iterator f = bands.begin();
  while(bands.size() > 0 && rem_units > 0) {
    // try to consume m_int from current band
    if(f->amount > rem_units) { // eg. have ooooo, rem_units = ooo: eating will end here
      f->amount -= rem_units;
      rem_units = 0;
    }
    else if (f->amount <= rem_units) { // eg. have have oo, rem_units = ooo: eating will continue
      units_frm_band = f->amount;
      f++; // move band iterator
      bands.pop_front(); 
      rem_units -= units_frm_band;
    }
  
  }


}

// seems has to be deque to have an iterator
void CropPatch::show_bands() {
  std::deque<AgeBand>::const_iterator it = bands.begin();

  AgeBand b;

  if(bands.size() == 0) {
    db("none\n");
  }
  else {
    while (it != bands.end()) {
      b = *it;
      db(get_age_of_band(b)); db("("); db(b.amount); db(")") ; db(' '); 
      it++;
    
    }
    db(get_total()); db("\n");

  }
}

int CropPatch::get_age_of_band(AgeBand& b) {
  return season_progress - b.ripen + 1; // so smallest 'age' is 1
}


void CropPatch::show_bars() {
  int tot;
  tot = get_total();
  if(tot == 0) { db(""); return; }
  for(int i=0; i < get_total(); i++) {
    //cout << "*";
    db(sym);
  }

  db("("); db(get_total()); db(")");

}

Resources::Resources() {}

Resources::Resources(int idd, int xx, int yy, int patch_yield, float energy_conv, int patch_rep) {
  id = idd;
  setup_record << "Resource " << id << endl;

  x = xx;
  y = yy;

  numWaiters = 0;
  
  in_wipeout = false;
  //p_wipeout = 0.01;
  //  p_wipeout = 0.10;
  p_wipeout = 0.00;
  setup_record << "p_wipeout:" << p_wipeout << endl;
  normal_after_wipeout = 0; 
  non_zero_after_wipeout = 0;
  // BEGIN block to automate
  // for berries
  def_name_start = "berries";
  def_patch_yield = patch_yield;

  def_lasts = 4;
  def_abs_init_day = 4;
  def_period = 20;
  def_profile = {0.02, 0.03, 0.04, 0.06, 0.08, 0.12,  0.15, 0.15,   0.12, 0.08, 0.06, 0.04, 0.03, 0.02};
  def_sym = '*';

  def_energy_conv = energy_conv;

  def_patch_rep = patch_rep;

  /***********************************/
  /* this makes the berries patches */
  /***********************************/
  for(int pc=0; pc < def_patch_rep; pc++) {
    CropPatch c(def_abs_init_day);
    c.name = def_name_start + to_string(pc);
    c.profile = def_profile; 
    c.patch_yield = def_patch_yield;
    c.lasts = def_lasts;
    c.energy_conv = def_energy_conv;
    c.period = def_period;
    c.season_progress = -1; // ? part of constructor ?
    c.sym = def_sym;
    resources.push_back(c);
  }
  
  setup_record << "has " << def_name_start;
  setup_record << " def_abs_init_day:" << def_abs_init_day;
  setup_record << " def_profile:";
  for(size_t i = 0; i < def_profile.size(); i++) {
    setup_record << def_profile[i];
    if(i != def_profile.size()-1) { setup_record << ","; }
  }
  setup_record << " def_patch_yield:" << def_patch_yield;
  setup_record << " def_lasts:" <<  def_lasts;
  setup_record << " def_energy_conv:" << def_energy_conv;
  setup_record << " def_period:" <<  def_period;
  setup_record << " def_sym:" << def_sym;
  setup_record << " x sep:" << 2;
  setup_record << " y sep:" << 0;
  setup_record << " has " <<  def_patch_rep << " copies at each point";
  setup_record << endl;

  // END block (for berries)
  
  // BEGIN block to automate for beans: mostly re-uses the vars for berries

  def_name_start = "beans";
  def_sym = 'o';
  def_abs_init_day = 14;

  /*******************************/
  /* the makes the beans patches */
  /*******************************/

  for(int pc=0; pc < def_patch_rep; pc++) {
    CropPatch c(def_abs_init_day);
    c.name = def_name_start + to_string(pc);
    char suffix = 'a' + pc;  // add 'a' for first, then 'b' for second etc
    c.name += suffix; 
    c.profile = def_profile; 
    c.patch_yield = def_patch_yield;
    c.lasts = def_lasts;
    c.energy_conv = def_energy_conv;
    c.period = def_period;
    c.season_progress = -1; // ? part of constructor ?
    c.sym = def_sym;
    resources.push_back(c);
  }

  setup_record << "has " << def_name_start;
  setup_record << " def_abs_init_day:" << def_abs_init_day;
  setup_record << " def_profile:";
  for(size_t i = 0; i < def_profile.size(); i++) {
    setup_record << def_profile[i];
    if(i != def_profile.size()-1) { setup_record << ","; }
  }
  setup_record << " def_patch_yield:" << def_patch_yield;
  setup_record << " def_lasts:" <<  def_lasts;
  setup_record << " def_energy_conv:" << def_energy_conv;
  setup_record << " def_period:" <<  def_period;
  setup_record << " def_sym:" << def_sym;
  setup_record << " x sep:" << 2;
  setup_record << " y sep:" << 0;
  setup_record << " has " <<  def_patch_rep << " copies at each point";
  setup_record << endl;
}

void Resources::show_bands() {

  // shows by crop type, by location
  for(int i = 0; i < resources.size(); i++) {
    db(resources[i].sym); db(":");
    resources[i].show_bands();
  }
  // eg. 
  // *c[2,y]:4(1) 1
  // *c[4,y]:4(1) 1
  // *c[6,y]:4(1) 1
  // oc[2,y]:4(3) 3(4) 2(6) 1(7) 20
  // oc[4,y]:4(3) 3(4) 2(6) 1(7) 20
  // oc[6,y]:4(3) 3(4) 2(6) 1(7) 20

  
  // for(size_t i=0; i < locs.size(); i++) {
  //   vector<size_t> patches;
  //   patches = loc_to_indices[locs[i]];
  //   for(size_t p=0; p < patches.size(); p++) {
  //     CropPatch c;
  //     c = resources[patches[p]];
  //     db(c.sym); db(c.pos.tostring()); db(":");
  //     c.show_bands();
  //   }
  // }

  // shows by location, by crop type
  // eg.
  // *c[2,y]:4(1) 1
  // oc[2,y]:4(3) 3(4) 2(6) 1(7) 20
  // *c[4,y]:4(1) 1
  // oc[4,y]:4(3) 3(4) 2(6) 1(7) 20
  // *c[6,y]:4(1) 1
  // oc[6,y]:4(3) 3(4) 2(6) 1(7) 20
}

void Resources::show_bars() {
 for(int i = 0; i < resources.size(); i++) {
    resources[i].show_bars();
    db(" | ");
  }
 // cout << endl;


}

int Resources::get_total() { // max edible from all CropPatches in this Resource area
  int tot = 0;
  for(int i = 0; i < resources.size(); i++) {
    tot += resources[i].get_total();
  }
  return tot;
}

void Resources::show_total() {
  db(get_total()); db("["); db(get_total()); db("]"); 
}

// update all CropPatches at given date (which inserting new if any, deleting inedible due to age if any
// one 'update' possibility is wipe out
int Resources::update_at_date(int date) {

  if(in_wipeout && date == non_zero_after_wipeout) {
    in_wipeout = false;
  }

  if(gsl_ran_bernoulli(r_global, p_wipeout)) {
    wipeout_at_date(date);
    in_wipeout = true;
  }
  
  double incr = 0.0;
   for(size_t i = 0; i < resources.size(); i++) {
     incr += resources[i].update_at_date(date);
  }
   return incr;
}

// flat-line all contained CropPatches
// represents something like fire spreading throughout a Resources area
// 'wiping' everything out. It will return at start of its next 'season'
void Resources::wipeout_at_date(int date) {
  int new_normal_after_wipeout = 0;
  

  bool area_wipeout = false;
  for(size_t i = 0; i < resources.size(); i++) {
    int poss_normal, poss_non_zero;
    bool patch_wipeout = false;
    patch_wipeout = resources[i].wipeout_at_date(date,poss_normal, poss_non_zero);
    if(patch_wipeout) {
      area_wipeout = true;
    }
    if(i == 0) {
      if(patch_wipeout) {
	new_normal_after_wipeout = poss_normal;
      }
      non_zero_after_wipeout = poss_non_zero; 
    }
    else {
      if(patch_wipeout) {
	if(poss_normal > new_normal_after_wipeout) {
	  new_normal_after_wipeout = poss_normal; // needs to be latest
	}
      }
      if(poss_non_zero < non_zero_after_wipeout) {
      non_zero_after_wipeout = poss_non_zero; // needs to be earliest
      }
    }
  }

  if(area_wipeout) {
    normal_after_wipeout = new_normal_after_wipeout;
  }
  #if DEBUG
  db(tostring()); db(" wipeout at "); db(date);
  if(!area_wipeout) { db("redundant, as before "); }
  db(" norm again:"); db(normal_after_wipeout); db(" non-zero again:"); db(non_zero_after_wipeout); db("\n");
  #endif
}

// check for any patches are being_eaten at given location
// if so the location should be added to someone's revisit list
bool Resources::being_eaten_patches_at_location() {
  for(int i=0; i < resources.size(); i++) {
      CropPatch c = resources[i];
      if(c.being_eaten) { return true; }
  }

  return false;
}

// get non-empty patches which are also not being_eaten at the given location
// stores these in 'patches'
// NB: the ints in 'patches' are indexes into 'resources'
// so patches contains i the relevant CropPatch is resources[i]
// returns true if there any such patches
bool Resources::patches_at_location(vector<int>& patches) {
  patches.clear();
  for(int i= 0; i < resources.size(); i++) {
    if((!resources[i].being_eaten) && (resources[i].get_total() > 0)) {
      patches.push_back(i);
    }
  }
  #if DEBUG
  db("#patches "); db((int)(patches.size())); db(" at "); db(pos->tostring()); db("\n");
  #endif
  if(patches.size() > 0) {
    return true;
  }
  else {
    return false;
  }
}

int Resources::choose_rand_patch(vector<int> &patches) {

  crop_ordering_size = patches.size();

  crop_ordering = new int[crop_ordering_size];
  
  for(int o=0; o < crop_ordering_size; o++) {
    crop_ordering[o] = patches[o];
  }

  gsl_ran_shuffle(r_global, crop_ordering, crop_ordering_size, sizeof(int));

  int which = crop_ordering[0];
  
  delete [] crop_ordering;
    
  return which;

}

string Resources::tostring() {
  string s;
  s += id;
  s += "(";
  s += f_to_s(x);
  s += ",";
  s += f_to_s(y);
  s += ")";
  return s;
}

void test_Resources() {

  Resources r;
 
  int date = 0;
  
  // int d;
  // for(d=0; d < 7; d++) {
  //   if(berries.update_at_date(d)) {
  //   berries.show_bands();
  //   }
  // }

  // for(d=7; d < 15; d++) {
  //   if(berries.update_at_date(d)) {
  //     berries.show_bands();
  //   }

  // }

  for(int date=0; date < 100; date++) {
    #if DEBUG
    db(date); db(":\n");
    #endif
    r.update_at_date(date);
    #if DEBUG
    //    r.show_bands();
    r.show_bars();
    db("\n");
    #endif
  }
  
}

bool Resources::equals(ResPtr r)
{
  if(x == r->x && y == r->y && id == r->id)
    return true;
  return false;
}

// all the Resources areas
vector<ResPtr> all_res;