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
#include <regex>
#include <exception>
#include "value.hh"
#include "code.hh"
#include "function.hh"
#include "error.hh"
#include "mpl.hh"

const std::string stdlib = "/usr/local/share/mpl/stdlib.mpl";

typedef std::string Token;
typedef std::deque<Token> Token_List;

std::stack<File_Pointer> file_stack;
std::stack<std::string> file_name_stack;
std::stack<int> file_line_stack;

bool in_function = false;
int locals = 0;
int stack_pointer = 0;
int frame_pointer = 0;
std::vector<Value_ptr> memory;

Symbol_Table local_symbol_table;
Symbol_Table global_symbol_table;
Function_Table function_table;

bool trace_flag;

[[ noreturn ]] void mpl_error(std::string const &s) {
  throw mpl_exception {
    "error: "
    + file_name_stack.top() + ": "
    + std::to_string(file_line_stack.top()) + ": "
    + s
  };
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
  if (index < (unsigned)frame_pointer && typeid(*x) == typeid(Memory_Reference))
    mpl_error("writing reference to global variable");
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
    mpl_error("missing quote (\")");
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
      file_stack.pop();
      file_name_stack.pop();
      file_line_stack.pop();
    }

    // now read
    
    std::getline(*file_stack.top(), next);
    file_line_stack.top()++;
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

const std::regex identifier_pattern("[A-Za-z_$][A-Za-z0-9_$]*");

bool check_identifier(Token_List const &tokens) {
  if (tokens.empty())
    return false;
  return std::regex_match(tokens.front(), identifier_pattern);
}

const std::regex number_pattern("([0-9]+|([0-9]*\\.[0-9]+)([Ee][+-]?[0-9]+)?)|(#[0-9A-Fa-f]+)");

bool check_number(Token_List const &tokens) {
  if (tokens.empty())
    return false;
  return std::regex_match(tokens.front(), number_pattern);
}

const std::regex string_pattern("\"([^\"]|\\\\\")*\"");

bool check_string(Token_List const &tokens) {
  if (tokens.empty())
    return false;
  return std::regex_match(tokens.front(), string_pattern);
}

bool is_id_start_character(int c) {
  return isalpha(c) || c == '_' || c == '$';
}

bool is_id_character(int c) {
  return isalnum(c) || c == '_' || c == '$';
}

std::unique_ptr<Token_List> tokenize(std::string const &line) {
  auto tokens = std::make_unique<Token_List>();
  std::string s;

  auto p = line.begin();
  while (p != line.end()) {
    if (!isgraph(*p)) {  
      // skip whitespace
      p++;
    }
    else if (is_id_start_character(*p)) {
      // identifier
      s += *p++;
      while (is_id_character(*p))
        s += *p++;
      tokens->push_back(move(s));
    }
    else if (isdigit(*p) || *p == '.') {
      // number or '.'
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
      tokens->push_back(move(s));
    }
    else if (*p == '#') {
      // hex number
      s += *p++;
      while (isxdigit(*p))
        s += *p++;
      tokens->push_back(move(s));
    }
    else if (*p == '"') {
      // string
      s += *p++;
      while (p != line.end() && *p != '"') {
        if (*p == '\\') {
          s += *p++;
          if (p == line.end()) // clean_line() should prevents this, but check anyway
            s += '"';
          else
            s += *p++;
        }
        else
          s += *p++;
      }
      if (p == line.end())
        mpl_error("missing \"");
      s += *p++;
      tokens->push_back(move(s));
    }
    else {
      // character symbol
      char c = *p++;
      s += c;
      if (p != line.end()) {
        if (*p == '='
            && (c == '='
                || c == '!'
                || c == '<'
                || c == '>'))
          s += *p++;
      }
      tokens->push_back(move(s));
    }
  }
  return tokens;
}

void line_end(Token_List &tokens) {
  if (!tokens.empty())
    mpl_error("syntax error near " + tokens.front());
}

bool check(Token_List const &tokens, std::string const &s) {
  if (tokens.empty())
    return false;
  return tokens.front() == s;
}

bool match(Token_List &tokens, std::string const &s) {
  if (check(tokens, s)) {
    tokens.pop_front();
    return true;
  }
  return false;
}

