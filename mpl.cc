#include <iostream>
#include <fstream>
#include <list>
#include "main.h"
#include "instruction.h"
#include "mpl.h"

using namespace std;

void MPL::read(istream *in) {
  istream *save_input = input;
  input = in;
  for (;;) {
    read_line();
    if (line.empty())
      break;
    command();
  } 
  input = save_input;
}

void MPL::read_line() {
  int c;
  bool instring = false;
  bool graphflag = false;
  
  do {
    line.clear();
    line_index = 0;
    for (;;) {
      c = input->get();
      if (c == EOF)
	return;
      if (c == '\n')
	break;
      if (c == '"')
	instring = !instring;
      else if (c == '~' && !instring) {
	comment();
	break;
      }
      else if (c == '\\') {
	if (instring) {
	  line += c;
	  c = input->get();
	  if (c == '\n' || c == EOF)
	    error("line continuations not allowed in strings");
	}
	else {
	  line_continuation();
	  c = ' ';
	}
      }
      if (isgraph(c))
	graphflag = true;
      line += c;
    }
    if (instring)
      error("missing \"");
  } while (!graphflag);
}

void MPL::line_continuation() {
  int c;
  
  for (;;) {
    c = input->get();
    if (c == '\n')
      return;
    if (isgraph(c)) {
      input->putback(c);
      return;
    }
  }
}
      

void MPL::comment() {
  int c;

  do
    c = input->get();
  while (c != '\n' && c != EOF);
}
    

void MPL::command() {
  if (match("include"))
    do_include();
  else if (match("function"))
    define_function();
  else {
    Code *e = expression();
    if (match("="))
      e = binary_operator(e, expression(), new Assign());
    else
      e = unary_operator(e, new Print());
    execute(e);
    delete e;
  }
}

void MPL::do_include() {
  XXX();
}

void MPL::define_function() {
  XXX();
}

Code *MPL::expression() {
  XXX();
  return 0;
}

Code *MPL::unary_operator(Code *e, Instruction *instr) {
  e->push_back(instr);
  return e;
}

Code *MPL::binary_operator(Code *e1, Code *e2, Instruction *instr) {
  e1->splice(e1->end(), *e2);
  e1->push_back(instr);
  return e1;
}

bool MPL::match(string const &s) {
  XXX();
  return false;
}

void MPL::execute(Code *c) {
  XXX();
}

void MPL::error(string const &msg) {
  cout << msg << endl;
  exit(1);
}
