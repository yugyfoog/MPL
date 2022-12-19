#include <iostream>
#include <sstream>
#include <list>
#include <map>
#include <stack>
#include <vector>
#include <algorithm>
#include <complex>
#include <valarray>
#include <memory>
#include <typeinfo>
#include "value.hh"
#include "code.hh"
#include "function.hh"
#include "mpl.hh"

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
  base = std::make_shared<std::valarray<double>>(s);
  indx = std::slice(0, s, 1);
}

// construct a vector[n] from a list of reals
// characterize already ensured that they are all reals

Vector::Vector(List *l, int n) {
  // We know that l is a list of Real
  base = std::make_shared<std::valarray<double>>(n);
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

CVector::CVector(int s) {
  base = std::make_shared<std::valarray<std::complex<double>>>(s);
  indx = std::slice(0, s, 1);
}

CVector::CVector(List *l, int n) {
  // l is a list of ONLY Real and Complex
  base = std::make_shared<std::valarray<std::complex<double>>>(n);
  indx = std::slice(0, n, 1);
  std::slice lindex = l->index();
  auto u = l->data();
  for (int i = 0; i < n; i++) {
    int j = lindex.stride()*i + lindex.start();
    if (typeid(*(*u.get())[j].get()) == typeid(Real))
      (*base)[i] = ((Real *)(*u.get())[j].get())->value();
    else if (typeid(*(*u.get())[j].get()) == typeid(Complex))
      (*base)[i] = ((Complex *)(*u.get())[j].get())->value();
    else
      mpl_error("internal error"); // we shoule never get here
  }
}

std::string CVector::print() const {
  std::ostringstream s;

  s << "[";
  std::valarray<std::complex<double>> *p = base.get();
  s << (*p)[indx.start()];
  for (int i = 1; i < (int)indx.size(); i++) {
    int ii = indx.stride()*i + indx.start();
    s << ", " << (*p)[ii];
  }
  s << "]";
  return s.str();
}

Matrix::Matrix(int r, int c) {
  base = std::make_shared<std::valarray<double>>(r*c);
  std::size_t lengths[] { (size_t)c, (size_t)r };
  std::size_t strides[] { (size_t)r, 1 };
  indx = std::gslice(0, std::valarray<std::size_t>(lengths, 2),
		     std::valarray<std::size_t>(strides, 2));
}
  

/*

   given  List {{1, 2, 3}, {4, 5, 6}}

start  = 0
size   = r,c (3,2)
stride = 1,c (1,3)

           +-----+-+-+
           |     | | |
       >---+-+-+ | | |
           | | | | | |
           | | | | | |
           | | | | | |
valarray  [1,2,3,4,5,6]

 */

Matrix::Matrix(List *l, int r, int c) {
  base = std::make_shared<std::valarray<double>>(r*c);
  
  // (row size, col size) i.e. (number of cols, number of rows)
  std::size_t lengths[] = {(size_t)c, (size_t)r};

  // (row stride, col stride)
  std::size_t strides[] = {1, (size_t)c};

  indx = std::gslice(0, std::valarray<std::size_t>(lengths,2),
		     std::valarray<std::size_t>(strides, 2));

  auto u = l->data();
  int ii = l->index().start();

  int k = 0;
  for (int i = 0; i < r; i++) {
    auto v = (List *)((*u.get()))[ii].get();
    int jj = v->index().start();
    // the list v may not fill entire row
    int j = 0;
    for (; j < (int)v->index().size(); j++) {
      (*base)[k++] = ((Real *)(*v->data())[jj].get())->value();
      jj += v->index().stride();
    }
    // fill row with 0
    for (; j < c; j++)
      (*base)[k++] = 0.0;
    ii += l->index().stride();
  }
}

void Matrix::transpose() {
  std::size_t lengths[2] { indx.size()[1], indx.size()[0] };
  std::size_t strides[2] { indx.stride()[1], indx.stride()[0] };
  indx = std::gslice(indx.start(), std::valarray<std::size_t>(lengths,2),
		     std::valarray<std::size_t>(strides, 2));
}

std::string Matrix::print() const {
  std::ostringstream s;

  std::valarray<double> *p = base.get();
  
  s << "[";

  int m = indx.size()[1] - 1;   // column size (number of rows)
  // print matrix

  int i = indx.start();
  int n = indx.size()[0] - 1;
  int j = i;
  s << (*p)[j];
  j += indx.stride()[0];
  while (n--) {
    s << ", " << (*p)[j];
    j += indx.stride()[0];
  }
  i += indx.stride()[1];
  while (m--) {
    n = indx.size()[0] - 1; // row size (number of columns)
    j = i;
    s << " | " << (*p)[j];
    j += indx.stride()[0];
    while (n--) {
      s << ", " << (*p)[j];
      j += indx.stride()[0]; // row stride
    }
    i += indx.stride()[1]; // column stride
  }
  s << "]";
  return s.str();
}

CMatrix::CMatrix(int r, int c) {
  base = std::make_shared<std::valarray<std::complex<double>>>(r*c);
  std::size_t lengths[] { (size_t)c, (size_t)r };
  std::size_t strides[] { (size_t)r, 1 };
  indx = std::gslice(0, std::valarray<std::size_t>(lengths, 2),
		     std::valarray<std::size_t>(strides, 2));
}

CMatrix::CMatrix(List *l, int r, int c) {
  base = std::make_shared<std::valarray<std::complex<double>>>(r*c);
  std::size_t lengths[] = {(size_t)c, (size_t)r};
  std::size_t strides[] = {1, (size_t)c};
  indx = std::gslice(0, std::valarray<std::size_t>(lengths,2),
		     std::valarray<std::size_t>(strides, 2));
  auto u = l->data();
  int ii = l->index().start();
  int k = 0;
  for (int i = 0; i < r; i++) {
    auto v = (List *)((*u.get()))[ii].get();
    int jj = v->index().start();
    int j = 0;
    for (; j < (int)v->index().size(); j++) {
      Value *vv = (*v->data())[jj].get();
      if (typeid(*vv) == typeid(Real))
	(*base)[k++] = ((Real *)vv)->value();
      else
	(*base)[k++] = ((Complex *)vv)->value();
      jj += v->index().stride();
    }
    for (; j < c; j++)
      (*base)[k++] = 0.0;
    ii += l->index().stride();
  }
}

void CMatrix::transpose() {
  std::size_t lengths[2] { indx.size()[1], indx.size()[0] };
  std::size_t strides[2] { indx.stride()[1], indx.stride()[0] };
  indx = std::gslice(indx.start(), std::valarray<std::size_t>(lengths,2),
		     std::valarray<std::size_t>(strides, 2));
}

std::string CMatrix::print() const {
  std::ostringstream s;
  std::valarray<std::complex<double>> *p = base.get();
  s << "[";
  int m = indx.size()[1] - 1;
  int i = indx.start();
  int n = indx.size()[0] - 1;
  int j = i;
  s << (*p)[j];
  j += indx.stride()[0];
  while (n--) {
    s << ", " << (*p)[j];
    j += indx.stride()[0];
  }
  i += indx.stride()[1];
  while (m--) {
    n = indx.size()[0] - 1;
    j = i;
    s << " | " << (*p)[j];
    j += indx.stride()[0];
    while (n--) {
      s << ", " << (*p)[j];
      j += indx.stride()[0];
    }
    i += indx.stride()[1];
  }
  s << "]";
  return s.str();
}

String::String(std::string const &s) {
  i = std::slice(0, s.length(), 1);
  base = std::make_shared<std::valarray<char>>(s.c_str(), s.length());
}

String::String(std::valarray<char> const &a, std::slice const &ai) {
  i = ai;
  base = std::make_shared<std::valarray<char>>(a[ai]);
}

std::string String::print() const  {
  std::string s;
  std::valarray<char> x = (*base)[i];
  
  for (auto p = begin(x); p != end(x); p++)
    s += *p;
  return s;
}

List::List() {
  base = std::make_shared<std::vector<Value_ptr>>();
  i = std::slice(0,0,1);
}

List::List(int n) {
  base = std::make_shared<std::vector<Value_ptr>>(n, std::make_shared<Real>(0.0));
  i = std::slice(0,n,1);
}

List::List(Value_ptr x) {
  base = std::make_shared<std::vector<Value_ptr>>(std::vector<Value_ptr>(1, x));
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
    if (index >= size)
      mpl_error("index out of range");
    s << p[index]->print();
    for (index += i.stride(), length--; length-- > 0; index += i.stride()) {
      if (index >= size)
	mpl_error("index out of range");
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

// is this function ever called?

std::string List_Member::print() const {
  mpl_error("internal error");
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
  /*  -- for statement ranges need sined values!!!
  if (_start < 0 || _size < 1 || _stride < 1)
    mpl_error("illegal slice");
  */
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

