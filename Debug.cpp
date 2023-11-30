#include "Debug.h"
#include <iostream>
using namespace std;


int db_level = 0;

bool db_to_file = false; 
ofstream db_stream;

void db(string s) {
  if(db_level == 0) { return ; }
  if(db_to_file) {
    db_stream << s;
  }
  else {
    cout << s;
  }
}

void db(int i) {
  if(db_level == 0) { return ; }
  if(db_to_file) {
    db_stream << i;
  }
  else {
    cout << i;
  }
}

void db(char c) {
  if(db_level == 0) { return ; }
  if(db_to_file) {
    db_stream << c;
  }
  else {
    cout << c;
  }
}

void db(float f) {
  if(db_level == 0) { return ; }
  if(db_to_file) {
    db_stream << f;
  }
  else {
    cout << f;
  }
}

void init_db_file(string dest) {
    db_to_file = true;
  db_stream.open(dest); // check
  if(!db_stream) { cout << "some prob opening dest\n"; exit(1); }

}

void close_db_file() {
  if(db_to_file) { db_stream.close(); }
  
}