void need(Token_List &tokens, std::string const &s) {
  if (!match(tokens, s)) {
    if (tokens.empty())
      mpl_error("missing " + s);
    else
      mpl_error("missing " + s + " near " + tokens.front());
  }
}

std::string identifier(Token_List &tokens) {
  if (tokens.empty()) {
    mpl_error("missing identifier");
    return "";
  }
  if (!check_identifier(tokens)) {
    mpl_error("missing identifier near " + tokens.front());
    return "";
  }
  std::string rv = tokens.front();
  tokens.pop_front();
  return rv;
}

Code *expression(Token_List &tokens);

// xlist() -- list, vector, or row of a matrix (not empty)

Code *xlist(Token_List &tokens) {
  Code *x = expression(tokens);
  if (match(tokens, ","))
    return new ListPart(x, xlist(tokens));
  return new ListPart(x, 0);
}

// rows of a matrix

Code *yylist(Token_List &tokens) {
  Code *x = xlist(tokens);
  if (tokens.empty()) {
    do
      tokens = *tokenize(read_line()).get(); // endless loop if missing extra lines at end of file
    while (tokens.empty());
    return new ListPart(x, yylist(tokens));
  }
  if (match(tokens, "|"))
    return new ListPart(x, yylist(tokens));
  return new ListPart(x, 0);
}

// ylist() -- either a vector or a matrix

Code *ylist(Token_List &tokens) {
  Code *x = xlist(tokens);
  if (tokens.empty()) {
    do
      tokens = *tokenize(read_line()).get(); // endless loop if missing extra lines at end of file
    while (tokens.empty());
    return new ListPart(x, yylist(tokens));
  }
  if (match(tokens, "|"))
    return new ListPart(x, yylist(tokens));
  return x;
}

Code *parameters(Token_List &tokens) {
  Code *param = expression(tokens);
  if (match(tokens, ","))
    return new Parameter(param, parameters(tokens));
  return new Parameter(param, 0);
}

Code *range(Token_List &tokens) {
  Code *x;
  Code *y = 0;
  Code *z = 0;

  x = expression(tokens);
  if (match(tokens, ":")) {
    y = expression(tokens);
    if (match(tokens, ":"))
      z = expression(tokens);
    return new Range(x, y, z);
  }
  return x;
}

Code *primary_expression(Token_List &tokens) {
  Code *x;
  
  if (match(tokens, "(")) {
    x = expression(tokens);
    if (match(tokens, ","))
      x = new ComplexX(x, expression(tokens));
    need(tokens, ")");
  }
  else if (match(tokens, "[")) {
    x = ylist(tokens);
    need(tokens, "]");
    return new Call("matvec", new Parameter(x, 0));
  }
  else if (match(tokens, "{")) {
    if (check(tokens, "}"))
      x = new ListX(0); // empty list
    else
      x = new ListX(xlist(tokens));
    need(tokens, "}");
  }
  else if (check_number(tokens)) {
    x = new RealX(tokens.front());
    tokens.pop_front();
  }
  else if (check_string(tokens)) {
    x = new StringX(tokens.front());
    tokens.pop_front();
  }
  else if (check_identifier(tokens)) {
    std::string id = identifier(tokens);
    if (match(tokens, "(")) {
      if (match(tokens, ")"))
	x = 0;
      else {
	x = parameters(tokens);
	need(tokens, ")");
      }
      x = new Call(id, x);
    }
    else {
      x = new Variable(id);
      // only allow indexing after 
      for (;;) {
	if (match(tokens, "[")) {
	  if (match(tokens, ","))
	    x = new ColumnIndex(x, range(tokens));
	  else {
	    Code *y = range(tokens);
	    if (match(tokens, ",")) {
	      if (check(tokens, "]"))
		x = new RowIndex(x, y);
	      else
		x = new ColumnIndex(new RowIndex(x, y), range(tokens));
	    }
	    else
	      x = new Index(x, y);
	  }
	  need(tokens, "]");
	}
	else
	  break;
      }
    }
  }
  else {
    if (tokens.empty())
      mpl_error("syntax error in expression");
    else
      mpl_error("syntax error in expression near " + tokens.front());
  }
  return x;
}

Code *exponential_expression(Token_List &tokens) {
  Code *x = primary_expression(tokens);
  if (match(tokens, "^"))
    return new Exponent(x, exponential_expression(tokens));
  return x;
}

