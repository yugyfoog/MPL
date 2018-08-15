#include "main.h"
#include "symbol.h"
#include "instruction.h"

using namespace std;

RealConstant::RealConstant(string const &s) {
  x = stod(s);
}

StringConstant::StringConstant(string const &) {
  XXX();
}

Call::Call(Function *func) {
  XXX();
}

Variable::Variable(Symbol *sym) {
  XXX();
}

