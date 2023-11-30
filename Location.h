#if !defined(POPLOC_H)
#define POPLOC_H
#include <string>
#include <vector>
using namespace std;

enum LocKind {HAB_ZONE, NODE, RES_ENTRY, PATCH};
enum ArcKind {HAB_ZONE_L, NODE_L, RES_ENTRY_L,RES_L, PATCH_L};

class Location;
typedef Location * LocPtr;

class LocArc {
 public:
  LocArc();
  ArcKind kind;
  LocPtr nxt;
  string tostring();
};

class Location {
 public:
  Location();
  Location(LocKind k);
  LocKind kind;
  float x;
  float y;
  string id;
  string tostring();
  void show();
  void show_links();
  void show_links(ostream& o);
  int occupancy; // num people at it; will include many 'really' in transit
  vector<LocArc> nbs; // to represent other Locations its linked to
  void add_arc(ArcKind k, LocPtr trg);
  bool trace_fst(ArcKind k, LocPtr& trg);// fetch first for kind k, if any
  bool operator<(const Location& l) const
  {
    if (x < l.x) {
      return true;
    }
    else if (x == l.x && y < l.y) {
      return true;
    }
    else {
      return false;
    }
  }
};

// globals vars
extern vector<LocPtr> all_home_loc;
extern vector<LocPtr> all_res_entry_loc;

#endif

