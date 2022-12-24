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

Code_Pointer expression(Token_List &tokens);

// xlist() -- list, vector, or row of a matrix (not empty)

Code_Pointer xlist(Token_List &tokens) {
  Code_Pointer x = expression(tokens);
  if (match(tokens, ","))
    return Code_Pointer(new ListPart(std::move(x), std::move(xlist(tokens))));
  return Code_Pointer(new ListPart(std::move(x), nullptr));
}

// rows of a matrix

Code_Pointer yylist(Token_List &tokens) {
  Code_Pointer x = xlist(tokens);
  if (tokens.empty()) {
    do
      tokens = *tokenize(read_line()).get(); // endless loop if missing extra lines at end of file
    while (tokens.empty());
    return Code_Pointer(new ListPart(std::move(x), std::move(yylist(tokens))));
  }
  if (match(tokens, "|"))
    return Code_Pointer(new ListPart(std::move(x), std::move(yylist(tokens))));
  return Code_Pointer(new ListPart(std::move(x), nullptr));
}

// ylist() -- either a vector or a matrix

Code_Pointer ylist(Token_List &tokens) {
  Code_Pointer x = xlist(tokens);
  if (tokens.empty()) {
    do
      tokens = *tokenize(read_line()).get(); // endless loop if missing extra lines at end of file
    while (tokens.empty());
    return Code_Pointer(new ListPart(std::move(x), std::move(yylist(tokens))));
  }
  if (match(tokens, "|"))
    return Code_Pointer(new ListPart(std::move(x), std::move(yylist(tokens))));
  return x;
}

Code_Pointer parameters(Token_List &tokens) {
  Code_Pointer param = expression(tokens);
  if (match(tokens, ","))
    return Code_Pointer(new Parameter(std::move(param), std::move(parameters(tokens))));
  return Code_Pointer(new Parameter(std::move(param), nullptr));
}

Code_Pointer range(Token_List &tokens) {
  Code_Pointer y = nullptr;
  Code_Pointer z = nullptr;

  Code_Pointer x = expression(tokens);
  if (match(tokens, ":")) {
    y = expression(tokens);
    if (match(tokens, ":"))
      z = expression(tokens);
    return Code_Pointer(new Range(std::move(x), std::move(y), std::move(z)));
  }
  return x;
}

Code_Pointer primary_expression(Token_List &tokens) {
  if (match(tokens, "(")) {
    Code_Pointer x = expression(tokens);
    if (match(tokens, ","))
      x = Code_Pointer(new ComplexX(std::move(x), std::move(expression(tokens))));
    need(tokens, ")");
    return x;
  }
  if (match(tokens, "[")) {
    Code_Pointer x = ylist(tokens);
    need(tokens, "]");
    return Code_Pointer(new Call("matvec", Code_Pointer(new Parameter(std::move(x), nullptr))));
  }
  if (match(tokens, "{")) {
    Code_Pointer x = nullptr;
    if (check(tokens, "}"))
      x = Code_Pointer(new ListX(nullptr)); // empty list
    else
      x = Code_Pointer(new ListX(xlist(tokens)));
    need(tokens, "}");
    return x;
  }
  if (check_number(tokens)) {
    Code_Pointer x = Code_Pointer(new RealX(tokens.front()));
    tokens.pop_front();
    return x;
  }
  if (check_string(tokens)) {
    Code_Pointer x = Code_Pointer(new StringX(tokens.front()));
    tokens.pop_front();
    return x;
  }
  if (check_identifier(tokens)) {
    std::string id = identifier(tokens);
    if (match(tokens, "(")) {
      Code_Pointer x = nullptr;
      if (!match(tokens, ")")) {
	x = parameters(tokens);
        need(tokens, ")");
      }
      return Code_Pointer(new Call(id, std::move(x)));
    }
    Code_Pointer x = Code_Pointer(new Variable(id));
    // only allow indexing after variable
    for (;;) {
      if (match(tokens, "[")) {
        if (match(tokens, ","))
          x = Code_Pointer(new ColumnIndex(std::move(x), std::move(range(tokens))));
        else {
          Code_Pointer y = range(tokens);
          if (match(tokens, ",")) {
            if (check(tokens, "]"))
              x = Code_Pointer(new RowIndex(std::move(x), std::move(y)));
            else
              x = Code_Pointer(new ColumnIndex(Code_Pointer(new RowIndex(std::move(x),
                                                                         std::move(y))),
                                               std::move(range(tokens))));
          }
          else
            x = Code_Pointer(new Index(std::move(x), std::move(y)));
        }
        need(tokens, "]");
      }
      else
        break;
    }
    return x;
  }
  if (tokens.empty())
    mpl_error("syntax error in expression");
  mpl_error("syntax error in expression near " + tokens.front());
}

