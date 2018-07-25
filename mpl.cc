#include <iostream>
#include <fstream>
#include <list>
#include <cctype>
#include "main.h"
#include "symbol.h"
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


/*

 ( expression )
 ( expression , expression )
 [ ylist ]
 { list }
 number
 "string"
 identiier
 identifier ( arguments )
 identifier [ expression ]
 identifier [ <expression> , <expession> ]
*/

Code *MPL::primary() {
  Code *code;
  string token;
  
  if (match("(")) {
    code = expression();
    if (match(","))
      code = binary_operator(code, expression(), new MakeComplex());
    need(")");
  }
  else if (match("[")) {
    code = ylist();
    need("]");
    code = unary_operator(code, new MatVec());
  }
  else if (match("{")) {
    code = zlist();
    need("}");
  }
  else if (match_number(token))
    code = new Code(1, new RealConstant(token));
  else if (match_string(token))
    code = new Code(1,new StringConstant(token));
  else if (match_identifier(token)) {
    Symbol *sym;
    if (symbol_table.find(token) == symbol_table.end())
      sym = 0;
    else
      sym = symbol_table[token];
    if (match("(")) {
      Function *func;
      if (sym == 0) {
	func = new Function();
	symbol_table[token] = func;
      }
      else {
	if (typeid(*sym) != typeid(Function))
	  error(token + " is not a function");
	func = (Function *)sym;
      }
      code = function_arguments(func->args);
      need(")");
      if (func->args == -1)
	func->args = argument_count;
      code = unary_operator(code, new Call(func));
    }
    else {
      if (sym == 0) {
	if (infunction)
	  sym = new Local();
	else
	  sym = new Global();
	symbol_table[token] = sym;
      }
      code = new Code(1, new Variable(sym));
      for (;;) {
	if (match("[")) {
	  if (match(",")) {
	    code = binary_operator(code, expression(), new ColumnIndex());
	    need("]");
	  }
	  else {
	    Code *code2 = expression();
	    if (match(",")) {
	      if (match("]"))
		code = binary_operator(code, code2, new RowIndex());
	      else {
		code = ternary_operator(code, code2, expression(),
					new MatrixIndex());
		need("]");
	      }
	    }
	    else {
	      code = binary_operator(code, code2, new Index());
	      need("]");
	    }
	  }
	}
	else
	  break;
      }
    }
  }
  else
    error("syntax error");
  return code;
}

Code *MPL::xlist() {
  XXX();
  return 0;
}

Code *MPL::ylist() {
  XXX();
  return 0;
}

Code *MPL::yylist() {
  XXX();
  return 0;
}

Code *MPL::zlist() {
  XXX();
  return 0;
}

Code *MPL::function_arguments(int n) {
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

Code *MPL::ternary_operator(Code *e1, Code *e2, Code *e3, Instruction *instr) {
  e1->splice(e1->end(), *e2);
  e1->splice(e1->end(), *e3);
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

bool MPL::match_number(string &s) {
  XXX();
  return false;
}

bool MPL::match_string(string &s) {
  XXX();
  return false;
}

bool MPL::match_identifier(string &s) {
  XXX();
  return false;
}

void MPL::need(string const &s) {
  white_space();
  string::iterator s1;
  string::const_iterator s2;
  for (s1 = lptr, s2 = s.begin(); s2 != s.end(); s1++, s2++)
    if (*s1 != *s2)
      error("missing " + s);
  lptr = s1;
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