Code *factor(Token_List &tokens) {
  if (match(tokens, "-"))
    return new Negate(factor(tokens));
  return exponential_expression(tokens);
}

Code *term(Token_List &tokens) {
  Code *x = factor(tokens);
  for (;;) {
    if (match(tokens, "*"))
      x = new Multiply(x, factor(tokens));
    else if (match(tokens, "/"))
      x = new Divide(x, factor(tokens));
    else if (match(tokens, "div"))
      x = new Floored_Divide(x, factor(tokens));
    else if (match(tokens, "mod"))
      x = new Modulo(x, factor(tokens));
    else if (match(tokens, ".")) {
      if (match(tokens, "*"))
	x = new Pointwise_Multiply(x, factor(tokens));
      else if (match(tokens, "/"))
	 x = new Pointwise_Divide(x, factor(tokens));
      else
	mpl_error("syntax error near " + tokens.front());
    }
    else
      break;
  }
  return x;
}

Code *arithmetic_expression(Token_List &tokens) {
  Code *x = term(tokens);
  for (;;) {
    if (match(tokens, "+"))
      x = new Add(x, term(tokens));
    else if (match(tokens, "-"))
      x = new Subtract(x, term(tokens));
    else
      break;
  }
  return x;
}

Code *relational_expression(Token_List &tokens) {
  Code *x = arithmetic_expression(tokens);
  for (;;) {
    if (match(tokens, "=="))
      x = new EQ(x, arithmetic_expression(tokens));
    else if (match(tokens, "!="))
      x = new NE(x, arithmetic_expression(tokens));
    else if (match(tokens, "<"))
      x = new LT(x, arithmetic_expression(tokens));
    else if (match(tokens, "<="))
      x = new LE(x, arithmetic_expression(tokens));
    else if (match(tokens, ">"))
      x = new GT(x, arithmetic_expression(tokens));
    else if (match(tokens, ">="))
      x = new GE(x, arithmetic_expression(tokens));
    else
      break;
  }
  return x;
}

Code *logical_factor(Token_List &tokens) {
  if (match(tokens, "not"))
    return new Not(logical_factor(tokens));
  if (match(tokens, "bnot"))
    return new BNot(logical_factor(tokens));
  return relational_expression(tokens);
}

Code *logical_term(Token_List &tokens) {
  Code *x = logical_factor(tokens);
  for (;;) {
    if (match(tokens, "and"))
      x = new And(x, logical_factor(tokens));
    else if (match(tokens, "band"))
      x = new BAnd(x, logical_factor(tokens));
    else
      break;
  }
  return x;
}

Code *expression(Token_List &tokens) {
  Code *x = logical_term(tokens);
  for (;;) {
    if (match(tokens, "or"))
      x = new Or(x, logical_term(tokens));
    else if (match(tokens, "bor"))
      x = new BOr(x, logical_term(tokens));
    else if (match(tokens, "bxor"))
      x = new BXor(x, logical_term(tokens));
    else
      break;
  }
  return x;
}

Code *print_or_assign(Token_List &tokens) {
  Code *code = expression(tokens);
  if (match(tokens, "="))
    return new Assign(code, expression(tokens));
  return new Print(code);
}

Code *statements(Token_List &tokens);

Code *if_statement(Token_List &tokens) {
  Code *cond = expression(tokens);
  need(tokens, "then");
  Code *true_part = statements(tokens);
  Code *false_part = 0;
  if (match(tokens, "else")) {
    if (match(tokens, "if"))
      false_part = if_statement(tokens);
    else {
      false_part = statements(tokens);
      need(tokens, "end");
    }
  }
  else
    need(tokens, "end");
  line_end(tokens);
  return new If(cond, true_part, false_part);
}

Code *while_statement(Token_List &tokens) {
  Code *cond = expression(tokens);
  Code *body = statements(tokens);
  need(tokens, "end");
  line_end(tokens);
  return new While(cond, body);
}

Code *repeat_statement(Token_List &tokens) {
  Code *body = statements(tokens);
  need(tokens, "until");
  Code *cond = expression(tokens);
  line_end(tokens);
  return new Repeat(cond, body);
}

// for var in range
// for var in set