Code_Pointer exponential_expression(Token_List &tokens) {
  Code_Pointer x = primary_expression(tokens);
  if (match(tokens, "^"))
    return Code_Pointer(new Exponent(std::move(x), std::move(exponential_expression(tokens))));
  return x;
}

Code_Pointer factor(Token_List &tokens) {
  if (match(tokens, "-"))
    return Code_Pointer(new Negate(factor(tokens)));
  return exponential_expression(tokens);
}

Code_Pointer term(Token_List &tokens) {
  Code_Pointer x = factor(tokens);
  for (;;) {
    if (match(tokens, "*"))
      x = Code_Pointer(new Multiply(std::move(x), std::move(factor(tokens))));
    else if (match(tokens, "/"))
      x = Code_Pointer(new Divide(std::move(x), std::move(factor(tokens))));
    else if (match(tokens, "div"))
      x = Code_Pointer(new Floored_Divide(std::move(x), std::move(factor(tokens))));
    else if (match(tokens, "mod"))
      x = Code_Pointer(new Modulo(std::move(x), std::move(factor(tokens))));
    else if (match(tokens, ".")) {
      if (match(tokens, "*"))
	x = Code_Pointer(new Pointwise_Multiply(std::move(x), std::move(factor(tokens))));
      else if (match(tokens, "/"))
        x = Code_Pointer(new Pointwise_Divide(std::move(x), std::move(factor(tokens))));
      else
	mpl_error("syntax error near " + tokens.front());
    }
    else
      break;
  }
  return x;
}

Code_Pointer arithmetic_expression(Token_List &tokens) {
  Code_Pointer x = term(tokens);
  for (;;) {
    if (match(tokens, "+"))
      x = Code_Pointer(new Add(std::move(x), std::move(term(tokens))));
    else if (match(tokens, "-"))
      x = Code_Pointer(new Subtract(std::move(x), std::move(term(tokens))));
    else
      break;
  }
  return x;
}

Code_Pointer relational_expression(Token_List &tokens) {
  Code_Pointer x = arithmetic_expression(tokens);
  for (;;) {
    if (match(tokens, "=="))
      x = Code_Pointer(new EQ(std::move(x), std::move(arithmetic_expression(tokens))));
    else if (match(tokens, "!="))
      x = Code_Pointer(new NE(std::move(x), std::move(arithmetic_expression(tokens))));
    else if (match(tokens, "<"))
      x = Code_Pointer(new LT(std::move(x), std::move(arithmetic_expression(tokens))));
    else if (match(tokens, "<="))
      x = Code_Pointer(new LE(std::move(x), std::move(arithmetic_expression(tokens))));
    else if (match(tokens, ">"))
      x = Code_Pointer(new GT(std::move(x), std::move(arithmetic_expression(tokens))));
    else if (match(tokens, ">="))
      x = Code_Pointer(new GE(std::move(x), std::move(arithmetic_expression(tokens))));
    else
      break;
  }
  return x;
}

Code_Pointer logical_factor(Token_List &tokens) {
  if (match(tokens, "not"))
    return Code_Pointer(new Not(std::move(logical_factor(tokens))));
  if (match(tokens, "bnot"))
    return Code_Pointer(new BNot(std::move(logical_factor(tokens))));
  return relational_expression(tokens);
}

Code_Pointer logical_term(Token_List &tokens) {
  Code_Pointer x = logical_factor(tokens);
  for (;;) {
    if (match(tokens, "and"))
      x = Code_Pointer(new And(std::move(x), std::move(logical_factor(tokens))));
    else if (match(tokens, "band"))
      x = Code_Pointer(new BAnd(std::move(x), std::move(logical_factor(tokens))));
    else
      break;
  }
  return x;
}

