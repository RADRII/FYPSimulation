#include "Location.h"
#include "Resource.h" // needed for linking of res entry locations
#include "Util.h"
#include <iostream>


LocArc::LocArc() {
  kind = PATCH_L;
  nxt = NULL;
}

string LocArc::tostring() {
  string s = " - ";
  if(kind == RES_L) {
    s += "res";
  }
  else if(kind == RES_ENTRY_L) {
    s += "res_entry";
  }
  else if(kind == PATCH_L) {
    s += "patch";
  }
  else if(kind == NODE_L) {
    s += "node";
  }
  else {
    s += "home";
  }
  s += " - ";
  if(nxt == NULL) { s += "||"; }
  else { s += nxt->tostring(); }
  return s;
}

Location::Location() {
  x = 0.0;
  y = 0.0;
  kind = PATCH;
  nbs.clear();
}

Location::Location(LocKind k) {
  x = 0.0;
  y = 0.0;
  kind = k;
  nbs.clear();

}

void  Location::show() {

  cout << tostring() << endl;
}

void  Location::show_links() {
  for(size_t i=0; i < nbs.size();i++) {
    cout <<  tostring() << nbs[i].tostring() << endl; 
  }

}

void  Location::show_links(ostream& o) {
  for(size_t i=0; i < nbs.size();i++) {
    o <<  tostring() << nbs[i].tostring() << endl; 
  }

}

string Location::tostring() {
  string s;
  if(kind == RES_ENTRY) { s = "e"; }
  else if(kind == PATCH) { s = "c"; }
  else if(kind == HAB_ZONE) { s = "h"; }
  else if(kind == NODE) { s = "n"; }
  else { s = "u"; }
  s += "[";
  // s += id;
  // s += " ";
  s += f_to_s(x);
  s += ",";
  s += f_to_s(y);
  s += "]";
  return s;
}

void Location::add_arc(ArcKind k, LocPtr trg) {
  LocArc l;
  l.kind = k;
  l.nxt = trg;
  nbs.push_back(l);
}

bool Location::trace_fst(ArcKind k, LocPtr& trg) { // fetch first for kind k, if any
  
  for(size_t i=0; i < nbs.size(); i++) {
    if(nbs[i].kind == k) {
      trg = nbs[i].nxt;
      return true;
    }
  }
  
  return false;

}

vector<LocPtr> all_home_loc;
vector<LocPtr> all_res_entry_loc;


