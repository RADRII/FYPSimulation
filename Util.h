#if !defined(POPUTIL_h)
#define POPUTIL_h

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <fstream>
#include <sstream>
#include "DayStats.h"
#include "AreaGain.h"
#include "Person.h"

using namespace std;

// random related
extern gsl_rng *r_global;
extern void rand_setup();

/********************************************/
/* day-by-day resource and population stats */
/* in R-friendly format                     */
/********************************************/
extern  ofstream r_stats;
extern DayStats r_line;

/******************/
/* pop snap shots */
/******************/
extern  ofstream pop_snapshots;

void write_pop_snapshot_header(ostream& o);
void write_pop_snapshot(ostream& o);


/********************/
/* starvation stats */
/********************/

extern  ofstream starvation_stats;

void write_starvation_stats_header(ostream& o);
void write_starvation_stats_line(ostream& o,PerPtr p);

/********************/
/* stranding stats */
/********************/

extern  ofstream stranding_stats;

void write_stranding_stats_header(ostream& o);
void write_stranding_stats_line(ostream& o,PerPtr p);


/*****************************************************/
/* record of how 'world' was set-up for a simulation */
/* People Resources                                  */
/*****************************************************/

extern  ofstream setup_record;

/*****************************************************/
/* debug stream, for whatever i want :)     */
/*****************************************************/
extern  ofstream debug_record;

string f_to_s(float f);

string time_stamp(void);

int count_spaces(string s);


void open_an_output(ofstream& strm, string name);

//void make_subsets(vector<int> &pop, vector<double> &props, vector<vector<int>> sub_pops);

#endif
