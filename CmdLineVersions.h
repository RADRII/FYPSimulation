#include <string>
using namespace std;

#if !defined POP_SIM_PAR_H
#define POP_SIM_PAR_H
class PopSimPar {
 public:
  PopSimPar();
  string results_dir;
  bool time_stamp; // if yes time will be a suffix
  string output_suffix; // use to pass in some pre-defined suffix
  string random_seed_policy; // via_default, via_time, user_supplied 
};

extern PopSimPar sim_params;

#endif

