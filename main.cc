#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include "main.h"
#include "instruction.h"
#include "mpl.h"
#include "error.h"

using namespace std;

int main(int argc, char **argv) {
  ifstream in;
  MPL mpl;

  in.open(STDLIB_FILE);
  if (!in.good())
    fatal_error(string("unable to open ") + STDLIB_FILE);
  mpl.read(&in);
  in.close();
  if (argc <= 1)
    mpl.read(&cin);
  else {
    for (int i = 1; i < argc; i++) {
      in.open(argv[i]);
      if (!in.good())
	fatal_error(string("unable to open ") + argv[i]);
      mpl.read(&in);
      in.close();
    }
  }
  return 0;
}

void x_undefined(char const *file, char const *func, int line) {
  cout << func << "() undefined in " << file << " near line " << line << endl;
  exit(1);
}

