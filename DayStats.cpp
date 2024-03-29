#include "DayStats.h"

void DayStats::clear()
{
  DATE = -1;
  CROP_INCR = -1;
  //  NEWTWO= -1;
  CROP_TOTAL = -1;
  // BEANS = -1;
  NUM_AREAS_IN_WIPEOUT = -1;
  NUM_AREAS_IN_PLENTY = -1;
  DEATHS_AGE = -1;
  DEATHS_STARVE = -1;
  DEATHS_STRANDED = -1;
  BIRTHS = -1;
  POP = -1;
  APOP = -1;
  BPOP = -1;
  CPOP = -1;
  MEAN_EN = -1.0;
  MEAN_EATEN = -1.0;
  MAX_NUM_PLACES_EATEN = 0;
  MEAN_NUM_PLACES_EXPLORED = 0;
}


void DayStats::write(ostream& o)
{
  o << DATE << " ";
  o << CROP_INCR << " ";
  o << CROP_TOTAL << " ";
  o << NUM_AREAS_IN_WIPEOUT << " ";
  o << NUM_AREAS_IN_PLENTY << " ";
  o << MAX_NUM_PLACES_EATEN << " ";
  o << MEAN_NUM_PLACES_EXPLORED << " ";
  o << DEATHS_AGE << " ";
  o << DEATHS_STARVE << " ";
  o << DEATHS_STRANDED << " ";
  o << BIRTHS << " ";
  o << POP << " ";
  o << APOP << " ";
  o << BPOP << " ";
  o << CPOP << " ";
  o << MEAN_EN << " " ;
  o << MEAN_EATEN << " ";
  o << endl;

  clear();

}

void DayStats::write_header(ostream& o) 
{

  o << "DATE CROP_INCR CROP_TOTAL NUM_AREAS_IN_WIPEOUT NUM_AREAS_IN_PLENTY MAX_NUM_PLACES_EATEN MEAN_NUM_PLACES_EXPLORED DEATHS_AGE DEATHS_STARVE DEATH_STRAND BIRTHS POP APOP BPOP CPOP MEAN_EN MEAN_EATEN \n";
}

