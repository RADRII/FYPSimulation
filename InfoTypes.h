#if !defined(INFOTYPES_h)
#define INFOTYPES_h

#include "Resource.h"
#include "AreaGain.h"

enum KnowledgeOrig {SENSES, TALKING};


class WipeoutInfo { // info about a resource area in wipeout
 public:
  WipeoutInfo();
  ResPtr res_to_ignore;
  int till_non_zero; // num days till become non-zero after wipeout
  int till_normal; // num days till become normal after wipeout
  bool impart_today;
  KnowledgeOrig orig;
  string tostring();
};





enum InfoType { WIPE };

#endif