Code *for_statement(Token_List &tokens) {
  Code *x = new Variable(identifier(tokens)); // no subscripts allowed!
  need(tokens, "in");
  Code *y = range(tokens);
  Code *z = statements(tokens);
  need(tokens, "end");
  line_end(tokens);
  return new For(x, y, z);
}

Code *return_statement(Token_List &tokens) {
  if (tokens.empty())
    return new Return(0);
  return new Return(expression(tokens));
}

Code *statement(Token_List &tokens) {
  if (match(tokens, "return"))
    return return_statement(tokens);
  if (match(tokens, "if"))
    return if_statement(tokens);
  if (match(tokens, "while"))
    return while_statement(tokens);
  if (match(tokens, "repeat"))
    return repeat_statement(tokens);
  if (match(tokens, "for"))
    return for_statement(tokens);
  return print_or_assign(tokens);
}

Code *statements(Token_List &tokens) {
  line_end(tokens);
  tokens = *tokenize(read_line()).get();
  if (tokens.empty())
    return statements(tokens);
  if (check(tokens, "end") || check(tokens, "else") || check(tokens, "until"))
    return 0;
  Code *stmt = statement(tokens);
  return new Statement(stmt, statements(tokens));
}

void formal_parameter_list(Token_List &tokens) {
  if (check_identifier(tokens)) {
    do {
      std::string id = identifier(tokens);
      auto p = local_symbol_table.find(id);
      if (p == local_symbol_table.end())
	local_symbol_table[id] = -(++locals);
      else
        mpl_error("parameter \"" + id + "\" repeated in parameter list");
    } while (match(tokens, ","));
  }
}

Code *define_function(Token_List &tokens) {
  in_function = true;
  std::string id = identifier(tokens);
  locals = 0;
  need(tokens, "(");
  formal_parameter_list(tokens);
  int params = locals;
  need(tokens, ")");
  Code *body = statements(tokens);
  need(tokens, "end");
  line_end(tokens);
  function_table[id] = Function_Pointer(new User_Function(params, locals-params, body));
  local_symbol_table.clear();
  in_function = false;
  return new Noop();
}

Code *include(Token_List &tokens) {
  return new Include(expression(tokens));
}

Code *compile(Token_List &tokens) {
  if (!tokens.empty()) {
    if (match(tokens, "include")) 
      return include(tokens);
    if (match(tokens, "function"))
      return define_function(tokens);
    return print_or_assign(tokens);
  }
  return new Noop();
}

Code *compile_command(std::unique_ptr<Token_List> tokens) {
  return compile(*tokens.get());
}

void command_loop() {
  while (!file_stack.empty()) {
    try {
      compile_command(tokenize(read_line()))->execute();
    } catch (mpl_exception const &e) {
      std::cerr << e.what() << std::endl;
    }
  }
}

