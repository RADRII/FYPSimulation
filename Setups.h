#if !defined(POPSETUP_H)
#define POPSETUP_H

#include <string>
#include <list>
using namespace std;

//Pre-defined list of setups
class Setups {
 public:
  Setups(int setupNum);
  int numA;
  int numB;
  int numC;
  float percentW;
  float percentP;
};

#endif
