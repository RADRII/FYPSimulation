#if !defined(DAYSTATS_H)
#define DAYSTATS_H

#include <fstream>

using namespace std;

class DayStats {
 public:
  int DATE;
  int CROP_INCR;
  int CROP_TOTAL;
  int NUM_AREAS_IN_WIPEOUT;
  int NUM_AREAS_IN_PLENTY;
  int DEATHS_AGE;
  int DEATHS_STARVE;
  int DEATHS_STRANDED;
  int BIRTHS;
  int POP;
  int APOP;
  int BPOP;
  int CPOP;
  float MEAN_EN;
  float MEAN_EATEN;
  int MAX_NUM_PLACES_EATEN;
  int MEAN_NUM_PLACES_EXPLORED;
  void clear();
  void write(ostream& o);
  void write_header(ostream& o);

};


#endif
