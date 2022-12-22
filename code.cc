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
#include "value.hh"
#include "code.hh"
#include "function.hh"
#include "math.hh"
#include "mpl.hh"

Value_ptr value(Value_ptr u) {
  for (;;) {
    if (u == 0)
      mpl_error("void value");
    if (typeid(*u) == typeid(Memory_Reference))
      u = ((Memory_Reference *)u.get())->value();
    else if (typeid(*u) == typeid(List_Member))
      u = ((List_Member *)u.get())->value();
    else
      break;
  }
  return u;
}

Value_ptr Assign::execute() {
  Value_ptr v = value(rhs->execute());
  Value_ptr u = lhs->execute();
  assign(u.get(), v); // v gets saved somewhere so don't delete
  return 0;
}

Value_ptr Print::execute() {
  Value_ptr u = code->execute();
  if (u) {
    u = value(u);
    if (u == 0)
      mpl_error("undefined variable");
    if (typeid(*u) != typeid(Null))
      std::cout << u->print() << std::endl;
  }
  return 0;
}

Value_ptr Include::execute() {
  Value_ptr u = value(code->execute());
  std::string s = u->print();
  auto in = new std::ifstream(s);
  if (!in->good())
    mpl_error("unable to open " + s);
  file_stack.push(File_Pointer(new std::ifstream(s)));
  return 0;
}

Value_ptr Statement::execute() {
  Value_ptr rv = 0;
  
  if (stmt)
    rv = stmt->execute();
  if (rv == 0)
    if (next)
      rv = next->execute();
  return rv;
}

bool is_true(Value_ptr t) {
  if (typeid(*t) == typeid(Real)) {
    Real *u = (Real *)t.get();
    if (u->value() == 0.0)
      return false;
  }
  else
    mpl_error("only reals allowed in conditions");
  return true;
}

Value_ptr If::execute() {
  Value_ptr rv = 0;
  
  if (is_true(value(condition->execute())))
    rv = true_part->execute();
  else if (false_part)
    rv = false_part->execute();
  return rv;
}

Value_ptr While::execute() {
  Value_ptr rv = 0;

  while (is_true(value(condition->execute()))) {
    rv = body->execute();
    if (rv)
      return rv;
  }
  return rv;
}

Value_ptr Repeat::execute() {
  Value_ptr rv = 0;

  do {
    rv = body->execute();
    if (rv)
      return rv;
  }
  while (!is_true(condition->execute()));
  return rv;
}

Value_ptr For::execute() {
  Value_ptr rv = 0;
  Value_ptr var = variable->execute();
  Value_ptr cond = value(range->execute()); // This should be a List or Slice
  if (typeid(*cond.get()) == typeid(List)) {
    list_ptr lst = ((List *)cond.get())->data();
    std::slice slc = ((List *)cond.get())->index();
    int n = slc.size();
    for (int i = slc.start(); n--; i += slc.stride()) {
      assign(var.get(), (*lst.get())[i]);
      body->execute();
    }
  }
  else if (typeid(*cond.get()) == typeid(Slice)) {
    // don't use std::slice because size_t is unsigned!!!
    int start = ((Slice *)cond.get())->start();
    int size = ((Slice *)cond.get())->size();
    int stride = ((Slice *)cond.get())->stride();
    // ranges in for statements are different then ranges in indexes!
    for (int i = start; stride < 0 ? i >= size : i <= size; i += stride) {
      assign(var.get(), std::make_shared<Real>(i));
      body->execute();
    }
  }
  else
    mpl_error("illegal for statement");
  return rv;
}

Value_ptr Return::execute() {
  if (expr)
    return value(expr->execute());
  return std::make_shared<Null>();
}

void push_value(Value_ptr x) {
  unsigned i = stack_pointer++;
  write_memory(i, x);
}

Value_ptr Parameter::execute() {
  push_value(param->execute());
  
  // std::cout << "push" << std::endl;
  // std::cout << "    " << frame_pointer << ", " << stack_pointer << std::endl;
  
  if (next)
    next->execute();
  return 0;
}

