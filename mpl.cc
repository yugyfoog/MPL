#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <stack>
#include <vector>
#include <string>
#include <complex>
#include <valarray>
#include <memory>
#include <typeinfo>
#include <cctype>
#include "value.hh"
#include "code.hh"
#include "function.hh"
#include "mpl.hh"

std::stack<std::istream *> file_stack;

Token_List *tokens = 0;

bool in_function = false;
int locals = 0;
int stack_pointer = 0;
int frame_pointer = 0;
std::vector<Value_ptr> memory;

Symbol_Table local_symbol_table;
Symbol_Table global_symbol_table;
Function_Table function_table;

void mpl_error(std::string const &s) {
  std::cout << "error: " << s << std::endl;
  exit(1);
}

Value_ptr read_memory(unsigned index) {
  if (memory.size() <= index)
    return 0;
  Value_ptr x = memory[index];
  return x;
}

void write_memory(unsigned index, Value_ptr x) {
  if (memory.size() <= (unsigned)stack_pointer)
    memory.resize(stack_pointer);
  if (index < (unsigned)frame_pointer && typeid(*x) == typeid(Memory_Reference)) {
    std::cout << "writing reference to global variable" << std::endl;
    exit(1);
  }
  if (memory[index] != x)
    memory[index] = x;
}

// scan_line() removes comments and returns true if
// there was a continuation.

//    This is tricky because both the comment character, '~', and
//    continuation character, '\', can be in strings.
//    (implemented as a state machine)

bool scan_line(std::string &line) {
  int state = 0;
  auto comment = line.end();
  auto continuation = line.end();
  
  for (auto p = line.begin(); p != line.end(); p++) {
    switch (state) {
    case 0:
    state0:
      // not in string, not in possible continuation
      switch (*p) {
      case '~':
	if (comment == line.end())
	  comment = p;
	break;
      case '\\':
	continuation = p;
	state = 1;
	break;
      case '"':
	if (comment == line.end()) 
	  state = 2;
	break;
      default:
	;
      }
      break;
    case 1:
      // not in string, possible continuation
      if (isgraph(*p)) {
	// Not a continuation. Process current character
	// as though we were in state 0
	continuation = line.end();
	state = 0;
	goto state0;
      }
      break;
    case 2:
      // in string, not following a backslash
      switch (*p) {
      case '\\':
	state = 3;
	break;
      case '"':
	state = 0;
	break;
      default:
	;
      }
      break;
    case 3:
      // in string, following a backslash
      state = 2;
      break;
    }
  }
  
  if (comment < continuation) {
    // remove comment
    line.erase(comment, line.end());
    line += ' ';
  }
  else if (continuation < comment) {
    // remove continuation
    line.erase(continuation, line.end());
    line += ' ';
  }
  switch (state) {
  case 0:
    return false;
  case 1:
    return true;
  default:
    std::cout << "missing quote (\")" << std::endl;
    exit(1);
  }
}

std::string read_line() {
  std::string line;
  std::string next;
  bool continuation = false;

  do {

    // make sure we have a file to read from
    
    for (;;) {
      if (file_stack.empty())
	return line;
      if (file_stack.top()->good())
	break;
      if (file_stack.top() != &std::cin) // don't delete cin!
	delete file_stack.top();
      file_stack.pop();
    }

    // now read
    
    std::getline(*file_stack.top(), next);
    continuation = scan_line(next);
    line += next;
  } while (continuation);
  return line;
}

bool is_id_startchar(int c) {
  return isalpha(c) || c == '_' || c == '$';
}

bool is_id_char(int c) {
  return isalnum(c) || c == '_' || c == '$';
}

bool is_identifier() {
  if (tokens->empty())
    return false;
  if (is_id_startchar((*tokens->front())[0]))
    return true;
  return false;
}

bool is_number() {
  if (tokens->empty())
    return false;
  int c = (*tokens->front())[0];
  return isdigit(c) || c == '#' || c== '.';
}

bool is_string() {
  if (tokens->empty())
    return false;
  return (*tokens->front())[0] == '"';
}