Code_Pointer expression(Token_List &tokens) {
  Code_Pointer x = logical_term(tokens);
  for (;;) {
    if (match(tokens, "or"))
      x = Code_Pointer(new Or(std::move(x), std::move(logical_term(tokens))));
    else if (match(tokens, "bor"))
      x = Code_Pointer(new BOr(std::move(x), std::move(logical_term(tokens))));
    else if (match(tokens, "bxor"))
      x = Code_Pointer(new BXor(std::move(x), std::move(logical_term(tokens))));
    else
      break;
  }
  return x;
}

Code_Pointer print_or_assign(Token_List &tokens) {
  Code_Pointer code = expression(tokens);
  if (match(tokens, "="))
    return Code_Pointer(new Assign(std::move(code), std::move(expression(tokens))));
  return Code_Pointer(new Print(std::move(code)));
}

Code_Pointer statements(Token_List &tokens);

Code_Pointer if_statement(Token_List &tokens) {
  Code_Pointer cond = expression(tokens);
  need(tokens, "then");
  Code_Pointer true_part = statements(tokens);
  Code_Pointer false_part = 0;
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
  return Code_Pointer(new If(std::move(cond), std::move(true_part), std::move(false_part)));
}

Code_Pointer while_statement(Token_List &tokens) {
  Code_Pointer cond = expression(tokens);
  Code_Pointer body = statements(tokens);
  need(tokens, "end");
  line_end(tokens);
  return Code_Pointer(new While(std::move(cond), std::move(body)));
}

Code_Pointer repeat_statement(Token_List &tokens) {
  Code_Pointer body = statements(tokens);
  need(tokens, "until");
  Code_Pointer cond = expression(tokens);
  line_end(tokens);
  return Code_Pointer(new Repeat(std::move(cond), std::move(body)));
}

// for var in range
// for var in set

Code_Pointer for_statement(Token_List &tokens) {
  Code_Pointer x = Code_Pointer(new Variable(std::move(identifier(tokens)))); // no subscripts allowed!
  need(tokens, "in");
  Code_Pointer y = range(tokens);
  Code_Pointer z = statements(tokens);
  need(tokens, "end");
  line_end(tokens);
  return Code_Pointer(new For(std::move(x), std::move(y), std::move(z)));
}

Code_Pointer return_statement(Token_List &tokens) {
  if (tokens.empty())
    return Code_Pointer(new Return(nullptr));
  return Code_Pointer(new Return(std::move(expression(tokens))));
}

Code_Pointer statement(Token_List &tokens) {
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

Code_Pointer statements(Token_List &tokens) {
  line_end(tokens);
  tokens = *tokenize(read_line()).get();
  if (tokens.empty())
    return statements(tokens);
  if (check(tokens, "end") || check(tokens, "else") || check(tokens, "until"))
    return nullptr;
  Code_Pointer stmt = statement(tokens);
  return Code_Pointer(new Statement(std::move(stmt), std::move(statements(tokens))));
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

Code_Pointer define_function(Token_List &tokens) {
  in_function = true;
  std::string id = identifier(tokens);
  locals = 0;
  need(tokens, "(");
  formal_parameter_list(tokens);
  int params = locals;
  need(tokens, ")");
  Code_Pointer body = statements(tokens);
  need(tokens, "end");
  line_end(tokens);
  function_table[id] = Function_Pointer(new User_Function(params, locals-params, std::move(body)));
  local_symbol_table.clear();
  in_function = false;
  return Code_Pointer(new Noop());
}

Code_Pointer include(Token_List &tokens) {
  return Code_Pointer(new Include(std::move(expression(tokens))));
}

Code_Pointer compile(Token_List &tokens) {
  if (!tokens.empty()) {
    if (match(tokens, "include")) 
      return include(tokens);
    if (match(tokens, "function"))
      return define_function(tokens);
    return print_or_assign(tokens);
  }
  return Code_Pointer(new Noop());
}

Code_Pointer compile_command(std::unique_ptr<Token_List> tokens) {
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
