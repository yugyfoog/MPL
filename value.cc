#include <iostream>
#include <sstream>
#include <list>
#include <map>
#include <stack>
#include <vector>
#include <complex>
#include <valarray>
#include <memory>
#include "value.hh"
#include "mpl.hh"

std::string type_name(Value *x) {
  if (typeid(*x) == typeid(Real))
    return "real";
  if (typeid(*x) == typeid(Complex))
    return "complex";
  if (typeid(*x) == typeid(Vector))
    return "vector";
  if (typeid(*x) == typeid(CVector))
    return "complex vector";
  if (typeid(*x) == typeid(Matrix))
    return "matrix";
  if (typeid(*x) == typeid(CMatrix))
    return "complex matrix";
  if (typeid(*x) == typeid(String))
    return "string";
  if (typeid(*x) == typeid(List))
    return "list";
  if (typeid(*x) == typeid(List_Member))
    return "list member";
  if (typeid(*x) == typeid(Slice))
    return "slice";
  if (typeid(*x) == typeid(Memory_Reference))
    return "memory reference";
  if (typeid(*x) == typeid(Value))
    return "value";
  return "unknown";
}

Value::~Value() { }

std::string Real::print() const {
  std::ostringstream s;
  s << (*base)[indx];
  return s.str();
}

std::string Complex::print() const {
  std::ostringstream s;
  s << (*base)[indx];
  return s.str();
}

Vector::Vector(int s) {
  base = double_ptr(new std::valarray<double>(s));
  indx = std::slice(0, s, 1);
}

/*
Vector::Vector(double_ptr x) {
  base = x;
  indx = std::slice(0, base->size(), 1);
}
*/

// construct a vector[n] from a list of reals
// characterize already ensured that they are all reals

Vector::Vector(List *l, int n) {
  // We know the l is a list of Real
  base = double_ptr(new std::valarray<double>(n));
  indx = std::slice(0, n, 1);
  std::slice j = l->index();
  auto u = l->data();  
  for (int i = 0; i < n; i++) {
    int jj = j.stride()*i + j.start();
    Real *x = (Real *)(*u.get())[jj].get();
    (*base)[i] = x->value();
  }
}

std::string Vector::print() const {
  std::ostringstream s;

  s << "[";
  std::valarray<double> *p = base.get();
  s << (*p)[indx.start()];
  for (int i = 1; i < (int)indx.size(); i++) {
    int ii = indx.stride()*i + indx.start();
    s << ", " << (*p)[ii];
  }
  s << "]";
  return s.str();
}

CVector::CVector(List *l, int i) {
  XXX();
}

std::string CVector::print() const {
  XXX();
  return 0;
}

Matrix::Matrix(List *l, int r, int c) {
  XXX();
}

std::string Matrix::print() const {
  XXX();
  return 0;
}

CMatrix::CMatrix(List *l, int r, int c) {
  XXX();
}

std::string CMatrix::print() const {
  XXX();
  return 0;
}

String::String(std::string const &s) {
  i = std::slice(0, s.length(), 1);
  base = string_ptr(new std::valarray<char>(s.c_str(), s.length()));
}

String::String(std::valarray<char> const &a, std::slice const &ai) {
  i = ai;
  base = string_ptr(new std::valarray<char>(a[ai]));
}

std::string String::print() const  {
  std::string s;
  std::valarray<char> x = (*base)[i];
  
  for (auto p = begin(x); p != end(x); p++)
    s += *p;
  return s;
}

List::List() {
  base = list_ptr(new std::vector<Value_ptr>);
  i = std::slice(0,0,1);
}

List::List(int n) {
  base = list_ptr(new std::vector<Value_ptr>(n, Value_ptr(new Real(0.0))));
  i = std::slice(0,n,1);
}

List::List(Value_ptr x) {
  base = list_ptr(new std::vector<Value_ptr>(1, x));
  i = std::slice(0,1,1);
}

std::string List::print() const {
  std::ostringstream s;

  s << "{";
  int size = base->size();
  auto p = base->data();
  if (i.size() > 0) {
    int index = i.start();
    int length = i.size();
    if (index >= size) {
      std::cout << "index out of range" << std::endl;
      exit(1);
    }
    s << p[index]->print();
    for (index += i.stride(), length--; length-- > 0; index += i.stride()) {
      if (index >= size) {
	std::cout << "index out of range" << std::endl;
	exit(1);
      }
      s << ", " << p[index]->print();
    }
  }
  s << "}";
  return s.str();
}

void List::add_to_front(Value_ptr x) {
  i = std::slice(i.start(), i.size()+1, i.stride());
  base->insert(base->begin(), x);
}

std::string List_Member::print() const {
  XXX();
  return 0;
}

Slice::Slice(Value_ptr strt, Value_ptr len, Value_ptr strd) {
  if (typeid(*strt) != typeid(Real))
    mpl_error("type error");
  if (typeid(*len) != typeid(Real))
    mpl_error("type error");
  if (typeid(*strd) != typeid(Real))
    mpl_error("type error");
  Real *sr = (Real *)strt.get();
  Real *sz = (Real *)len.get();
  Real *sd = (Real *)strd.get();
  _start = round(sr->value());
  _size = round(sz->value());
  _stride = round(sd->value());
}

// this should never get called

std::string Slice::print() const {
  std::ostringstream s;
  s << "slice<" << _start << ":" << _size << ":" << _stride << ">" << std::endl;
  return s.str();
}

Memory_Reference::Memory_Reference(int i) {
  if (i < 0)
    index = frame_pointer - i - 1;
  else
    index = i;
}

void Memory_Reference::assign(Value_ptr u) {
  Value_ptr x = read_memory(index);
  if ((x != 0) && (typeid(*x) == typeid(Memory_Reference)))
    ((Memory_Reference *)x.get())->assign(u);
  else
    write_memory(index, u);
}

Value_ptr Memory_Reference::value() {
  return read_memory(index);
}

std::string Memory_Reference::print() const {
  std::ostringstream s;
  s << "M[" << index << "]";
  return s.str();
  //  return read_memory(index)->print();
}

