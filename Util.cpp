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

ofstream pop_snapshots;

ofstream starvation_stats;

ofstream stranding_stats;

ofstream debug_record;

void write_pop_snapshot_header(ostream& o) {
  o << "DATE TYPE AGE EN EATEN" << endl;
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
    o << to_string(p->identifier) << " ";
    o << p->age << " ";
    o << p->current_energy << " ";
    o << p->eaten_today << " ";
    o << "\n";
  }

  // if crashes below may leave more output captured
  // o.flush();

}


void write_starvation_stats_header(ostream& o) {
  o << "DATE ID TYPE AGE EN" << endl;

}

void write_starvation_stats_line(ostream& o,PerPtr p) {
  o << r_line.DATE << " ";
  o << p->toid() << " ";
  o << p->type << " ";
  o << p->age << " ";
  o << p->current_energy << " ";   
  o << endl;
}

void write_stranding_stats_header(ostream& o) {
  o << "DATE ID TYPE PREVACTION LOCX LOCY" << endl;
}

void write_stranding_stats_line(ostream& o,PerPtr p) {
  o << r_line.DATE << " ";
  o << p->toid() << " ";
  o << p->type << " ";
  o << p->prevAction << " ";
  o << p->loc->x << " ";   
  o << p->loc->y << " ";   
  o << endl;
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