Token_List *tokenize(std::string const &line) {
  auto tkns = new Token_List();
  std::string s;
  for (auto p = line.begin(); p != line.end();) {
    if (!isgraph(*p))
      p++;
    else if (is_id_startchar(*p)) {
      s += *p++;
      while (is_id_char(*p))
	s += *p++;
      tkns->push_back(new std::string(s));
      s.clear();
    }
    else if (isdigit(*p) || *p == '.') {
      while (isdigit(*p))
	s += *p++;
      if (*p == '.') {
	s += *p++;
	while (isdigit(*p))
	  s += *p++;
      }
      if (toupper(*p) == 'E') {
	s += *p++;
	if (*p == '+' || *p == '-')
	  s += *p++;
	while (isdigit(*p))
	  s += *p++;
      }
      tkns->push_back(new std::string(s));
      s.clear();
    }
    else if (*p == '#') {
      s += *p++;
      while (isxdigit(*p))
	s += *p++;
      tkns->push_back(new std::string(s));
      s.clear();
    }
    else if (*p == '\"') {
      s += *p++;
      while (p != line.end() && *p != '"') {
	if (*p == '\\') {
	  s += *p++;
	  if (p != line.end())
	    s += *p++;
	}
	else
	  s += *p++;
      }
      if (*p == '"')
	s += *p++;
      else
	s += '"';
      tkns->push_back(new std::string(s));
      s.clear();
    }
    else {
      char c = *p++;
      s += c;
      if (p != line.end()) {
	if (c == '=' && *p == '=')
	  s += *p++;
	else if (c == '!' && *p == '=')
	  s += *p++;
	else if (c == '<' && *p == '=')
	  s += *p++;
	else if (c == '>' && *p == '=')
	  s += *p++;
      }
      tkns->push_back(new std::string(s));
      s.clear();
    }
  }
  return tkns;
}

void next_line() {
  std::string line = read_line();
  tokens = tokenize(line);
}

bool at_end_line() {
  return tokens->empty();
}

void line_end() {
  if (!at_end_line()) {
    std::cout << "syntax error at " << *tokens->front() << std::endl;
    exit(1);
  }
}

void next_token() {
  delete tokens->front();
  tokens->pop_front();
}

bool check(std::string const &s) {
  if (tokens->empty()) // tokens will be empty if we are at the end of the line
    return false;
  return *tokens->front() == s;
}

bool match(std::string const &s) {
  if (check(s)) {
    next_token();
    return true;
  }
  return false;
}

void need(std::string const &s) {
  if (check(s))
    next_token();
  else {
    if (tokens->front())
      std::cout << "syntax error: " << s << " expected near " << *tokens->front() << std::endl;
    else
      std::cout << "syntax error: " << s << " expected" << std::endl;
    exit(1);
  }
}

std::string identifier() {
  std::string s = *tokens->front();
  next_token();
  return s;
}

Code *expression();

// xlist() -- list, vector, or row of a matrix (not empty)

Code *xlist() {
  Code *x = expression();
  if (match(","))
    return new ListPart(x, xlist());
  return new ListPart(x, 0);
}

// rows of a matrix

Code *yylist() {
  Code *x = xlist();
  if (at_end_line()) {
    do
      next_line();
    while (at_end_line());
    return new ListPart(x, yylist());
  }
  if (match("|"))
    return new ListPart(x, yylist());
  return new ListPart(x, 0);
}

// ylist() -- either a vector or a matrix

Code *ylist() {
  Code *x = xlist();
  if (at_end_line()) {
    do
      next_line();
    while (at_end_line());
    return new ListPart(x, yylist());
  }
  if (match("|"))
    return new ListPart(x, yylist());
  return x;
}

Code *parameters() {
  Code *param = expression();
  if (match(","))
    return new Parameter(param, parameters());
  return new Parameter(param, 0);
}

Code *range() {
  Code *x;
  Code *y = 0;
  Code *z = 0;

  x = expression();
  if (match(":")) {
    y = expression();
    if (match(":"))
      z = expression();
    return new Range(x, y, z);
  }
  return x;
}

Code *primary_expression() {
  Code *x;
  
  if (match("(")) {
    x = expression();
    if (match(","))
      x = new ComplexX(x, expression());
    need(")");
  }
  else if (match("[")) {
    x = ylist();
    need("]");
    return new Call("matvec", new Parameter(x, 0));
  }
  else if (match("{")) {
    if (check("}"))
      x = new ListX(0); // empty list
    else
      x = new ListX(xlist());
    need("}");
  }
  else if (is_number()) {
    x = new RealX(*tokens->front());
    next_token();
  }
  else if (is_string()) {
    x = new StringX(*tokens->front());
    next_token();
  }
  else if (is_identifier()) {
    std::string id = identifier();
    if (match("(")) {
      if (match(")"))
	x = 0;
      else {
	x = parameters();
	need(")");
      }
      x = new Call(id, x);
    }
    else {
      x = new Variable(id);
      // only allow indexing after 
      for (;;) {
	if (match("[")) {
	  if (match(","))
	    x = new ColumnIndex(x, range());
	  else {
	    Code *y = range();
	    if (match(",")) {
	      if (check("]"))
		x = new RowIndex(x, y);
	      else
		x = new ColumnIndex(new RowIndex(x, y), range());
	    }
	    else
	      x = new Index(x, y);
	  }
	  need("]");
	}
	else
	  break;
      }
    }
  }
  else
    mpl_error("syntax error in expression");
  return x;
}

