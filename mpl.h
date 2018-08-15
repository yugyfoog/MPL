#ifndef _MPL_H_ // -*- C++ -*-
#define _MPL_H_

#include <map>

class MPL {
public:
  void read(std::istream *);

private:
  std::istream *input;
  std::string line;
  std::string::iterator lptr;
  int line_index;
  int argument_count;
  bool infunction;
  
  std::map<std::string, Symbol *> symbol_table;
  
  void read_line();
  void line_continuation();
  void comment();
  void need(std::string const &);
  bool match(std::string const &);
  bool match_number(std::string &);
  bool match_string(std::string &);
  bool match_identifier(std::string &);
  void white_space();
  bool match_identifier(std::string const &);
  void command();
  void execute(Code *);
  void do_include();
  void define_function();
  void define_procedure();
  Code *parse();
  
  Code *expression();
  Code *logical_term();
  Code *logical_factor();
  Code *equality_expression();
  Code *arithmetic_expression();
  Code *term();
  Code *factor();
  Code *negate_expression();
  Code *primary();
  Code *xlist();
  Code *ylist();
  Code *yylist();
  Code *zlist();
  Code *function_arguments(int);
  Code *unary_operator(Code *, Instruction *);
  Code *binary_operator(Code *, Code *, Instruction *);
  Code *ternary_operator(Code *, Code *, Code *, Instruction *);
  void error(std::string const &);
};

#endif
