#include "Util.h"
#include "CmdLineVersions.h"
#include "Person.h"

#include <iostream>
#include <algorithm>
#include <ctime>

gsl_rng *r_global = 0;

void rand_setup() {
  r_global = gsl_rng_alloc (gsl_rng_default);

}

ofstream r_stats;

DayStats r_line;

ofstream gains_info_history;
//AreaGains a_agg;
//AreaGains b_agg;
AreaGains agg;
vector<float> sched_agg; // prob redundant
int num_switches = 0;
int num_switches_A = 0;
int num_switches_B = 0;



ofstream pop_snapshots;

ofstream starvation_stats;

string gains_info_header;

void write_gains_info_header(ostream& history) {
  // has side effect of saving the header string in gains_info_header
  // used to calc how many dummy entries to add for days with no people
  gains_info_header = "";
  gains_info_header += "DATE OneA TwoA ThreeA FourA FiveA OneB TwoB ThreeB FourB FiveB ";
  gains_info_header += "A0 A1 A2 A3 A4 B0 B1 B2 B3 B4 "; // obsolete
  gains_info_header += "sh_a ";
  gains_info_header += "c0_a c1_a c2_a c3_a c4_a g0_a g1_a g2_a g3_a g4_a m0_a m1_a m2_a m3_a m4_a ";
  gains_info_header += "sh_b ";
  gains_info_header += "c0_b c1_b c2_b c3_b c4_b g0_b g1_b g2_b g3_b g4_b m0_b m1_b m2_b m3_b m4_b";

  history << gains_info_header << "\n";

}
void write_gains_info_line(ostream& history) {
  // was in update after feeding
   // record  area gains into gains_info_history 
  history << r_line.DATE << " ";


  /***************************************************************/
  /* (OneA TwoA ThreeA FourA FiveA)                              */
  /* output A sub-type gains, disregarding position in schedules */
  /***************************************************************/
  agg.clear_area_gains();
  pop.aggregate_area_gains('A',agg);
  for(int i = 0; i < all_res.size(); i++) {
      float f;
      if(agg.get_gain(all_res[i],f)) {
	history << f ; 
      }
      else {
	history << 0;
      }
      history << " ";
  }

  /***************************************************************/
  /* (OneB TwoB ThreeB FourB FiveB)                              */
  /* output B sub-type gains, disregarding position in schedules */
  /***************************************************************/

  agg.clear_area_gains();
  pop.aggregate_area_gains('B',agg);
  for(int i = 0; i < all_res.size(); i++) {
      float f;
      if(agg.get_gain(all_res[i],f)) {
	history << f ; 
      }
      else {
	history << 0;
      }
      history << " ";
  }
  history << endl;


  
}


void write_pop_snapshot_header(ostream& o) {
  o << "DATE TYPE AGE EN EATEN FIRST\n";

}


// v.sim to Population::show
void write_pop_snapshot(ostream& o) {
  if(pop.population.size() == 0) { return; }
  
  vector<PerPtr> pop_cpy;
  pop_cpy = pop.population;
  sort(pop_cpy.begin(),pop_cpy.end(),compare_person);

  PerPtr p;
  for(size_t i=0; i < pop_cpy.size(); i++) {
    o << r_line.DATE << " ";
    p = pop_cpy[i]; 
    o << p->type << " ";
    o << p->age << " ";
    o << p->current_energy << " ";
    o << p->eaten_today << " ";
    o << "\n";
  }

  // if crashes below may leave more output captured
  // o.flush();

}


void write_starvation_stats_header(ostream& o) {
  o << "DATE ID TYPE AGE EN\n";

}

void write_starvation_stats_line(ostream& o,PerPtr p) {
  o << r_line.DATE << " ";
  o << p->toid() << " ";
  o << p->type << " ";
  o << p->age << " ";
  o << p->current_energy << " ";
    
}

string f_to_s(float f) {
  string s;
  ostringstream ss;
  ss << f;
  s = ss.str();
  return s;
}

int count_spaces(string s) {

  int count = 0;
  size_t pos = 0;
  while((pos = s.find(' ',pos)) != string::npos) {
    count++;
    pos++;
  }

  return count;

}

ofstream setup_record;

string time_stamp(void)
{
  time_t  cal_time;
  
  cal_time = time(NULL);
  char * time_ptr;
  time_ptr = ctime(&cal_time); 
  string time_string;
  time_string = string(time_ptr);
  size_t spc_pos=0;
  while((spc_pos = time_string.find(' ',spc_pos)) != string::npos) {
    time_string[spc_pos] = '_';
    spc_pos++;
  }
  // to get rid of newline at end
  time_string = time_string.substr(0,time_string.size()-1);
  return time_string;
}


void open_an_output(ofstream& strm, string name) {
  string full_name;
  full_name = sim_params.results_dir + "/" + name;
  if(sim_params.output_suffix != "") {
    full_name += "_";
    full_name += sim_params.output_suffix;
  }
  strm.open(full_name);
  if (!strm) {
    cout << "prob opening " << full_name << endl;
    exit(1);
  }

}

// UNFINISHED
void props_to_sizes(vector<double> &props, int N, vector<size_t> sizes) {
  // make each size[i] ~= props[i] * N
  // round props[i] * N to nearest : 3.1 --> 3,  3.6 --> 4


}

// UNFINISHED
// void make_subsets(vector<int> &pop, vector<double> &props, map<int,<vector<int>> sub_pops) {
//   // each sub_pops[i] should be random subset pop of size = props[i]
//   // eg. props = { 0.5, 0.3, 0.2}, pop of size 10
//   // randomly shuffle pop
//   // set props_index = { 0 ... N-1 }  N is len(props)
//   // randomly shuffle props_index
//   // eg. 3 0 4 2 1
//   // set start (in pop) = 0
//   // for each in i shuffled props_index
//   //   set sub_size = props[i] * P   P is size pop
//   //   set end = sub_size-1
//   //   put pop[start:end] into sub_pops[i]
// }