Code *exponential_expression() {
  Code *x = primary_expression();
  if (match("^"))
    return new Exponent(x, exponential_expression());
  return x;
}

Code *factor() {
  if (match("-"))
    return new Negate(factor());
  return exponential_expression();
}

Code *term() {
  Code *x = factor();
  for (;;) {
    if (match("*"))
      x = new Multiply(x, factor());
    else if (match("/"))
      x = new Divide(x, factor());
    else if (match("div"))
      x = new Floored_Divide(x, factor());
    else if (match("mod"))
      x = new Modulo(x, factor());
    else
      break;
  }
  return x;
}

Code *arithmetic_expression() {
  Code *x = term();
  for (;;) {
    if (match("+"))
      x = new Add(x, term());
    else if (match("-"))
      x = new Subtract(x, term());
    else
      break;
  }
  return x;
}

Code *relational_expression() {
  Code *x = arithmetic_expression();
  for (;;) {
    if (match("=="))
      x = new EQ(x, arithmetic_expression());
    else if (match("!="))
      x = new NE(x, arithmetic_expression());
    else if (match("<"))
      x = new LT(x, arithmetic_expression());
    else if (match("<="))
      x = new LE(x, arithmetic_expression());
    else if (match(">"))
      x = new GT(x, arithmetic_expression());
    else if (match(">="))
      x = new GE(x, arithmetic_expression());
    else
      break;
  }
  return x;
}

Code *logical_factor() {
  if (match("not"))
    return new Not(logical_factor());
  return relational_expression();
}

Code *logical_term() {
  Code *x = logical_factor();
  for (;;) {
    if (match("and"))
      x = new And(x, logical_factor());
    else
      break;
  }
  return x;
}

Code *expression() {
  Code *x = logical_term();
  for (;;) {
    if (match("or"))
      x = new Or(x, logical_term());
    else
      break;
  }
  return x;
}

Code *print_or_assign() {
  Code *code = expression();
  if (match("="))
    return new Assign(code, expression());
  return new Print(code);
}

Code *statements();

Code *if_statement() {
  Code *cond = expression();
  need("then");
  line_end();
  Code *true_part = statements();
  Code *false_part = 0;
  if (match("else")) {
    if (match("if"))
      false_part = if_statement();
    else {
      line_end();
      false_part = statements();
      need("end");
    }
  }
  else
    need("end");
  line_end();
  return new If(cond, true_part, false_part);
}

Code *while_statement() {
  Code *cond = expression();
  line_end();
  Code *body = statements();
  need("end");
  line_end();
  return new While(cond, body);
}

Code *repeat_statement() {
  line_end();
  Code *body = statements();
  need("until");
  Code *cond = expression();
  line_end();
  return new Repeat(cond, body);
}

// for var in range
// for var in set

Code *for_statement() {
  if (!is_identifier())
    mpl_error("syntax error in for statement");
  Code *x = new Variable(identifier()); // no subscripts allowed!
  need("in");
  Code *y = range();
  line_end();
  Code *z = statements();
  need("end");
  line_end();
  return new For(x, y, z);
}

Code *return_statement() {
  if (tokens->empty())
    return new Return(0);
  return new Return(expression());
}

Code *statement() {
  if (match("return"))
    return return_statement();
  if (match("if"))
    return if_statement();
  if (match("while"))
    return while_statement();
  if (match("repeat"))
    return repeat_statement();
  if (match("for"))
    return for_statement();
  return print_or_assign();
}

Code *statements() {
  next_line();
  if (at_end_line())
    return statements();
  if (check("end") || check("else") || check("until"))
    return 0;
  Code *stmt = statement();
  line_end();
  return new Statement(stmt, statements());
}

void formal_parameter_list() {
  if (is_identifier()) {
    do {
      std::string id = identifier();
      auto p = local_symbol_table.find(id);
      if (p == local_symbol_table.end()) {
	local_symbol_table[id] = -(++locals);
	// std::cout << "new parameter: " << id << ": " << local_symbol_table[id] << std::endl;
      }
      else {
	std::cout << "parameter \"" << id << "\" repeated in parameter list" << std::endl;
	exit(1);
      }
    } while (match(","));
  }
}

