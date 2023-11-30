#include <fstream>
using namespace std;

void db(string s); 
void db(int i); 
void db(char c); 
void db(float f); 

extern bool db_to_file; 
extern int db_level;
extern ofstream db_stream;
void init_db_file(string dest);
void close_db_file();