void initialize_builtin_functions() {
  function_table["matvec"] = std::make_unique<Builtin_Function>(1, mpl_matvec);
  function_table["stor"] = std::make_unique<Builtin_Function>(1, mpl_stor);
  function_table["gfmt"] = std::make_unique<Builtin_Function>(1, mpl_gfmt);
  function_table["ffmt"] = std::make_unique<Builtin_Function>(2, mpl_ffmt);
  function_table["sfmt"] = std::make_unique<Builtin_Function>(2, mpl_sfmt);
  function_table["hfmt"] = std::make_unique<Builtin_Function>(1, mpl_hfmt);
  function_table["exit"] = std::make_unique<Builtin_Function>(1, mpl_exit);
  function_table["list"] = std::make_unique<Builtin_Function>(1, mpl_list);
  function_table["vector"] = std::make_unique<Builtin_Function>(1, mpl_vector);
  function_table["cvector"] = std::make_unique<Builtin_Function>(1, mpl_cvector);
  function_table["matrix"] = std::make_unique<Builtin_Function>(2, mpl_matrix);
  function_table["cmatrix"] = std::make_unique<Builtin_Function>(2, mpl_cmatrix);
  function_table["size"] = std::make_unique<Builtin_Function>(1, mpl_size);
  function_table["concat"] = std::make_unique<Builtin_Function>(2, mpl_concat);
  function_table["read"] = std::make_unique<Builtin_Function>(0, mpl_read);
  function_table["eof"] = std::make_unique<Builtin_Function>(0, mpl_eof);
  function_table["type"] = std::make_unique<Builtin_Function>(1, mpl_type);
  function_table["ascii"] = std::make_unique<Builtin_Function>(1, mpl_ascii);
  function_table["char"] = std::make_unique<Builtin_Function>(1, mpl_char);
  function_table["floor"] = std::make_unique<Builtin_Function>(1, mpl_floor);
  function_table["ceil"] = std::make_unique<Builtin_Function>(1, mpl_ceil);
  function_table["round"] = std::make_unique<Builtin_Function>(1, mpl_round);
  function_table["random"] = std::make_unique<Builtin_Function>(0, mpl_random);
  function_table["randomize"] = std::make_unique<Builtin_Function>(0, mpl_randomize);
  function_table["real"] = std::make_unique<Builtin_Function>(1, mpl_real);
  function_table["imag"] = std::make_unique<Builtin_Function>(1, mpl_imag);
  function_table["abs"] = std::make_unique<Builtin_Function>(1, mpl_abs);
  function_table["arg"] = std::make_unique<Builtin_Function>(1, mpl_arg);
  function_table["norm"] = std::make_unique<Builtin_Function>(1, mpl_norm);
  function_table["conj"] = std::make_unique<Builtin_Function>(1, mpl_conj);
  function_table["polar"] = std::make_unique<Builtin_Function>(2, mpl_polar);

  function_table["sqrt"] = std::make_unique<Builtin_Function>(1, mpl_sqrt);
  function_table["exp"] = std::make_unique<Builtin_Function>(1, mpl_exp);
  function_table["expm1"] = std::make_unique<Builtin_Function>(1, mpl_expm1);
  
  function_table["log"] = std::make_unique<Builtin_Function>(1, mpl_log);
  function_table["log1p"] = std::make_unique<Builtin_Function>(1, mpl_log1p);
  function_table["log10"] = std::make_unique<Builtin_Function>(1, mpl_log10);
  
  function_table["sin"] = std::make_unique<Builtin_Function>(1, mpl_sin);
  function_table["cos"] = std::make_unique<Builtin_Function>(1, mpl_cos);
  function_table["tan"] = std::make_unique<Builtin_Function>(1, mpl_tan);
  function_table["asin"] = std::make_unique<Builtin_Function>(1, mpl_asin);
  function_table["acos"] = std::make_unique<Builtin_Function>(1, mpl_acos);
  function_table["atan"] = std::make_unique<Builtin_Function>(1, mpl_atan);
  function_table["atan2"] = std::make_unique<Builtin_Function>(2, mpl_atan2);
  function_table["sinh"] = std::make_unique<Builtin_Function>(1, mpl_sinh);
  function_table["cosh"] = std::make_unique<Builtin_Function>(1, mpl_cosh);
  function_table["tanh"] = std::make_unique<Builtin_Function>(1, mpl_tanh);
  function_table["asinh"] = std::make_unique<Builtin_Function>(1, mpl_asinh);
  function_table["acosh"] = std::make_unique<Builtin_Function>(1, mpl_acosh);
  function_table["atanh"] = std::make_unique<Builtin_Function>(1, mpl_atanh);

  function_table["gamma"] = std::make_unique<Builtin_Function>(1, mpl_gamma);
  function_table["lgamma"] = std::make_unique<Builtin_Function>(1, mpl_lgamma);

  function_table["tr"] = std::make_unique<Builtin_Function>(1, mpl_tr);
}

int main(int argc, char **argv) {
  trace_flag = false;
  initialize_builtin_functions();

  file_stack.push(File_Pointer(new std::ifstream(stdlib)));
  file_name_stack.push("<standard library>");
  file_line_stack.push(0);
  
  command_loop();
  if (argc == 1) {
    file_stack.push(File_Pointer(&std::cin));
    file_name_stack.push("<stdin>");
    file_line_stack.push(0);
    command_loop();
  }
  else {
    for (int i = 1; i < argc; i++) {
      file_stack.push(File_Pointer(new std::ifstream(argv[i])));
      file_name_stack.push(argv[i]);
      file_line_stack.push(0);
      if (file_stack.top()->good())
	command_loop();
      else {
        std::cerr << "unable to open: " << argv[i] << std::endl;
      }
    }
  }
  return 0;
}