RealX::RealX(std::string const &s) {
  if (s.front() == '#') {
    auto p = s.begin();
    p++;
    union {
      unsigned long i;
      double r;
    } u;
    u.i = 0;
    for (; p != s.end(); p++) {
      if (isdigit(*p))
	u.i = 16*u.i + (*p - '0');
      else if (isupper(*p))
	u.i = 16*u.i + (*p - 'A' + 10);
      else
	u.i = 16*u.i + (*p - 'a' + 10);
      
    }
    x = u.r;
  }
  else
    x = atof(s.c_str());
}

Value_ptr RealX::execute() {
  return std::make_shared<Real>(x);
}

Value_ptr ComplexX::execute() {
  Value_ptr r = value(real->execute());
  Value_ptr i = value(imag->execute());
  if ((typeid(*r) != typeid(Real)) || (typeid(*i) != typeid(Real)))
    mpl_error("illegal complex");
  Real *ru = (Real *)r.get();
  Real *iu = (Real *)i.get();
  return std::make_shared<Complex>(std::complex<double>(ru->value(), iu->value()));
}

void StringX::fix_string(std::string const &s) {
  auto p = begin(s);
  p++;
  while (*p != '"') {
    if (*p == '\\') {
      p++;
      switch (*p) {
      case 'a':
	str.push_back('\a');
	break;
      case 'b':
	str.push_back('\b');
	break;
      case 'e':
	str.push_back('\033');
	break;
      case 'f':
	str.push_back('\f');
	break;
      case 'n':
	str.push_back('\n');
	break;
      case 'r':
	str.push_back('\r');
	break;
      case 't':
	str.push_back('\t');
	break;
      case 'v':
	str.push_back('\v');
	break;
      default:
	str.push_back(*p);
	break;
      }
      p++;
    }
    else
      str.push_back(*p++);
  }
}

Value_ptr StringX::execute() {
  return std::make_shared<String>(str);
}

Value_ptr ListX::execute() {
  if (list)
    return list->execute();
  return std::make_shared<List>();
}

Value_ptr ListPart::execute() {
  Value_ptr x = value(member->execute());
  if (next == 0)
    return std::make_shared<List>(x);
  Value_ptr rest = next->execute();
  ((List *)rest.get())->add_to_front(x);
  return rest;
}

Variable::Variable(std::string const &s) {
  auto p = local_symbol_table.find(s);
  if (p == local_symbol_table.end()) {
    p = global_symbol_table.find(s);
    if (p == global_symbol_table.end())
      if (in_function) {
	local_symbol_table[s] = index = -(++locals);
      }
      else {
	global_symbol_table[s] = index = stack_pointer++;
      }
    else
      index = p->second;
  }
  else
    index = p->second;
}

Value_ptr Variable::execute() {
  return std::make_shared<Memory_Reference>(index);
}

Value_ptr ColumnIndex::execute() {
  Value_ptr a = value(base->execute());
  Value_ptr i = value(index->execute());
  return column_index(a.get(), i.get());
}

Value_ptr RowIndex::execute() {
  Value_ptr a = value(base->execute());
  Value_ptr i = value(index->execute());
  return row_index(a.get(), i.get());
}

Value_ptr Index::execute() {
  Value_ptr a = value(base->execute());
  Value_ptr i = value(index->execute());
  return simple_index(a.get(), i.get());
}

Value_ptr Range::execute() {
  Value_ptr strt = value(start->execute());
  Value_ptr len = value(length->execute());
  Value_ptr strd;
  if (stride == 0)
    strd = std::make_shared<Real>(1.0);
  else
    strd = value(stride->execute());
  return std::make_shared<Slice>(strt, len, strd);
}

Value_ptr Call::execute() {
  if (function_table.count(func) == 0)
    mpl_error(func + " is not defined");
  int save_frame_pointer = frame_pointer;
  int save_stack_pointer = stack_pointer;
  if (args)
    args->execute();

  frame_pointer = save_stack_pointer;
  
  if (trace_flag)
    std::cout << ">" << func << std::endl;
  
  Value_ptr rv = function_table[func]->execute(stack_pointer-frame_pointer);

  if (trace_flag)
    std::cout << "<" << func << std::endl;

  for (int i = save_stack_pointer; i < stack_pointer; i++)
    write_memory(i, 0);
  
  stack_pointer = save_stack_pointer;
  frame_pointer = save_frame_pointer;
  return rv;
}

