#include <iostream>
#include <fstream>
#include <list>
#include <cctype>
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
  lptr = line.begin();
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
  Code *code = logical_term();
  for (;;) {
    if (match("or"))
      code = binary_operator(code, logical_term(), new Or());
    else
      break;
  }
  return code;
}

Code *MPL::logical_term() {
  Code *code = logical_factor();
  for (;;) {
    if (match("and"))
      code = binary_operator(code, logical_factor(), new And());
    else
      break;
  }
  return code;
}

Code *MPL::logical_factor() {
  if (match("not"))
    return unary_operator(logical_factor(), new Not());
  return equality_expression();
}

Code *MPL::equality_expression() {
  Code *code = arithmetic_expression();
  if (match("=="))
    code = binary_operator(code, arithmetic_expression(), new EQ());
  else if (match("<>"))
    code = binary_operator(code, arithmetic_expression(), new NE());
  else if (match("<="))
    code = binary_operator(code, arithmetic_expression(), new LE());
  else if (match("<"))
    code = binary_operator(code, arithmetic_expression(), new LT());
  else if (match(">="))
    code = binary_operator(code, arithmetic_expression(), new GE());
  else if (match(">"))
    code = binary_operator(code, arithmetic_expression(), new GT());
  return code;
}

Code *MPL::arithmetic_expression() {
  Code *code = term();
  for (;;) {
    if (match("+"))
      code = binary_operator(code, term(), new Add());
    else if (match("-"))
      code = binary_operator(code, term(), new Subtract());
    else
      break;
  }
  return code;
}

Code *MPL::term() {
  Code *code = factor();
  for (;;) {
    if (match("*"))
      code = binary_operator(code, factor(), new Multiply());
    else if (match("/"))
      code = binary_operator(code, factor(), new Divide());
    else if (match("mod"))
      code = binary_operator(code, factor(), new Modulus());
    else
      break;
  }
  return code;
}

Code *MPL::factor() {
 Code *code;

  code = negate_expression();
  if (match("^"))
    code = binary_operator(code, factor(), new Power());
  return code;
}

Code *MPL::negate_expression() {
  if (match("-"))
    return unary_operator(negate_expression(), new Negate());
  return primary();
}

Code *MPL::primary() {
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
  white_space();
  if (isalpha(s[0]))
    return match_identifier(s);
  string::iterator s1;
  string::const_iterator s2;
  for (s1 = lptr, s2 = s.begin(); *s1 == *s2; s1++, s2++)
    ;
  if (s2 == s.end()) {
    lptr = s1;
    return true;
  } 
  return false;
}

void MPL::white_space() {
  while (isspace(*lptr))
    lptr++;
}

bool MPL::match_identifier(string const &s) {
  string::iterator s1;
  string::const_iterator s2;
  for (s1 = lptr, s2 = s.begin(); *s1 == *s2; s1++, s2++)
    ;
  if (s2 == s.end() && !isalnum(*s1)) {
    lptr = s1;
    return true;
  }
  return false;
}
      
void MPL::execute(Code *c) {
  XXX();
}

void MPL::error(string const &msg) {
  cout << msg << endl;
  exit(1);
}
