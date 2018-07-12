#include <iostream>
#include "error.h"

using namespace std;

void fatal_error(string msg) {
  cout << "fatal error: " << msg << endl;
  exit(1);
}
