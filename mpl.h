#ifndef _MPL_H_ // -*- C++ -*-
#define _MPL_H_

class MPL {
public:
  void read(std::istream *);

private:
  std::istream *input;
  std::string line;
  int line_index;
  
  void read_line();
  void line_continuation();
  void comment();
  bool match(std::string const &);
  void command();
  void execute(Code *);
  void do_include();
  void define_function();
  Code *expression();
  Code *unary_operator(Code *, Instruction *);
  Code *binary_operator(Code *, Code *, Instruction *);
  void error(std::string const &);
};

#endif
