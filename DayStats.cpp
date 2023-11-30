#include "DayStats.h"

void DayStats::clear() {
  DATE = -1;
  CROP_INCR = -1;
  //  NEWTWO= -1;
  CROP_TOTAL = -1;
  // BEANS = -1;
  NUM_AREAS_IN_WIPEOUT = -1;
  DEATHS_AGE = -1;
  DEATHS_STARVE = -1;
  BIRTHS = -1;
  POP = -1;
  TYPEA = -1;
  TYPEB = -1;
  A_EN = -1.0;
  B_EN = -1.0;
  A_EATEN = -1.0;
  B_EATEN = -1.0;
  HOMETIME_MAX = -1.0;
  HOMETIME_MAX_LIVING = -1.0;
  HOMETIME_MAX_DEAD = -1.0;
  TWOPLACETIME_MAX = -1.0;
  MAX_NUM_PLACES_EATEN = 0;
  MAX_NUM_PLACES_EXPLORED = 0;
}


void DayStats::write(ostream& o) {
  o << DATE << " ";
  o << CROP_INCR << " ";
  o << CROP_TOTAL << " ";
  o << NUM_AREAS_IN_WIPEOUT << " ";
  o << HOMETIME_MAX_LIVING << " ";
  o << HOMETIME_MAX_DEAD << " ";
  o << HOMETIME_MAX << " ";
  o << TWOPLACETIME_MAX << " ";
  o << MAX_NUM_PLACES_EATEN << " ";
  o << MAX_NUM_PLACES_EXPLORED << " ";
  o << DEATHS_AGE << " ";
  o << DEATHS_STARVE << " ";
  o << BIRTHS << " ";
  o << POP << " ";
  o << TYPEA << " ";
  o << TYPEB << " ";
  o << A_EN << " " ;
  o << B_EN << " ";
  o << A_EATEN << " ";
  o << B_EATEN << endl;

  clear();

}

void DayStats::write_header(ostream& o) {

  o << "DATE CROP_INCR CROP_TOTAL NUM_AREAS_IN_WIPEOUT HOMETIME_MAX_LIVING HOMETIME_MAX_DEAD HOMETIME_MAX TWOPLACETIME_MAX MAX_NUM_PLACES_EATEN MAX_NUM_PLACES_EXPLORED DEATHS_AGE DEATHS_STARVE BIRTHS POP TYPEA TYPEB A_EN B_EN A_EATEN B_EATEN\n";
}

