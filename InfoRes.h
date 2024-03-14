#if !defined(INFORES_h)
#define INFORES_h

#include "Resource.h"
#include "AreaGain.h"

enum KnowledgeOrig {SENSES, TALKING};

class InfoRes { // info about a resource area
 public:
    InfoRes();
    int numPatches;
    vector<int> known_total_of_patches;

    bool isWipeout;
    KnowledgeOrig wipeOutOrig;
    int till_non_zero; // num days till become non-zero after wipeout
    int till_normal; // num days till become normal after wipeout

    bool isPlenty;
    int numAdded;
    KnowledgeOrig plentyOrig;
    int till_non_plenty; // num days till go back to normal amounts of patches after a plenty
    
    string tostring();
};

#endif
