#if !defined(INFORES_h)
#define INFORES_h
#include <vector>

using namespace std;

enum KnowledgeOrig {SENSES, TALKING, NA};

class InfoRes { // info about a resource area
 public:
    InfoRes();
    int knownResIndex;
    vector<int> known_total_of_patches;

    bool isWipeout;
    bool isNotNormal; //aka is it post zero but not back to normal yet
    KnowledgeOrig wipeOutOrig;
    int till_non_zero; // num days till become non-zero after wipeout
    int till_normal; // num days till become normal after wipeout

    bool isPlenty;
    int originalSize;
    KnowledgeOrig plentyOrig;
    int till_non_plenty; // num days till go back to normal amounts of patches after a plenty

    void dailyDateCheck(int date);
    
    string tostring();
};

#endif