Code *define_function() {
  in_function = true;
  std::string id = identifier();
  locals = 0;
  need("(");
  formal_parameter_list();
  int params = locals;
  need(")");
  line_end();
  Code *body = statements();
  // print_code(body);
  need("end");
  line_end();
  delete function_table[id];
  function_table[id] = new User_Function(params, locals-params, body);
  local_symbol_table.clear();
  in_function = false;
  return 0;
}

Code *include() {
  return new Include(expression());
}

Code *compile() {
  if (!tokens->empty()) {
    if (match("include")) 
      return include();
    if (match("function"))
      return define_function();
    return print_or_assign();
  }
  return 0;
}

void command_loop() {
  while (!file_stack.empty()) {
    next_line();
    if (!tokens->empty()) {
      Code *cmd = compile();
      line_end();
      if (cmd) {
	cmd->execute();
	delete cmd;
      }
    }
    for (auto p : *tokens)
      delete p;
    delete tokens;
  }
}

void initialize_builtin_functions() {
  function_table["matvec"] = new Builtin_Function(1, mpl_matvec);

  function_table["stor"] = new Builtin_Function(1, mpl_stor);
  function_table["gfmt"] = new Builtin_Function(1, mpl_gfmt);
  
  function_table["exit"] = new Builtin_Function(1, mpl_exit);
  function_table["list"] = new Builtin_Function(1, mpl_list);
  function_table["vector"] = new Builtin_Function(1, mpl_vector);
  function_table["size"] = new Builtin_Function(1, mpl_size);
  function_table["concat"] = new Builtin_Function(2, mpl_concat);
  function_table["read"] = new Builtin_Function(0, mpl_read);
  function_table["eof"] = new Builtin_Function(0, mpl_eof);
  function_table["type"] = new Builtin_Function(1, mpl_type);
  function_table["ascii"] = new Builtin_Function(1, mpl_ascii);
  function_table["real"] = new Builtin_Function(1, mpl_real);
  function_table["imag"] = new Builtin_Function(1, mpl_imag);
  function_table["abs"] = new Builtin_Function(1, mpl_abs);
  function_table["arg"] = new Builtin_Function(1, mpl_arg);
  function_table["norm"] = new Builtin_Function(1, mpl_norm);
  function_table["conj"] = new Builtin_Function(1, mpl_conj);
  function_table["polar"] = new Builtin_Function(2, mpl_polar);

  function_table["sqrt"] = new Builtin_Function(1, mpl_sqrt);
  function_table["exp"] = new Builtin_Function(1, mpl_exp);
  function_table["expm1"] = new Builtin_Function(1, mpl_expm1);
  
  function_table["log"] = new Builtin_Function(1, mpl_log);
  function_table["log1p"] = new Builtin_Function(1, mpl_log1p);
  function_table["log10"] = new Builtin_Function(1, mpl_log10);
  
  function_table["sin"] = new Builtin_Function(1, mpl_sin);
  function_table["cos"] = new Builtin_Function(1, mpl_cos);
  function_table["tan"] = new Builtin_Function(1, mpl_tan);
  function_table["asin"] = new Builtin_Function(1, mpl_asin);
  function_table["acos"] = new Builtin_Function(1, mpl_acos);
  function_table["atan"] = new Builtin_Function(1, mpl_atan);
  function_table["atan2"] = new Builtin_Function(2, mpl_atan2);
  function_table["sinh"] = new Builtin_Function(1, mpl_sinh);
  function_table["cosh"] = new Builtin_Function(1, mpl_cosh);
  function_table["tanh"] = new Builtin_Function(1, mpl_tanh);
  function_table["asinh"] = new Builtin_Function(1, mpl_asinh);
  function_table["acosh"] = new Builtin_Function(1, mpl_acosh);
  function_table["atanh"] = new Builtin_Function(1, mpl_atanh);

  function_table["gamma"] = new Builtin_Function(1, mpl_gamma);
}

int main(int argc, char **argv) {
  initialize_builtin_functions();
  file_stack.push(new std::ifstream("stdlib.mpl"));
  command_loop();
  if (argc == 1) {
    file_stack.push(&std::cin);
    command_loop();
  }
  else {
    for (int i = 1; i < argc; i++) {
      file_stack.push(new std::ifstream(argv[i]));
      if (file_stack.top()->good())
	command_loop();
      else {
	mpl_error(std::string("unable to open: ") + argv[i]);
      }
    }
  }
  return 0;
}