Value_ptr Or::execute() {
  Value_ptr u = value(lhs->execute());
  Value_ptr v = value(rhs->execute());
  return logic_or(u.get(), v.get());
}

Value_ptr BOr::execute() {
  Value_ptr u = value(lhs->execute());
  Value_ptr v = value(rhs->execute());
  return bitwise_or(u.get(), v.get());
}

Value_ptr BXor::execute() {
  Value_ptr u = value(lhs->execute());
  Value_ptr v = value(rhs->execute());
  return bitwise_xor(u.get(), v.get());
}

Value_ptr And::execute() {
  Value_ptr u = value(lhs->execute());
  Value_ptr v = value(rhs->execute());
  return logic_and(u.get(), v.get());
}

Value_ptr BAnd::execute() {
  Value_ptr u = value(lhs->execute());
  Value_ptr v = value(rhs->execute());
  return bitwise_and(u.get(), v.get());
}

Value_ptr Not::execute() {
  Value_ptr u = value(code->execute());
  return logic_not(u.get());
}

Value_ptr BNot::execute() {
  Value_ptr u = value(code->execute());
  return bitwise_not(u.get());
}

Value_ptr EQ::execute() {
  Value_ptr u = value(lhs->execute());
  Value_ptr v = value(rhs->execute());
  return eq(u.get(), v.get());
}

Value_ptr NE::execute() {
  Value_ptr u = value(lhs->execute());
  Value_ptr v = value(rhs->execute());
  return ne(u.get(), v.get());
}

Value_ptr LT::execute() {
  Value_ptr u = value(lhs->execute());
  Value_ptr v = value(rhs->execute());
  return lt(u.get(), v.get());
}

Value_ptr LE::execute() {
  Value_ptr u = value(lhs->execute());
  Value_ptr v = value(rhs->execute());
  return le(u.get(), v.get());
}

Value_ptr GT::execute() {
  Value_ptr u = value(lhs->execute());
  Value_ptr v = value(rhs->execute());
  return gt(u.get(), v.get());
}

Value_ptr GE::execute() {
  Value_ptr u = value(lhs->execute());
  Value_ptr v = value(rhs->execute());
  return ge(u.get(), v.get());
}

Value_ptr Add::execute() {
  Value_ptr u = value(lhs->execute());
  Value_ptr v = value(rhs->execute());
  return add(u.get(), v.get());
}
 
Value_ptr Subtract::execute() {
  Value_ptr u = value(lhs->execute());
  Value_ptr v = value(rhs->execute());
  return subtract(u.get(), v.get());
}

Value_ptr Multiply::execute() {
  Value_ptr u = value(lhs->execute());
  Value_ptr v = value(rhs->execute());
  return multiply(u.get(), v.get());
}

Value_ptr Pointwise_Multiply::execute() {
  Value_ptr u = value(lhs->execute());
  Value_ptr v = value(rhs->execute());
  return pointwise_multiply(u.get(), v.get());
}

Value_ptr Divide::execute() {
  Value_ptr u = value(lhs->execute());
  Value_ptr v = value(rhs->execute());
  return divide(u.get(), v.get());
}

Value_ptr Pointwise_Divide::execute() {
  Value_ptr u = value(lhs->execute());
  Value_ptr v = value(rhs->execute());
  return pointwise_divide(u.get(), v.get());
}

Value_ptr Floored_Divide::execute() {
  Value_ptr u = value(lhs->execute());
  Value_ptr v = value(rhs->execute());
  return floored_divide(u.get(), v.get());
}

Value_ptr Modulo::execute() {
  Value_ptr u = value(lhs->execute());
  Value_ptr v = value(rhs->execute());
  return modulo(u.get(), v.get());
}

Value_ptr Negate::execute() {
  Value_ptr u = value(code->execute());
  return negate(u.get());
}

Value_ptr Exponent::execute() {
  Value_ptr u = value(lhs->execute());
  Value_ptr v = value(rhs->execute());
  return power(u.get(), v.get());
}

