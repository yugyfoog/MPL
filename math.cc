#include <iostream>
#include <string>
#include <algorithm>
#include <list>
#include <map>
#include <stack>
#include <vector>
#include <valarray>
#include <complex>
#include <memory>
#include <typeinfo>
#include <numeric>
#include <cmath>
#include "value.hh"
#include "math.hh"
#include "mpl.hh"

typedef Value_ptr Doit1(Value *);
typedef Value_ptr Doit2(Value *, Value *);

// generic unary operations on a list

Value_ptr dolist(List *u, Doit1 *func) {  
  std::slice index = u->index();
  list_ptr data = u->data();
  int n = index.size();
  auto w = new std::vector<Value_ptr>(n);
  for (int i = 0; i < n; i++)
    (*w)[i] = func((*data)[index.stride()*i + index.start()].get());
  return Value_ptr(new List(list_ptr(w), std::slice(0, n, 1)));
}

// generic binary operations with lists

Value_ptr dolist(Value *u, Value *v, Doit2 *func) {
  if (typeid(*u) == typeid(List)) {
    if (typeid(*v) == typeid(List)) {
      std::slice uindex =  ((List *)u)->index();
      std::slice vindex =  ((List *)v)->index();
      list_ptr udata = ((List *)u)->data();
      list_ptr vdata = ((List *)v)->data();
      int n = std::max(uindex.size(), vindex.size());
      auto w = new std::vector<Value_ptr>(n);
      for (int i = 0; i < n; i++) {
	int j = (i%uindex.size())*uindex.stride() + uindex.start();
	int k = (i%vindex.size())*vindex.stride() + vindex.start();
	(*w)[i] = func((*udata)[j].get(), (*vdata)[k].get());
      }
      return Value_ptr(new List(list_ptr(w), std::slice(0, n, 1)));
    }
    else {
      std::slice index =  ((List *)u)->index();
      list_ptr data = ((List *)u)->data();
      auto w = new std::vector<Value_ptr>(index.size());
      for (int i = 0, j = index.start(); i < (int)index.size(); i++, j += index.stride())
	(*w)[i] = func((*data)[j].get(), v);
      return Value_ptr(new List(list_ptr(w), std::slice(0, index.size(), 1)));
    }
  }
  else if (typeid(*v) == typeid(List)) {
    std::slice index = ((List *)v)->index();
    list_ptr data = ((List *)v)->data();
    auto w = new std::vector<Value_ptr>(index.size());
    for (int i = 0, j = index.start(); i < (int)index.size(); i++, j += index.stride())
      (*w)[i] = func(u, (*data)[j].get());
    return Value_ptr(new List(list_ptr(w), std::slice(0, index.size(), 1)));
  }
  mpl_error("interanl error");
  return 0;
}

Real *logic_or(Real *u, Real *v) {
  return new Real(u->value() != 0.0 || v->value() != 0.0);
}

Value_ptr logic_or(Value *u, Value *v) {
  if (typeid(*u) == typeid(List) || typeid(*v) == typeid(List))
    return dolist(u, v, logic_or);
  if (typeid(*u) == typeid(Real) && typeid(*v) == typeid(Real))
    return Value_ptr(logic_or((Real *)u, (Real *)v));
  mpl_error("type error");
  return 0;
}

Real *logic_and(Real *u, Real *v) {
  return new Real(u->value() != 0.0 && v->value() != 0.0);
}

Value_ptr logic_and(Value *u, Value *v) {
  if (typeid(*u) == typeid(List) || typeid(*v) == typeid(List))
    return dolist(u, v, logic_and);
  if (typeid(*u) == typeid(Real) && typeid(*v) == typeid(Real))
    return Value_ptr(logic_and((Real *)u, (Real *)v));
  mpl_error("type error");
  return 0;
}

Real *logic_not(Real *u) {
  return new Real(u->value() == 0.0);
}

Value_ptr logic_not(Value *u) {
  if (typeid(*u) == typeid(List))
    return dolist((List *)u, logic_not);
  if (typeid(*u) == typeid(Real))
    return Value_ptr(logic_not((Real *)u));
  mpl_error("type error");
  return 0;
}

Real *eq(Real *u, Real *v) {
  return new Real(u->value() == v->value());
}

Real *eq(Real *u, Complex *v) {
  return new Real(u->value() == v->value());
}

Real *eq(Complex *u, Complex *v) {
  return new Real(u->value() == v->value());
}

Real *eq(Vector *u, Vector *v) {
  if (u->index().size() != v->index().size())
    return new Real(0.0);
  int n = u->index().size();
  int i = u->index().start();
  int j = v->index().start();
  while (n--) {
    if ((*u->data())[i] != (*v->data())[j])
      return new Real(0.0);
    i += u->index().stride();
    j += v->index().stride();
  }
  return new Real(1.0);
}

Real *eq(Vector *u, CVector *v) {
  if (u->index().size() != v->index().size())
    return new Real(0.0);
  int n = u->index().size();
  int i = u->index().start();
  int j = v->index().start();
  while (n--) {
    if ((*u->data())[i] != (*v->data())[j])
      return new Real(0.0);
    i += u->index().stride();
    j += v->index().stride();
  }
  return new Real(1.0);
}

Real *eq(CVector *u, CVector *v) {
  if (u->index().size() != v->index().size())
    return new Real(0.0);
  int n = u->index().size();

  int i = u->index().start();
  int j = v->index().start();
  while(n--) {
    if ((*u->data())[i]  != (*v->data())[j])
      return new Real(0.0);
    i += u->index().stride();
    j += v->index().stride();
  }
  return new Real(1.0);
}

Real *eq(Matrix *u, Matrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();

  if (ui.size()[0] != vi.size()[0] || ui.size()[1] != vi.size()[1])
    return new Real(0.0);

  int cols = ui.size()[0];
  int rows = vi.size()[1];

  int ii = ui.start();
  int jj = vi.start();
  
  int m = cols;
  while (m--) {
    int n = rows;

    int i = ii;
    int j = jj;
    while (n--) {
      if ((*u->data())[i] != (*v->data())[j])
	return new Real(0.0);
      i += u->index().stride()[1];
      j += v->index().stride()[1];
    }
    ii += u->index().stride()[0];
    jj += v->index().stride()[0];
  }
  return new Real(1.0);
}

Real *eq(Matrix *u, CMatrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();

  if (ui.size()[0] != vi.size()[0] || ui.size()[1] != vi.size()[1])
    return new Real(0.0);

  int cols = ui.size()[0];
  int rows = vi.size()[1];

  int ii = ui.start();
  int jj = vi.start();
  
  int m = cols;
  while (m--) {
    int n = rows;

    int i = ii;
    int j = jj;
    while (n--) {
      if ((*u->data())[i] != (*v->data())[j])
	return new Real(0.0);
      i += u->index().stride()[1];
      j += v->index().stride()[1];
    }
    ii += u->index().stride()[0];
    jj += v->index().stride()[0];
  }
  return new Real(1.0);
}
  
Real *eq(CMatrix *u, CMatrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();

  if (ui.size()[0] != vi.size()[0] || ui.size()[1] != vi.size()[1])
    return new Real(0.0);

  int cols = ui.size()[0];
  int rows = vi.size()[1];

  int ii = ui.start();
  int jj = vi.start();
  
  int m = cols;
  while (m--) {
    int n = rows;

    int i = ii;
    int j = jj;
    while (n--) {
      if ((*u->data())[i] != (*v->data())[j])
	return new Real(0.0);
      i += u->index().stride()[1];
      j += v->index().stride()[1];
    }
    ii += u->index().stride()[0];
    jj += v->index().stride()[0];
  }
  return new Real(1.0);
}
  
Real *eq(String *u, String *v) { 
  int i, j, n;

  if (u->index().size() != v->index().size())
    return new Real(0.0);
  for (i = u->index().start(), j = v->index().start(), n = u->index().size();
       n > 0;
       n--, i += u->index().stride(), j += v->index().stride()) {
    if ((*u->data())[i] != (*v->data())[j])
      return new Real(0.0);
  }
  return new Real(1.0);
}

Value_ptr eq(Value *u, Value *v) {
  if (typeid(*u) == typeid(List) || typeid(*v) == typeid(List))
    return dolist(u, v, eq);
  if (typeid(*u) == typeid(Real)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(eq((Real *)u, (Real *)v));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(eq((Real *)u, (Complex *)v));
  }
  else if (typeid(*u) == typeid(Complex)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(eq((Real *)v, (Complex *)u));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(eq((Complex *)u, (Complex *)v));
  }
  else if (typeid(*u) == typeid(Vector)) {
    if (typeid(*v) == typeid(Vector))
      return Value_ptr(eq((Vector *)u, (Vector *)v));
    if (typeid(*v) == typeid(CVector))
      return Value_ptr(eq((Vector *)u, (CVector *)v));
  }
  else if (typeid(*u) == typeid(CVector)) {
    if (typeid(*v) == typeid(Vector))
      return Value_ptr(eq((Vector *)v, (CVector *)u));
    if (typeid(*v) == typeid(CVector))
      return Value_ptr(eq((CVector *)u, (CVector *)v));
  }
  else if (typeid(*u) == typeid(Matrix)) {
    if (typeid(*v) == typeid(Matrix))
      return Value_ptr(eq((Matrix *)u, (Matrix *)v));
    if (typeid(*v) == typeid(CMatrix))
      return Value_ptr(eq((Matrix *)u, (CMatrix *)v));
  }
  else if (typeid(*u) == typeid(CMatrix)) {
    if (typeid(*v) == typeid(Matrix))
      return Value_ptr(eq((Matrix *)v, (CMatrix *)u));
    if (typeid(*v) == typeid(CMatrix))
      return Value_ptr(eq((CMatrix *)u, (CMatrix *)v));
  }	
  else if (typeid(*u) == typeid(String)) {
    if (typeid(*v) == typeid(String))
      return Value_ptr(eq((String *)u, (String *)v));
  }
  mpl_error("type error");
  return 0;
}

Real *ne(Real *u, Real *v) {
  return new Real(u->value() != v->value());
}

Real *ne(Real *u, Complex *v) {
  return new Real(u->value() != v->value());
}

Real *ne(Complex *u, Complex *v) {
  return new Real(u->value() != v->value());
}

Real *ne(Vector *u, Vector *v) {
  if (u->index().size() != v->index().size())
    return new Real(1.0);
  int n = u->index().size();
  int i = u->index().start();
  int j = v->index().start();
  while (n--) {
    if ((*u->data())[i] != (*v->data())[j])
      return new Real(1.0);
    i += u->index().stride();
    j += v->index().stride();
  }
  return new Real(0.0);
}

Real *ne(Vector *u, CVector *v) {
  if (u->index().size() != v->index().size())
    return new Real(1.0);
  int n = u->index().size();
  int i = u->index().start();
  int j = v->index().start();
  while (n--) {
    if ((*u->data())[i] != (*v->data())[j])
      return new Real(1.0);
    i += u->index().stride();
    j += v->index().stride();
  }
  return new Real(0.0);
}

Real *ne(CVector *u, CVector *v) {
  if (u->index().size() != v->index().size())
    return new Real(1.0);
  int n = u->index().size();
  int i = u->index().start();
  int j = v->index().start();
  while (n--) {
    if ((*u->data())[i] != (*v->data())[j])
      return new Real(1.0);
    i += u->index().stride();
    j += v->index().stride();
  }
  return new Real(0.0);
}

Real *ne(Matrix *u, Matrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();

  if (ui.size()[0] != vi.size()[0] || ui.size()[1] != vi.size()[1])
    return new Real(1.0);

  int cols = ui.size()[0];
  int rows = vi.size()[1];

  int ii = ui.start();
  int jj = vi.start();
  
  int m = cols;
  while (m--) {
    int n = rows;

    int i = ii;
    int j = jj;
    while (n--) {
      if ((*u->data())[i] != (*v->data())[j])
	return new Real(1.0);
      i += u->index().stride()[1];
      j += v->index().stride()[1];
    }
    ii += u->index().stride()[0];
    jj += v->index().stride()[0];
  }
  return new Real(0.0);
}

Real *ne(Matrix *u, CMatrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();

  if (ui.size()[0] != vi.size()[0] || ui.size()[1] != vi.size()[1])
    return new Real(0.0);

  int cols = ui.size()[0];
  int rows = vi.size()[1];

  int ii = ui.start();
  int jj = vi.start();
  
  int m = cols;
  while (m--) {
    int n = rows;

    int i = ii;
    int j = jj;
    while (n--) {
      if ((*u->data())[i] != (*v->data())[j])
	return new Real(1.0);
      i += u->index().stride()[1];
      j += v->index().stride()[1];
    }
    ii += u->index().stride()[0];
    jj += v->index().stride()[0];
  }
  return new Real(0.0);
}
  
Real *ne(CMatrix *u, CMatrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();

  if (ui.size()[0] != vi.size()[0] || ui.size()[1] != vi.size()[1])
    return new Real(0.0);

  int cols = ui.size()[0];
  int rows = vi.size()[1];

  int ii = ui.start();
  int jj = vi.start();
  
  int m = cols;
  while (m--) {
    int n = rows;

    int i = ii;
    int j = jj;
    while (n--) {
      if ((*u->data())[i] != (*v->data())[j])
	return new Real(1.0);
      i += u->index().stride()[1];
      j += v->index().stride()[1];
    }
    ii += u->index().stride()[0];
    jj += v->index().stride()[0];
  }
  return new Real(0.0);
}
  
Real *ne(String *u, String *v) { 
  int i, j, n;

  if (u->index().size() != v->index().size())
    return new Real(1.0);
  for (i = u->index().start(), j = v->index().start(), n = u->index().size();
       n > 0;
       n--, i += u->index().stride(), j += v->index().stride()) {
    if ((*u->data())[i] != (*v->data())[j])
      return new Real(1.0);
  }
  return new Real(0.0);
}

Value_ptr ne(Value *u, Value *v) {
  if (typeid(*u) == typeid(List) || typeid(*v) == typeid(List))
    return dolist(u, v, ne);
  if (typeid(*u) == typeid(Real)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(ne((Real *)u, (Real *)v));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(ne((Real *)u, (Complex *)v));
  }
  else if (typeid(*u) == typeid(Complex)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(ne((Real *)v, (Complex *)u));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(ne((Complex *)u, (Complex *)v));
  }
  else if (typeid(*u) == typeid(Vector)) {
    if (typeid(*v) == typeid(Vector))
      return Value_ptr(ne((Vector *)u, (Vector *)v));
    if (typeid(*v) == typeid(CVector))
      return Value_ptr(ne((Vector *)u, (CVector *)v));
  }
  else if (typeid(*u) == typeid(CVector)) {
    if (typeid(*v) == typeid(Vector))
      return Value_ptr(ne((Vector *)v, (CVector *)u));
    if (typeid(*v) == typeid(CVector))
      return Value_ptr(ne((CVector *)u, (CVector *)v));
  }
  else if (typeid(*u) == typeid(Matrix)) {
    if (typeid(*v) == typeid(Matrix))
      return Value_ptr(ne((Matrix *)u, (Matrix *)v));
    if (typeid(*v) == typeid(CMatrix))
      return Value_ptr(ne((Matrix *)u, (CMatrix *)v));
  }
  else if (typeid(*u) == typeid(CMatrix)) {
    if (typeid(*v) == typeid(Matrix))
      return Value_ptr(ne((Matrix *)v, (CMatrix *)u));
    if (typeid(*v) == typeid(CMatrix))
      return Value_ptr(ne((CMatrix *)v, (CMatrix *)u));
  }
  else if (typeid(*u) == typeid(String)) {
    if (typeid(*v) == typeid(String))
      return Value_ptr(ne((String *)u, (String *)v));
  }
  mpl_error("type error");
  return 0;
}

Real *lt(Real *u, Real *v) {
  return new Real(u->value() < v->value());
}

Real *lt(String *u, String *v) {
  int n = std::min(u->index().size(), v->index().size());
  int i, j;
  
  for (i = u->index().start(), j = v->index().start(); n > 0;
       n--, i += u->index().stride(), j += v->index().stride()) {
    if ((*u->data())[i] < ((*v->data())[j]))
      return new Real(1.0);
    if ((*u->data())[i] > ((*v->data())[j]))
      return new Real(0.0);
  }
  if (u->index().size() < v->index().size())
    return new Real(1.0);
  return new Real(0.0);
}

Value_ptr lt(Value *u, Value *v) {
  if (typeid(*u) == typeid(List) || typeid(*v) == typeid(List))
    return dolist(u, v, lt);
  if (typeid(*u) == typeid(Real) && typeid(*v) == typeid(Real))
    return Value_ptr(lt((Real *)u, (Real *)v));
  if (typeid(*u) == typeid(String) && typeid(*v) == typeid(String))
    return Value_ptr(lt((String *)u, (String *)v));
  mpl_error("type error");
  return 0;
}

Real *le(Real *u, Real *v) {
  return new Real(u->value() <= v->value());
}

Real *le(String *u, String *v) {
  int n = std::min(u->index().size(), v->index().size());
  int i, j;
  
  for (i = u->index().start(), j = v->index().start(); n > 0;
       n--, i += u->index().stride(), j += v->index().stride()) {
    if ((*u->data())[i] < ((*v->data())[j]))
      return new Real(1.0);
    if ((*u->data())[i] > ((*v->data())[j]))
      return new Real(0.0);
  }
  if (u->index().size() <= v->index().size())
    return new Real(1.0);
  return new Real(0.0);
}

Value_ptr le(Value *u, Value *v) {
  if (typeid(*u) == typeid(List) || typeid(*v) == typeid(List))
    return dolist(u, v, le);
  if (typeid(*u) == typeid(Real) && typeid(*v) == typeid(Real))
    return Value_ptr(le((Real *)u, (Real *)v));
  if (typeid(*u) == typeid(String) && typeid(*v) == typeid(String))
    return Value_ptr(le((String *)u, (String *)v));
  mpl_error("type error");
  return 0;
}

Real *gt(Real *u, Real *v) {
  return new Real(u->value() > v->value());
}

Real *gt(String *u, String *v) {
  int n = std::min(u->index().size(), v->index().size());
  int i, j;
  
  for (i = u->index().start(), j = v->index().start(); n > 0;
       n--, i += u->index().stride(), j += v->index().stride()) {
    if ((*u->data())[i] > ((*v->data())[j]))
      return new Real(1.0);
    if ((*u->data())[i] < ((*v->data())[j]))
      return new Real(0.0);
  }
  if (u->index().size() > v->index().size())
    return new Real(1.0);
  return new Real(0.0);
}

Value_ptr gt(Value *u, Value *v) {
  if (typeid(*u) == typeid(List) || typeid(*v) == typeid(List))
    return dolist(u, v, gt);      
  if (typeid(*u) == typeid(Real) && typeid(*v) == typeid(Real))
    return Value_ptr(gt((Real *)u, (Real *)v));
  if (typeid(*u) == typeid(String) && typeid(*v) == typeid(String))
    return Value_ptr(gt((String *)u, (String *)v));
  mpl_error("type error");
  return 0;
}

Real *ge(Real *u, Real *v) {
  return new Real(u->value() >= v->value());
}

Real *ge(String *u, String *v) {
  int n = std::min(u->index().size(), v->index().size());
  int i, j;
  
  for (i = u->index().start(), j = v->index().start(); n > 0;
       n--, i += u->index().stride(), j += v->index().stride()) {
    if ((*u->data())[i] > ((*v->data())[j]))
      return new Real(1.0);
    if ((*u->data())[i] < ((*v->data())[j]))
      return new Real(0.0);
  }
  if (u->index().size() >= v->index().size())
    return new Real(1.0);
  return new Real(0.0);
}

Value_ptr ge(Value *u, Value *v) {
  if (typeid(*u) == typeid(List) || typeid(*v) == typeid(List))
    return dolist(u, v, ge);
  if (typeid(*u) == typeid(Real) && typeid(*v) == typeid(Real))
    return Value_ptr(ge((Real *)u, (Real *)v));
  if (typeid(*u) == typeid(String) && typeid(*v) == typeid(String))
    return Value_ptr(ge((String *)u, (String *)v));
  mpl_error("type error");
  return 0;
}

Real *add(Real *u, Real *v) {
  return new Real(u->value() + v->value());
}

Complex *add(Real *u, Complex *v) {
  return new Complex(u->value() + v->value());
}

Complex *add(Complex *u, Complex *v) {
  return new Complex(u->value() + v->value());
}

Vector *add(Vector *u, Vector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("sizes unequal in vector addition");
  std::valarray<double> *w = new std::valarray<double>((*u->data())[u->index()]);
  *w += (*v->data())[v->index()];
  return new Vector(double_ptr(w), std::slice(0, w->size(), 1));
}

CVector *add(Vector *u, CVector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("sizes unequal in vector addition");
  std::valarray<std::complex<double>> *w = new std::valarray<std::complex<double>>((*v->data())[v->index()]);
  // *w += (*u->data())[u->index()];
  int j = u->index().start();
  for (int i = 0; i < (int)w->size(); i++) {
    (*w)[i] += (*u->data())[j];
    j += u->index().stride();
  }
  return new CVector(complex_ptr(w), std::slice(0, w->size(), 1));
}

CVector *add(CVector *u, CVector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("sizes unequal in vector addition");
  std::valarray<std::complex<double>> *w = new std::valarray<std::complex<double>>((*u->data())[u->index()]);
  *w += (*v->data())[v->index()];
  return new CVector(complex_ptr(w), std::slice(0, w->size(), 1));
}

Matrix *add(Matrix *u, Matrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();

  if (ui.size()[0] != vi.size()[0] || ui.size()[1] != vi.size()[1]) 
    mpl_error("sizes unequal in matrix addition");
  
  int cols = ui.size()[0]; 
  int rows = ui.size()[1];
  
  // this forces w to column-major, whatever u is.
  std::valarray<double> *w = new std::valarray<double>((*u->data())[u->index()]);
  
  (*w) += (*v->data())[v->index()];
  
  std::size_t lengths[] = {(size_t)cols, (size_t)rows};
  std::size_t strides[] = {(size_t)rows, 1};
  return new Matrix(double_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths,2),
					       std::valarray<std::size_t>(strides, 2)));
}

CMatrix *add(Matrix *u, CMatrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();

  if (ui.size()[0] != vi.size()[0] || ui.size()[1] != vi.size()[1])
    mpl_error("sizes unequal in matrix addition");

  int cols = ui.size()[0];
  int rows = ui.size()[1];

  auto w = new std::valarray<std::complex<double>>((*v->data())[v->index()]);
  int i = 0;
  int j = ui.start();
  int m = cols;
  while (m--) {
    int n = rows;
    int k = j;
    while (n--) {
      (*w)[i++] += (*u->data())[k];
      k += ui.stride()[1];
    }
    j += ui.stride()[0];
  }
  std::size_t lengths[] = {(size_t)cols, (size_t)rows};
  std::size_t strides[] = {(size_t)rows, 1};
  return new CMatrix(complex_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths,2),
						std::valarray<std::size_t>(strides, 2)));
}

CMatrix *add(CMatrix *u, CMatrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();

  if (ui.size()[0] != vi.size()[0] || ui.size()[1] != vi.size()[1]) 
    mpl_error("sizes unequal in matrix addition");
  
  int cols = ui.size()[0]; 
  int rows = ui.size()[1];
  
  // this forces w to column-major, whatever u is.
  auto *w = new std::valarray<std::complex<double>>((*u->data())[u->index()]);
  
  (*w) += (*v->data())[v->index()];
  
  std::size_t lengths[] = {(size_t)cols, (size_t)rows};
  std::size_t strides[] = {(size_t)rows, 1};
  return new CMatrix(complex_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths,2),
					       std::valarray<std::size_t>(strides, 2)));
}

String *add(String *u, String *v) {
  int size = u->index().size() + v->index().size();
  std::valarray<char> x(size);
  x[std::slice(0, u->index().size(), 1)] = (*u->data())[u->index()];
  x[std::slice(u->index().size(), v->index().size(), 1)] = (*v->data())[v->index()];
  return new String(x, std::slice(0, size, 1));
}

Value_ptr add(Value *u, Value *v) {
  if (typeid(*u) == typeid(List) || typeid(*v) == typeid(List))
    return dolist(u, v, add);
  if (typeid(*u) == typeid(Real)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(add((Real *)u, (Real *)v));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(add((Real *)u, (Complex *)v));
  }
  else if (typeid(*u) == typeid(Complex)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(add((Real *)v, (Complex *)u));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(add((Complex *)u, (Complex *)v));
  }
  else if (typeid(*u) == typeid(Vector)) {
    if (typeid(*v) == typeid(Vector))
      return Value_ptr(add((Vector *)u, (Vector *)v));
    if (typeid(*v) == typeid(CVector))
      return Value_ptr(add((Vector *)u, (CVector *)v));
  }
  else if (typeid(*u) == typeid(CVector)) {
    if (typeid(*v) == typeid(Vector))
      return Value_ptr(add((Vector *)v, (CVector *)u));
    if (typeid(*v) == typeid(CVector))
      return Value_ptr(add((CVector *)u, (CVector *)v));
  }
  else if (typeid(*u) == typeid(Matrix)) {
    if (typeid(*v) == typeid(Matrix))
      return Value_ptr(add((Matrix *)u, (Matrix *)v));
    if (typeid(*v) == typeid(CMatrix))
      return Value_ptr(add((Matrix *)u, (CMatrix *)v));
  }
  else if (typeid(*u) == typeid(CMatrix)) {
    if (typeid(*v) == typeid(Matrix))
      return Value_ptr(add((Matrix *)v, (CMatrix *)u));
    if (typeid(*v) == typeid(CMatrix))
      return Value_ptr(add((CMatrix *)v, (CMatrix *)u));
  }
  else if (typeid(*u) == typeid(String)) {
    if (typeid(*v) == typeid(String))
      return Value_ptr(add((String *)u, (String *)v));
  }
  mpl_error("type error in addition");
  return 0;
}

Real *subtract(Real *u, Real *v) {
  return new Real(u->value() - v->value());
}

Complex *subtract(Real *u, Complex *v) {
  return new Complex(u->value() - v->value());
}

Complex *subtract(Complex *u, Real *v) {
  return new Complex(u->value() - v->value());
}

Complex *subtract(Complex *u, Complex *v) {
  return new Complex(u->value() - v->value());
}

Vector *subtract(Vector *u, Vector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("sizes unequal in vector subtraction");
  std::valarray<double> *w = new std::valarray<double>((*u->data())[u->index()]);
  *w -= (*v->data())[v->index()];
  return new Vector(double_ptr(w), std::slice(0, w->size(), 1));
}

CVector *subtract(Vector *u, CVector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("sizes unequal in vector subtraction");
  std::valarray<std::complex<double>> *w = new std::valarray<std::complex<double>>((*v->data())[v->index()]);
  int j = u->index().start();
  for (int i = 0; i < (int)w->size(); i++) {
    (*w)[i] = (*u->data())[j] - (*w)[i];
    j += u->index().stride();
  }
  return new CVector(complex_ptr(w), std::slice(0, w->size(), 1));
}

CVector *subtract(CVector *u, Vector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("sizes unequal in vector subtraction");
  std::valarray<std::complex<double>> *w = new std::valarray<std::complex<double>>((*u->data())[u->index()]);
  int j = v->index().start();
  for (int i = 0; i < (int)w->size(); i++) {
    (*w)[i] -= (*v->data())[j];
    j += v->index().stride();
  }
  return new CVector(complex_ptr(w), std::slice(0, w->size(), 1));
}

CVector *subtract(CVector *u, CVector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("sizes unequal in vector subtraction");
  std::valarray<std::complex<double>> *w = new std::valarray<std::complex<double>>((*u->data())[u->index()]);
  *w -= (*v->data())[v->index()];
  return new CVector(complex_ptr(w), std::slice(0, w->size(), 1));
}

Matrix *subtract(Matrix *u, Matrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();

  if (ui.size()[0] != vi.size()[0] || ui.size()[1] != vi.size()[1]) 
    mpl_error("sizes unequal in matrix subtraction");

  int cols = ui.size()[0]; 
  int rows = ui.size()[1];

  std::valarray<double> *w = new std::valarray<double>((*u->data())[u->index()]);

  (*w) -= (*v->data())[v->index()];

  std::size_t lengths[] = {(size_t)cols, (size_t)rows};
  std::size_t strides[] = {(size_t)rows, 1};
  return new Matrix(double_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths,2),
				   std::valarray<std::size_t>(strides, 2)));
}
  
CMatrix *subtract(Matrix *u, CMatrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();

  if (ui.size()[0] != vi.size()[0] || ui.size()[1] != vi.size()[1])
    mpl_error("sizes unequal in matrix subtraction");

  int cols = ui.size()[0];
  int rows = ui.size()[1];

  auto w = new std::valarray<std::complex<double>>((*v->data())[v->index()]);
  int i = 0;
  int j = ui.start();
  int m = cols;
  while (m--) {
    int n = rows;
    int k = j;
    while (n--) {
      (*w)[i] = (*u->data())[k] - (*w)[i];
      i++;
      k += ui.stride()[1];
    }
    j += ui.stride()[0];
  }
  std::size_t lengths[] = {(size_t)cols, (size_t)rows};
  std::size_t strides[] = {(size_t)rows, 1};
  return new CMatrix(complex_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths,2),
						std::valarray<std::size_t>(strides, 2)));
}

CMatrix *subtract(CMatrix *u, Matrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();

  if (ui.size()[0] != vi.size()[0] || ui.size()[1] != vi.size()[1])
    mpl_error("sizes unequal in matrix subtraction");

  int cols = ui.size()[0];
  int rows = ui.size()[1];

  auto w = new std::valarray<std::complex<double>>((*u->data())[u->index()]);
  int i = 0;
  int j = vi.start();
  int m = cols;
  while (m--) {
    int n = rows;
    int k = j;
    while (n--) {
      (*w)[i] -= (*v->data())[k];
      i++;
      k += vi.stride()[1];
    }
    j += vi.stride()[0];
  }
  std::size_t lengths[] = {(size_t)cols, (size_t)rows};
  std::size_t strides[] = {(size_t)rows, 1};
  return new CMatrix(complex_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths,2),
						std::valarray<std::size_t>(strides, 2)));
}

CMatrix *subtract(CMatrix *u, CMatrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();

  if (ui.size()[0] != vi.size()[0] || ui.size()[1] != vi.size()[1]) 
    mpl_error("sizes unequal in matrix subtraction");

  int cols = ui.size()[0]; 
  int rows = ui.size()[1];

  auto *w = new std::valarray<std::complex<double>>((*u->data())[u->index()]);

  (*w) -= (*v->data())[v->index()];

  std::size_t lengths[] = {(size_t)cols, (size_t)rows};
  std::size_t strides[] = {(size_t)rows, 1};
  return new CMatrix(complex_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths,2),
						std::valarray<std::size_t>(strides, 2)));
}

String *subtract(String *u, String *v) {
  int size = u->index().size() + v->index().size();
  std::valarray<char> x(size);
  x[std::slice(0, u->index().size(), 1)] = (*u->data())[u->index()];
  x[std::slice(u->index().size(), v->index().size(), 1)] =
    (*v->data()) [std::slice(v->index().start() + (v->index().size() - 1)*v->index().stride(),
			     v->index().size(),
			     -v->index().stride())];
  //                         ^
  //                         |
  //  +----------------------+
  //  |
  //  This is risky. Using signed arithmetic on unsigned types!
  //  This works on my computer, compiler, library. It may not
  //  work on others! (Why are slice components type size_t! Ugh!)
  
  return new String(x, std::slice(0, size, 1));
}

Value_ptr subtract(Value *u, Value *v) {
  if (typeid(*u) == typeid(List) || typeid(*v) == typeid(List))
    return dolist(u, v, subtract);
  if (typeid(*u) == typeid(Real)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(subtract((Real *)u, (Real *)v));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(subtract((Real *)u, (Complex *)v));
  }
  else if (typeid(*u) == typeid(Complex)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(subtract((Complex *)u, (Real *)v));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(subtract((Complex *)u, (Complex *)v));
  }
  else if (typeid(*u) == typeid(Vector)) {
    if (typeid(*v) == typeid(Vector))
      return Value_ptr(subtract((Vector *)u, (Vector *)v));
    if (typeid(*v) == typeid(CVector))
      return Value_ptr(subtract((Vector *)u, (CVector *)v));
  }
  else if (typeid(*u) == typeid(CVector)) {
    if (typeid(*v) == typeid(Vector))
      return Value_ptr(subtract((CVector *)u, (Vector *)v));
    if (typeid(*v) == typeid(CVector))
      return Value_ptr(subtract((CVector *)u, (CVector *)v));
  }
  else if (typeid(*u) == typeid(Matrix)) {
    if (typeid(*v) == typeid(Matrix))
      return Value_ptr(subtract((Matrix *)u, (Matrix *)v));
    if (typeid(*v) == typeid(CMatrix))
      return Value_ptr(subtract((Matrix *)u, (CMatrix *)v));
  }
  else if (typeid(*u) == typeid(CMatrix)) {
    if (typeid(*v) == typeid(Matrix))
      return Value_ptr(subtract((CMatrix *)u, (Matrix *)v));
    if (typeid(*v) == typeid(CMatrix))
      return Value_ptr(subtract((CMatrix *)u, (CMatrix *)v));
  }
  else if (typeid(*u) == typeid(String)) {
    if (typeid(*v) == typeid(String))
      return Value_ptr(subtract((String *)u, (String *)v));
  }
  mpl_error("type error in subtraction");
  return 0;
}

Real *multiply(Real *u, Real *v) {
  return new Real(u->value() * v->value());
}

Complex *multiply(Real *u, Complex *v) {
  return new Complex(u->value() * v->value());
}

Complex *multiply(Complex *u, Complex *v) {
  return new Complex(u->value() * v->value());
}

Vector *multiply(Real *u, Vector *v) {
  auto *w = new std::valarray<double>((*v->data())[v->index()]);
  *w *= u->value();
  return new Vector(double_ptr(w), std::slice(0, w->size(), 1));
}

CVector *multiply(Complex *u, Vector *v) {
  auto *w = new std::valarray<std::complex<double>>(v->index().size());
  for (int i = 0, j = v->index().start(); i < (int)v->index().size(); i++, j += v->index().stride())
    (*w)[i] = (*v->data())[j];
  *w *= u->value();
  return new CVector(complex_ptr(w), std::slice(0, w->size(), 1));
}

CVector *multiply(Complex *u, CVector *v) {
  auto *w = new std::valarray<std::complex<double>>((*v->data())[v->index()]);
  *w *= u->value();
  return new CVector(complex_ptr(w), std::slice(0, w->size(), 1));
}

Real *multiply(Vector *u, Vector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("sizes unequal in vector multiplication");
  int n = u->index().size();
  double p = 0.0;
  int i = u->index().start();
  int j = v->index().start();
  while (n--) {
    p += (*u->data())[i]*(*v->data())[j];
    i += u->index().stride();
    j += v->index().stride();
  }
  return new Real(p);
}

// for complex vectors, x*y == conj(y*x)
// we can calculate u*v as conj(v*u)
// That way we can avoid taking the conjucates of
// the members of v since they are all real

Complex *multiply(Vector *u, CVector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("sizes unequal in vector multiplication");
  int n = u->index().size();
  std::complex<double> p = 0;
  int i = u->index().start();
  int j = v->index().start();
  while (n--) {
    p += (*u->data())[i]*(*v->data())[j];
    i += u->index().stride();
    j += u->index().stride();
  }
  return new Complex(conj(p));
}

// since v are all real no need for conjugation

Complex *multiply(CVector *u, Vector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("sizes unequal in vector multiplication");
  int n = u->index().size();
  std::complex<double> p = 0;
  int i = u->index().start();
  int j = v->index().start();
  while (n--) {
    p += (*u->data())[i]*(*v->data())[j];
    i += u->index().stride();
    j += u->index().stride();
  }
  return new Complex(p);
}

CVector *multiply(Real *u, CVector *v) {
  std::valarray<std::complex<double>> *w = new std::valarray<std::complex<double>>((*v->data())[v->index()]);
  *w *= u->value();
  return new CVector(complex_ptr(w), std::slice(0, w->size(), 1));
}

Complex *multiply(CVector *u, CVector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("sizes unequal in vector multiplication");
  int n = u->index().size();
  std::complex<double> p = 0.0;
  int i = u->index().start();
  int j = v->index().start();
  while (n--) {
    p += (*u->data())[i]*conj((*v->data())[j]);
    i += u->index().stride();
    j += v->index().stride();
  }
  return new Complex(p);
}

Matrix *multiply(Real *u, Matrix *v) {
  auto *w = new std::valarray<double>((*v->data())[v->index()]);
  *w *= u->value();
  std::size_t lengths[] = {v->index().size()[0], v->index().size()[1]};
  std::size_t strides[] = {v->index().size()[1], 1};
  return new Matrix(double_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths, 2),
					       std::valarray<std::size_t>(strides, 2)));
}

CMatrix *multiply(Real *u, CMatrix *v) {
  auto *w = new std::valarray<std::complex<double>>((*v->data())[v->index()]);
  *w *= u->value();
  std::size_t lengths[] = {v->index().size()[0], v->index().size()[1]};
  std::size_t strides[] = {v->index().size()[1], 1};
  return new CMatrix(complex_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths, 2),
						 std::valarray<std::size_t>(strides, 2)));
}

CMatrix *multiply(Complex *u, Matrix *v) {
  int rows = v->index().size()[1];
  int cols = v->index().size()[0];
  auto *w = new std::valarray<std::complex<double>>(rows*cols);
  int i = 0;
  int j = v->index().start();
  int m = cols;
  while (m--) {
    int n = rows;
    int k = j;
    while (n--) {
      (*w)[i++] = u->value()*(*v->data())[k];
      k += v->index().stride()[1];
    }
    j += v->index().stride()[0];
  }
  std::size_t lengths[] = {(size_t)cols, (size_t)rows};
  std::size_t strides[] = {(size_t)rows, 1};
  return new CMatrix(complex_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths, 2),
					       std::valarray<std::size_t>(strides, 2)));
}

CMatrix *multiply(Complex *u, CMatrix *v) {
  auto *w = new std::valarray<std::complex<double>>((*v->data())[v->index()]);
  *w *= u->value();
  std::size_t lengths[] = {v->index().size()[0], v->index().size()[1]};
  std::size_t strides[] = {v->index().size()[1], 1};
  return new CMatrix(complex_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths, 2),
						 std::valarray<std::size_t>(strides, 2)));
}

Vector *multiply(Matrix *u, Vector *v) {
  std::gslice ui = u->index();
  std::slice vi = v->index();
  if (ui.size()[0] != vi.size())
    mpl_error("size mismatch in matrix/vector multiplication");
  std::valarray<double> *ux = u->data().get();
  std::valarray<double> *vx = v->data().get();
  auto *w = new std::valarray<double>(vi.size());
  int r = ui.start();
  for (int i = 0; i < v->size(); i++) { // i is the index into w
    double sum = 0.0;
    int j = r;
    int k = vi.start();
    int n = vi.size();
    while (n--) {
      sum += (*ux)[j] * (*vx)[k];
      j += ui.stride()[0];
      k += vi.stride();
    }
    (*w)[i] = sum;
    r += ui.stride()[1];
  }
  return new Vector(double_ptr(w), std::slice(0, vi.size(), 1));
}

CVector *multiply(Matrix *u, CVector *v) {
  std::gslice ui = u->index();
  std::slice vi = v->index();
  if (ui.size()[0] != vi.size())
    mpl_error("size mismatch in matrix/vector multiplication");
  std::valarray<double> *ux = u->data().get();
  std::valarray<std::complex<double>> *vx = v->data().get();
  auto *w = new std::valarray<std::complex<double>>(vi.size());
  int r = ui.start();
  for (int i = 0; i < (int)vi.size(); i++) { // i is the index into w
    std::complex<double> sum = 0.0;
    int j = r;
    int k = vi.start();
    int n = vi.size();
    while (n--) {
      sum += (*ux)[j] * (*vx)[k];
      j += ui.stride()[0];
      k += vi.stride();
    }
    (*w)[i] = sum;
    r += ui.stride()[1];
  }
  return new CVector(complex_ptr(w), std::slice(0, vi.size(), 1));
}

CVector *multiply(CMatrix *u, Vector *v) {
  std::gslice ui = u->index();
  std::slice vi = v->index();
  if (ui.size()[0] != vi.size())
    mpl_error("size mismatch in matrix/vector multiplication");
  std::valarray<std::complex<double>> *ux = u->data().get();
  std::valarray<double> *vx = v->data().get();
  auto *w = new std::valarray<std::complex<double>>(vi.size());
  int r = ui.start();
  for (int i = 0; i < v->size(); i++) { // i is the index into w
    std::complex<double> sum = 0.0;
    int j = r;
    int k = vi.start();
    int n = vi.size();
    while (n--) {
      sum += (*ux)[j] * (*vx)[k];
      j += ui.stride()[0];
      k += vi.stride();
    }
    (*w)[i] = sum;
    r += ui.stride()[1];
  }
  return new CVector(complex_ptr(w), std::slice(0, vi.size(), 1));
}

CVector *multiply(CMatrix *u, CVector *v) {
  std::gslice ui = u->index();
  std::slice vi = v->index();
  if (ui.size()[0] != vi.size())
    mpl_error("size mismatch in matrix/vector multiplication");
  std::valarray<std::complex<double>> *ux = u->data().get();
  std::valarray<std::complex<double>> *vx = v->data().get();
  auto *w = new std::valarray<std::complex<double>>(vi.size());
  int r = ui.start();
  for (int i = 0; i < (int)vi.size(); i++) { // i is the index into w
    std::complex<double> sum = 0.0;
    int j = r;
    int k = vi.start();
    int n = vi.size();
    while (n--) {
      sum += (*ux)[j] * (*vx)[k];
      j += ui.stride()[0];
      k += vi.stride();
    }
    (*w)[i] = sum;
    r += ui.stride()[1];
  }
  return new CVector(complex_ptr(w), std::slice(0, vi.size(), 1));
}

Matrix *multiply(Matrix *u, Matrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();
  if (ui.size()[0] != vi.size()[1])
    mpl_error("size mismatch in matrix multiplication");
  std::valarray<double> *ux = u->data().get();
  std::valarray<double> *vx = v->data().get();
  auto *w = new std::valarray<double>(ui.size()[1]*vi.size()[0]);
  int i = 0;
  int sk = vi.start();
  int m = ui.size()[0];
  while (m--) {  // for each column in w (column in v)
    int sj = ui.start();
    int n = vi.size()[1];
    while (n--) { // for each row in w (row in u)
      double sum = 0.0;
      int j = sj;
      int k = sk;
      int p = ui.size()[0];
      while (p--) {
	sum += (*ux)[j] * (*vx)[k];
	j += ui.stride()[0];
	k += vi.stride()[1];
      }
      (*w)[i++] = sum;
      sj += ui.stride()[1];
    }
    sk += vi.stride()[0];
  }
  std::size_t lengths[] = { vi.size()[1], ui.size()[0] };
  std::size_t strides[] = { ui.size()[0], 1 };
  return new Matrix(double_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths, 2),
					       std::valarray<std::size_t>(strides, 2)));
}

CMatrix *multiply(Matrix *u, CMatrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();
  if (ui.size()[0] != vi.size()[1])
    mpl_error("size mismatch in matrix multiplication");
  std::valarray<double> *ux = u->data().get();
  std::valarray<std::complex<double>> *vx = v->data().get();
  auto *w = new std::valarray<std::complex<double>>(ui.size()[1]*vi.size()[0]);
  int i = 0;
  int sk = vi.start();
  int m = ui.size()[0];
  while (m--) {  // for each column in w (column in v)
    int sj = ui.start();
    int n = vi.size()[1];
    while (n--) { // for each row in w (row in u)
      std::complex<double> sum = 0.0;
      int j = sj;
      int k = sk;
      int p = ui.size()[0];
      while (p--) {
	sum += (*ux)[j] * (*vx)[k];
	j += ui.stride()[0];
	k += vi.stride()[1];
      }
      (*w)[i++] = sum;
      sj += ui.stride()[1];
    }
    sk += vi.stride()[0];
  }
  std::size_t lengths[] = { vi.size()[1], ui.size()[0] };
  std::size_t strides[] = { ui.size()[0], 1 };
  return new CMatrix(complex_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths, 2),
					       std::valarray<std::size_t>(strides, 2)));
}

CMatrix *multiply(CMatrix *u, Matrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();
  if (ui.size()[0] != vi.size()[1])
    mpl_error("size mismatch in matrix multiplication");
  std::valarray<std::complex<double>> *ux = u->data().get();
  std::valarray<double> *vx = v->data().get();
  auto *w = new std::valarray<std::complex<double>>(ui.size()[1]*vi.size()[0]);
  int i = 0;
  int sk = vi.start();
  int m = ui.size()[0];
  while (m--) {  // for each column in w (column in v)
    int sj = ui.start();
    int n = vi.size()[1];
    while (n--) { // for each row in w (row in u)
      std::complex<double> sum = 0.0;
      int j = sj;
      int k = sk;
      int p = ui.size()[0];
      while (p--) {
	sum += (*ux)[j] * (*vx)[k];
	j += ui.stride()[0];
	k += vi.stride()[1];
      }
      (*w)[i++] = sum;
      sj += ui.stride()[1];
    }
    sk += vi.stride()[0];
  }
  std::size_t lengths[] = { vi.size()[1], ui.size()[0] };
  std::size_t strides[] = { ui.size()[0], 1 };
  return new CMatrix(complex_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths, 2),
						std::valarray<std::size_t>(strides, 2)));
}

CMatrix *multiply(CMatrix *u, CMatrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();
  if (ui.size()[0] != vi.size()[1])
    mpl_error("size mismatch in matrix multiplication");
  std::valarray<std::complex<double>> *ux = u->data().get();
  std::valarray<std::complex<double>> *vx = v->data().get();
  auto *w = new std::valarray<std::complex<double>>(ui.size()[1]*vi.size()[0]);
  int i = 0;
  int sk = vi.start();
  int m = ui.size()[0];
  while (m--) {  // for each column in w (column in v)
    int sj = ui.start();
    int n = vi.size()[1];
    while (n--) { // for each row in w (row in u)
      std::complex<double> sum = 0.0;
      int j = sj;
      int k = sk;
      int p = ui.size()[0];
      while (p--) {
	sum += (*ux)[j] * (*vx)[k];
	j += ui.stride()[0];
	k += vi.stride()[1];
      }
      (*w)[i++] = sum;
      sj += ui.stride()[1];
    }
    sk += vi.stride()[0];
  }
  std::size_t lengths[] = { vi.size()[1], ui.size()[0] };
  std::size_t strides[] = { ui.size()[0], 1 };
  return new CMatrix(complex_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths, 2),
						 std::valarray<std::size_t>(strides, 2)));
}

String *multiply(Real *u, String *v) {
  int n = round(u->value());
  std::slice s = v->index();
  if (n < 0) {
    n = -n;
    s = v->index();
    s = std::slice(s.start() + (s.size() - 1)*s.stride(), s.size(), -s.stride());
  }
  int m = v->index().size();
  int size = n*m;
  std::valarray<char> x(size);
  for (int i = 0; i < n; i++)
    x[std::slice(i*m, m, 1)] = (*v->data())[s];
  return new String(x, std::slice(0, size, 1));
}

Value_ptr multiply(Value *u, Value *v) {
  if (typeid(*u) == typeid(List) || typeid(*v) == typeid(List))
    return dolist(u, v, multiply);
  if (typeid(*u) == typeid(Real)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(multiply((Real *)u, (Real *)v));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(multiply((Real *)u, (Complex *)v));
    if (typeid(*v) == typeid(Vector))
      return Value_ptr(multiply((Real *)u, (Vector *)v));
    if (typeid(*v) == typeid(CVector))
      return Value_ptr(multiply((Real *)u, (CVector *)v));
    if (typeid(*v) == typeid(String))
      return Value_ptr(multiply((Real *)u, (String *)v));
    if (typeid(*v) == typeid(Matrix))
      return Value_ptr(multiply((Real *)u, (Matrix *)v));
    if (typeid(*v) == typeid(CMatrix))
      return Value_ptr(multiply((Real *)u, (CMatrix *)v));
  }
  else if (typeid(*u) == typeid(Complex)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(multiply((Real *)v, (Complex *)u));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(multiply((Complex *)u, (Complex *)v));
    if (typeid(*v) == typeid(Vector))
      return Value_ptr(multiply((Complex *)u, (Vector *)v));
    if (typeid(*v) == typeid(CVector))
      return Value_ptr(multiply((Complex *)u, (CVector *)v));
    if (typeid(*v) == typeid(Matrix))
      return Value_ptr(multiply((Complex *)u, (Matrix *)v));
    if (typeid(*v) == typeid(CMatrix))
      return Value_ptr(multiply((Complex *)u, (CMatrix *)v));
  }
  else if (typeid(*u) == typeid(Vector)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(multiply((Real *)v, (Vector *)u));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(multiply((Complex *)v, (Vector *)u));
    if (typeid(*v) == typeid(Vector))
      return Value_ptr(multiply((Vector *)u, (Vector *)v));
    if (typeid(*v) == typeid(CVector))
      return Value_ptr(multiply((Vector *)u, (CVector *)v));
  }
  else if (typeid(*u) == typeid(CVector)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(multiply((Real *)v, (CVector *)u));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(multiply((Complex *)v, (CVector *)u));
    if (typeid(*v) == typeid(Vector))
      return Value_ptr(multiply((CVector *)u, (Vector *)v));
    if (typeid(*v) == typeid(CVector))
      return Value_ptr(multiply((CVector *)u, (CVector *)v));
  }
  else if (typeid(*u) == typeid(Matrix)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(multiply((Real *)v, (Matrix *)u));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(multiply((Complex *)v, (Matrix *)u));
    if (typeid(*v) == typeid(Vector))
      return Value_ptr(multiply((Matrix *)u, (Vector *)v));
    if (typeid(*v) == typeid(CVector))
      return Value_ptr(multiply((Matrix *)u, (CVector *)v));
    if (typeid(*v) == typeid(Matrix))
      return Value_ptr(multiply((Matrix *)u, (Matrix *)v));
    if (typeid(*v) == typeid(CMatrix))
      return Value_ptr(multiply((Matrix *)u, (CMatrix *)v));
  }
  else if (typeid(*u) == typeid(CMatrix)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(multiply((Real *)v, (CMatrix *)u));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(multiply((Complex *)v, (CMatrix *)u));
    if (typeid(*v) == typeid(Vector))
      return Value_ptr(multiply((CMatrix *)u, (Vector *)v));
    if (typeid(*v) == typeid(CVector))
      return Value_ptr(multiply((CMatrix *)u, (CVector *)v));
    if (typeid(*v) == typeid(Matrix))
      return Value_ptr(multiply((CMatrix *)u, (Matrix *)v));
    if (typeid(*v) == typeid(CMatrix))
      return Value_ptr(multiply((CMatrix *)u, (CMatrix *)v));
  }
  else if (typeid(*u) == typeid(String)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(multiply((Real *)v, (String *)u));
  }
  std::cout << "don't know how to multiply " << typeid(*u).name() << " and " << typeid(*v).name() << std::endl;
  mpl_error("type error");
  return 0;
}

Vector *pointwise_multiply(Vector *u, Vector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("sizes unequal in pointwise multiplication");
  std::valarray<double> *w = new std::valarray<double>((*u->data())[u->index()]);
  *w *= (*v->data())[v->index()];
  return new Vector(double_ptr(w), std::slice(0, w->size(), 1));
}

CVector *pointwise_multiply(Vector *u, CVector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("sizes unequal in pointwise multiplication");
  std::valarray<std::complex<double>> *w = new std::valarray<std::complex<double>>((*v->data())[v->index()]);
  int j = u->index().start();
  for (int i = 0; i < (int)w->size(); i++) {
    (*w)[i] *= (*u->data())[j];
    j += u->index().stride();
  }
  return new CVector(complex_ptr(w), std::slice(0, w->size(), 1));
}

CVector *pointwise_multiply(CVector *u, CVector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("sizes unequal in pointwise multiplication");
  std::valarray<std::complex<double>> *w = new std::valarray<std::complex<double>>((*u->data())[u->index()]);
  *w *= (*v->data())[v->index()];
  return new CVector(complex_ptr(w), std::slice(0, w->size(), 1));
}

Matrix *pointwise_multiply(Matrix *u, Matrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();
  
  if (ui.size()[0] != vi.size()[0] || ui.size()[1] != vi.size()[1])
    mpl_error("sizes unequal inpointwise multiplication");

  int cols = ui.size()[0];
  int rows = ui.size()[1];

  std::valarray<double> *w = new std::valarray<double>((*u->data())[u->index()]);
  (*w) *= (*v->data())[v->index()];
  std::size_t lengths[] = {(size_t)cols, (size_t)rows};
  std::size_t strides[] = {(size_t)rows, 1};
  return new Matrix(double_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths, 2),
					       std::valarray<std::size_t>(strides, 2)));
}

CMatrix *pointwise_multiply(Matrix *u, CMatrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();

  if (ui.size()[0] != vi.size()[0] || ui.size()[1] != vi.size()[1])
    mpl_error("sizes unequal in pointwise multiply");
  int cols = ui.size()[0];
  int rows = ui.size()[1];
  std::valarray<std::complex<double>> *w = new std::valarray<std::complex<double>>((*v->data())[v->index()]);
  int i = 0;
  int j = ui.start();
  int m = cols;
  while (m--) {
    int n = rows;
    int k = j;
    while (n--) {
      (*w)[i++] *= (*u->data())[k];
      k += ui.stride()[1];
    }
    j += ui.stride()[0];
  }
  std::size_t lengths[] = {(size_t)cols, (size_t)rows};
  std::size_t strides[] = {(size_t)rows, 1};
  return new CMatrix(complex_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths, 2),
						 std::valarray<std::size_t>(strides, 2)));
}
  

CMatrix *pointwise_multiply(CMatrix *u, CMatrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();
  
  if (ui.size()[0] != vi.size()[0] || ui.size()[1] != vi.size()[1])
    mpl_error("sizes unequal inpointwise multiplication");

  int cols = ui.size()[0];
  int rows = ui.size()[1];

  std::valarray<std::complex<double>> *w = new std::valarray<std::complex<double>>((*u->data())[u->index()]);
  (*w) *= (*v->data())[v->index()];
  std::size_t lengths[] = {(size_t)cols, (size_t)rows};
  std::size_t strides[] = {(size_t)rows, 1};
  return new CMatrix(complex_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths, 2),
					       std::valarray<std::size_t>(strides, 2)));
}

Value_ptr pointwise_multiply(Value *u, Value *v) {
  if (typeid(*u) == typeid(List) || typeid(*v) == typeid(List))
    return dolist(u, v, pointwise_multiply);
  if (typeid(*u) == typeid(Vector)) {
    if (typeid(*v) == typeid(Vector))
      return Value_ptr(pointwise_multiply((Vector *)u, (Vector *)v));
    if (typeid(*v) == typeid(CVector))
      return Value_ptr(pointwise_multiply((Vector *)u, (CVector *)v));
  }
  else if (typeid(*u) == typeid(CVector)) {
    if (typeid(*v) == typeid(Vector))
      return Value_ptr(pointwise_multiply((Vector *)v, (CVector *)u));
    if (typeid(*v) == typeid(CVector))
      return Value_ptr(pointwise_multiply((CVector *)v, (CVector *)u));
  }
  else if (typeid(*u) == typeid(Matrix)) {
    if (typeid(*v) == typeid(Matrix))
      return Value_ptr(pointwise_multiply((Matrix *)u, (Matrix *)v));
    if (typeid(*v) == typeid(CMatrix))
      return Value_ptr(pointwise_multiply((Matrix *)u, (CMatrix *)v));
  }
  else if (typeid(*u) == typeid(CMatrix)) {
    if (typeid(*v) == typeid(Matrix))
      return Value_ptr(pointwise_multiply((Matrix *)v, (CMatrix *)u));
    if (typeid(*v) == typeid(CMatrix))
      return Value_ptr(pointwise_multiply((CMatrix *)v, (CMatrix *)u));
  }
  mpl_error("type error in pointwise multiply");
  return 0;
}

Real *divide(Real *u, Real *v) {
  return new Real(u->value() / v->value());
}

Complex *divide(Real *u, Complex *v) {
  return new Complex(u->value() / v->value());
}

Complex *divide(Complex *u, Real *v) {
  return new Complex(u->value() / v->value());
}

Complex *divide(Complex *u, Complex *v) {
  return new Complex(u->value() / v->value());
}

Vector *divide(Vector *u, Real *v) {
  std::valarray<double> *w = new std::valarray<double>((*u->data())[u->index()]);
  *w /= v->value();
  return new Vector(double_ptr(w), std::slice(0,w->size(),1));
}

CVector *divide(Vector *u, Complex *v) {
  int n = u->index().size();
  auto *w = new std::valarray<std::complex<double>>(n);
  int j = u->index().start();
  for (int i = 0; i < n; i++) {
    (*w)[i] = (*u->data())[j]/v->value();
    j += u->index().stride();
  }
  return new CVector(complex_ptr(w), std::slice(0, n, 1));
}

CVector *divide(CVector *u, Real *v) {
  int n = u->index().size();
  auto *w = new std::valarray<std::complex<double>>(n);
  int j = u->index().start();
  for (int i = 0; i < n; i++) {
    (*w)[i] = (*u->data())[j]/v->value();
    j += u->index().stride();
  }
  return new CVector(complex_ptr(w), std::slice(0, n, 1));
}

CVector *divide(CVector *u, Complex *v) {
  int n = u->index().size();
  auto *w = new std::valarray<std::complex<double>>(n);
  int j = u->index().start();
  for (int i = 0; i < n; i++) {
    (*w)[i] = (*u->data())[j]/v->value();
    j += u->index().stride();
  }
  return new CVector(complex_ptr(w), std::slice(0, n, 1));
}

Matrix *divide(Matrix *u, Real *v) {
  auto *w = new std::valarray<double>((*u->data())[u->index()]);
  *w /= v->value();
  std::size_t lengths[] = {u->index().size()[0], u->index().size()[1]};
  std::size_t strides[] = {u->index().size()[1], 1};
  return new Matrix(double_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths, 2),
					       std::valarray<std::size_t>(strides, 2)));
}

CMatrix *divide(Matrix *u, Complex *v) {
  int rows = u->index().size()[1];
  int cols = u->index().size()[0];
  auto *w = new std::valarray<std::complex<double>>(rows*cols);
  int i = 0;
  int j = u->index().start();
  int m = cols;
  while (m--) {
    int k = j;
    int n = rows;
    while (n--) {
      (*w)[i++] = (*u->data())[k]/v->value();
      k += u->index().stride()[1];
    }
    j += u->index().stride()[0];
  }
  std::size_t lengths[] = {(size_t)cols, (size_t)rows};
  std::size_t strides[] = {(size_t)rows, 1};
  return new CMatrix(complex_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths, 2),
						 std::valarray<std::size_t>(strides, 2)));
}

CMatrix *divide(CMatrix *u, Real *v) {
  auto *w = new std::valarray<std::complex<double>>((*u->data())[u->index()]);
  *w /= v->value();
  std::size_t lengths[] = {u->index().size()[0], u->index().size()[1]};
  std::size_t strides[] = {u->index().size()[1], 1};
  return new CMatrix(complex_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths, 2),
					       std::valarray<std::size_t>(strides, 2)));
}

CMatrix *divide(CMatrix *u, Complex *v) {
  auto *w = new std::valarray<std::complex<double>>((*u->data())[u->index()]);
  *w /= v->value();
  std::size_t lengths[] = {u->index().size()[0], u->index().size()[1]};
  std::size_t strides[] = {u->index().size()[1], 1};
  return new CMatrix(complex_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths, 2),
					       std::valarray<std::size_t>(strides, 2)));
}

Value_ptr divide(Value *u, Value *v) {
  if (typeid(*u) == typeid(List) || typeid(*v) == typeid(List))
    return dolist(u, v, divide);
  if (typeid(*u) == typeid(Real)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(divide((Real *)u, (Real *)v));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(divide((Real *)u, (Complex *)v));
  }
  else if (typeid(*u) == typeid(Complex)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(divide((Complex *)u, (Real *)v));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(divide((Complex *)u, (Complex *)v));
  }
  else if (typeid(*u) == typeid(Vector)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(divide((Vector *)u, (Real *)v));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(divide((Vector *)u, (Complex *)v));
  }
  else if (typeid(*u) == typeid(CVector)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(divide((CVector *)u, (Real *)v));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(divide((CVector *)u, (Complex *)v));
  }
  else if (typeid(*u) == typeid(Matrix)){
    if (typeid(*v) == typeid(Real))
      return Value_ptr(divide((Matrix *)u, (Real *)v));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(divide((Matrix *)u, (Complex *)v));
  }
  else if (typeid(*u) == typeid(CMatrix)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(divide((CMatrix *)u, (Real *)v));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(divide((CMatrix *)u, (Complex *)v));
  }
  std::cout << "don't know how to divide " << typeid(*u).name() << " and " << typeid(*v).name() << std::endl;
  mpl_error("type_error");
  return 0;
}

Vector *pointwise_divide(Vector *u, Vector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("sizes unequal in pointwise divide");
  std::valarray<double> *w = new std::valarray<double>((*u->data())[u->index()]);
  *w /= (*v->data())[v->index()];
  return new Vector(double_ptr(w), std::slice(0, w->size(), 1));
}

CVector *pointwise_divide(Vector *u, CVector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("sizes unequal in pointwise multiplication");
  std::valarray<std::complex<double>> *w = new std::valarray<std::complex<double>>(u->index().size());
  int j = u->index().start();
  int k = v->index().start();
  for (int i = 0; i < (int)w->size(); i++) {
    (*w)[i] = (*u->data())[j] / (*v->data())[k];
    j += u->index().stride();
    k += v->index().stride();
  }
  return new CVector(complex_ptr(w), std::slice(0, w->size(), 1));
}

CVector *pointwise_divide(CVector *u, Vector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("sizes unequal in pointwise multiplication");
  std::valarray<std::complex<double>> *w = new std::valarray<std::complex<double>>((*u->data())[u->index()]);
  int j = v->index().start();
  for (int i = 0; i < (int)w->size(); i++) {
    (*w)[i] /= (*v->data())[j];
    j += v->index().stride();
  }
  return new CVector(complex_ptr(w), std::slice(0, w->size(), 1));
}

CVector *pointwise_divide(CVector *u, CVector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("sizes unequal in pointwise multiplication");
  std::valarray<std::complex<double>> *w = new std::valarray<std::complex<double>>((*u->data())[u->index()]);
  *w /= (*v->data())[v->index()];
  return new CVector(complex_ptr(w), std::slice(0, w->size(), 1));
}

Matrix *pointwise_divide(Matrix *u, Matrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();
  
  if (ui.size()[0] != vi.size()[0] || ui.size()[1] != vi.size()[1])
    mpl_error("sizes unequal inpointwise multiplication");

  int cols = ui.size()[0];
  int rows = ui.size()[1];

  std::valarray<double> *w = new std::valarray<double>((*u->data())[u->index()]);
  (*w) /= (*v->data())[v->index()];
  std::size_t lengths[] = {(size_t)cols, (size_t)rows};
  std::size_t strides[] = {(size_t)rows, 1};
  return new Matrix(double_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths, 2),
					       std::valarray<std::size_t>(strides, 2)));
}

CMatrix *pointwise_divide(Matrix *u, CMatrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();

  if (ui.size()[0] != vi.size()[0] || ui.size()[1] != vi.size()[1])
    mpl_error("sizes unequal in pointwise divide");
  int cols = ui.size()[0];
  int rows = ui.size()[1];
  std::valarray<std::complex<double>> *w = new std::valarray<std::complex<double>>(ui.size()[0]*ui.size()[1]);
  int i = 0;
  int j = ui.start();
  int k = vi.start();
  int m = cols;
  while (m--) {
    int n = rows;
    int jj = j;
    int kk = k;
    while (n--) {
      (*w)[i++] = (*u->data())[jj]/(*v->data())[kk];
      jj += ui.stride()[1];
      kk += vi.stride()[1];
    }
    j += ui.stride()[0];
    k += vi.stride()[0];
  }
  std::size_t lengths[] = {(size_t)cols, (size_t)rows};
  std::size_t strides[] = {(size_t)rows, 1};
  return new CMatrix(complex_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths, 2),
						 std::valarray<std::size_t>(strides, 2)));
}

CMatrix *pointwise_divide(CMatrix *u, Matrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();

  if (ui.size()[0] != vi.size()[0] || ui.size()[1] != vi.size()[1])
    mpl_error("sizes unequal in pointwise divide");
  int cols = ui.size()[0];
  int rows = ui.size()[1];
  std::valarray<std::complex<double>> *w = new std::valarray<std::complex<double>>((*u->data())[u->index()]);
  int i = 0;
  int j = vi.start();
  int m = cols;
  while (m--) {
    int n = rows;
    int k = j;
    while (n--) {
      (*w)[i++] /= (*v->data())[k];
      k += vi.stride()[1];
    }
    j += vi.stride()[0];
  }
  std::size_t lengths[] = {(size_t)cols, (size_t)rows};
  std::size_t strides[] = {(size_t)rows, 1};
  return new CMatrix(complex_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths, 2),
						 std::valarray<std::size_t>(strides, 2)));
}

CMatrix *pointwise_divide(CMatrix *u, CMatrix *v) {
  std::gslice ui = u->index();
  std::gslice vi = v->index();
  
  if (ui.size()[0] != vi.size()[0] || ui.size()[1] != vi.size()[1])
    mpl_error("sizes unequal inpointwise multiplication");

  int cols = ui.size()[0];
  int rows = ui.size()[1];

  std::valarray<std::complex<double>> *w = new std::valarray<std::complex<double>>((*u->data())[u->index()]);
  (*w) /= (*v->data())[v->index()];
  std::size_t lengths[] = {(size_t)cols, (size_t)rows};
  std::size_t strides[] = {(size_t)rows, 1};
  return new CMatrix(complex_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths, 2),
					       std::valarray<std::size_t>(strides, 2)));
}

Value_ptr pointwise_divide(Value *u, Value *v) {
  if (typeid(*u) == typeid(List) || typeid(*v) == typeid(List))
    return dolist(u, v, pointwise_divide);
  if (typeid(*u) == typeid(Vector)) {
    if (typeid(*v) == typeid(Vector))
      return Value_ptr(pointwise_divide((Vector *)u, (Vector *)v));
    if (typeid(*v) == typeid(CVector))
      return Value_ptr(pointwise_divide((Vector *)u, (CVector *)v));
  }
  else if (typeid(*u) == typeid(CVector)) {
    if (typeid(*v) == typeid(Vector))
      return Value_ptr(pointwise_divide((CVector *)u, (Vector *)v));
    if (typeid(*v) == typeid(CVector))
      return Value_ptr(pointwise_divide((CVector *)u, (CVector *)v));
  }
  else if (typeid(*u) == typeid(Matrix)) {
    if (typeid(*v) == typeid(Matrix))
      return Value_ptr(pointwise_divide((Matrix *)u, (Matrix *)v));
    if (typeid(*v) == typeid(CMatrix))
      return Value_ptr(pointwise_divide((Matrix *)u, (CMatrix *)v));
  }
  else if (typeid(*u) == typeid(CMatrix)) {
    if (typeid(*v) == typeid(Matrix))
      return Value_ptr(pointwise_divide((CMatrix *)u, (Matrix *)v));
    if (typeid(*v) == typeid(CMatrix))
      return Value_ptr(pointwise_divide((CMatrix *)u, (CMatrix *)v));
  }
  std::cout << "don't know how to divide " << typeid(*u).name() << " and " << typeid(*v).name() << std::endl;
  mpl_error("type error in pointwise divide");
  return 0;
}

Real *floored_divide(Real *u, Real *v) {
  return new Real(floor(u->value() / v->value()));
}

Value_ptr floored_divide(Value *u, Value *v) {
  if (typeid(*u) == typeid(List) || typeid(*v) == typeid(List))
    return dolist(u, v, floored_divide);
  if (typeid(*u) == typeid(Real) && typeid(*v) == typeid(Real))
    return Value_ptr(floored_divide((Real *)u, (Real *)v));
  mpl_error("type_error");
  return 0;
}

Real *modulo(Real *u, Real *v) {
  double r = fmod(u->value(), v->value());
  if (((r < 0.0) && (v->value() > 0.0)) || ((r > 0.0) && (v->value() < 0.0)))
    r += v->value();
  return new Real(r);
}

Value_ptr modulo(Value *u, Value *v) {
  if (typeid(*u) == typeid(List) || typeid(*v) == typeid(List))
    return dolist(u, v, modulo);
  if (typeid(*u) == typeid(Real) && typeid(*v) == typeid(Real))
    return Value_ptr(modulo((Real *)u, (Real *)v));
  mpl_error("type error");
  return 0;
}

Real *negate(Real *u) {
  return new Real(-u->value());
}

Complex *negate(Complex *u) {
  return new Complex(-u->value());
}

Vector *negate(Vector *u) {
  auto *w = new std::valarray<double>((*u->data())[u->index()]);
  *w = -(*w);
  return new Vector(double_ptr(w), std::slice(0, w->size(), 1));
}

CVector *negate(CVector *u) {
  auto *w = new std::valarray<std::complex<double>>((*u->data())[u->index()]);
  *w = -(*w);
  return new CVector(complex_ptr(w), std::slice(0, w->size(), 1));
}

Matrix *negate(Matrix *u) {
  std::gslice ui = u->index();
  int cols = ui.size()[0];
  int rows = ui.size()[1];
  std::valarray<double> *w = new std::valarray<double>((*u->data())[u->index()]);
  *w = -*w;
  std::size_t lengths[] = {(size_t)cols, (size_t)rows};
  std::size_t strides[] = {(size_t)rows, 1};
  return new Matrix(double_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths,2),
					       std::valarray<std::size_t>(strides, 2)));
}

CMatrix *negate(CMatrix *u) {
  std::gslice ui = u->index();
  int cols = ui.size()[0];
  int rows = ui.size()[1];
  auto *w = new std::valarray<std::complex<double>>((*u->data())[u->index()]);
  *w = -*w;
  std::size_t lengths[] = {(size_t)cols, (size_t)rows};
  std::size_t strides[] = {(size_t)rows, 1};
  return new CMatrix(complex_ptr(w), std::gslice(0, std::valarray<std::size_t>(lengths,2),
						 std::valarray<std::size_t>(strides, 2)));
}

String *negate(String *u) {
  std::valarray<char> x((*u->data())[std::slice(u->index().start()
					      + (u->index().size() - 1)*u->index().stride(),
					      u->index().size(), -u->index().stride())]);
  return new String(x, std::slice(0, u->index().size(), 1));
}

Value_ptr negate(Value *u) {
  if (typeid(*u) == typeid(List))
    return dolist((List *)u, negate);
  if (typeid(*u) == typeid(Real))
    return Value_ptr(negate((Real *)u));
  if (typeid(*u) == typeid(Complex))
    return Value_ptr(negate((Complex *)u));
  if (typeid(*u) == typeid(Vector))
    return Value_ptr(negate((Vector *)u));
  if (typeid(*u) == typeid(CVector))
    return Value_ptr(negate((CVector *)u));
  if (typeid(*u) == typeid(Matrix))
    return Value_ptr(negate((Matrix *)u));
  if (typeid(*u) == typeid(CMatrix))
    return Value_ptr(negate((CMatrix *)u));
  if (typeid(*u) == typeid(String))
    return Value_ptr(negate((String *)u));
  std::cout << "don't know how to negate " << typeid(*u).name() << std::endl;
  mpl_error("type error");
  return 0;
}

Real *power(Real *u, Real *v) {
  return new Real(pow(u->value(), v->value()));
}

Complex *power(Real *u, Complex *v) {
  return new Complex(pow(u->value(), v->value()));
}

Complex *power(Complex *u, Real *v) {
  return new Complex(pow(u->value(), v->value()));
}

Complex *power(Complex *u, Complex *v) {
  return new Complex(pow(u->value(), v->value()));
}

Value_ptr power(Value *u, Value *v) {
  if (typeid(*u) == typeid(List) || typeid(*v) == typeid(List))
    return dolist(u, v, power);
  if (typeid(*u) == typeid(Real)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(power((Real *)u, (Real *)v));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(power((Real *)u, (Complex *)v));
  }
  else if (typeid(*u) == typeid(Complex)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(power((Complex *)u, (Real *)v));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(power((Complex *)u, (Complex *)v));
  }
  mpl_error("type error");
  return 0;
}

/*

 a.start, a.size, a.stride
 b.start, b.size. b.stride

 c.start = a.start + a.stride*b.start;
 c.size = b.size
 c.stride = a.stride*b.stride





 */


std::slice compose(std::slice const &a, std::slice const &b) {
  int new_start = a.stride()*b.start() + a.start();
  int new_size = b.size();
  if ((b.size()-1)*b.stride() + b.start() > a.size())
    mpl_error("slice out of range");
  int new_stride = a.stride()*b.stride();
  return std::slice(new_start, new_size, new_stride);
}

Vector *row_index(Matrix *a, Real *i) {
  int r = round(i->value());
  std::gslice ai = a->index();
  if (r < 0 || r >= (int)ai.size()[1])
    mpl_error("index out of range");
  return new Vector(a->data(), std::slice(ai.start() + r*ai.stride()[1], ai.size()[0], ai.stride()[0]));
}

CVector *row_index(CMatrix *a, Real *i) {
  int r = round(i->value());
  std::gslice ai = a->index();
  if (r < 0 || r >= (int)ai.size()[1])
    mpl_error("index out of range");
  return new CVector(a->data(), std::slice(ai.start() + r*ai.stride()[1], ai.size()[0], ai.stride()[0]));
}

Matrix *row_index(Matrix *a, Slice *i) {
  std::slice r = i->slice();
  std::gslice ai = a->index();
  std::size_t lengths[] { ai.size()[1], r.size() };
  std::size_t strides[] { ai.stride()[0], r.stride()*ai.stride()[1] };
  return new Matrix(a->data(), std::gslice(ai.start() + i->start()*ai.stride()[1],
					   std::valarray<std::size_t>(lengths,2),
					   std::valarray<std::size_t>(strides,2)));
}  

CMatrix *row_index(CMatrix *a, Slice *i) {
  std::slice r = i->slice();
  std::gslice ai = a->index();
  std::size_t lengths[] { ai.size()[1], r.size() };
  std::size_t strides[] { ai.stride()[0], r.stride()*ai.stride()[1] };
  return new CMatrix(a->data(), std::gslice(ai.start() + i->start()*ai.stride()[1],
					   std::valarray<std::size_t>(lengths,2),
					   std::valarray<std::size_t>(strides,2)));
}

Value_ptr row_index(Value *a, Value *i) {
  if (typeid(*a) == typeid(Matrix)) {
    if (typeid(*i) == typeid(Real))
      return Value_ptr(row_index((Matrix *)a, (Real *)i));
    if (typeid(*i) == typeid(Slice))
      return Value_ptr(row_index((Matrix *)a, (Slice *)i));
  }
  if (typeid(*a) == typeid(CMatrix)) {
    if (typeid(*i) == typeid(Real))
      return Value_ptr(row_index((CMatrix *)a, (Real *)i));
    if (typeid(*i) == typeid(Slice))
      return Value_ptr(row_index((CMatrix *)a, (Slice *)i));
  }
  std::cout << typeid(*a).name() << "[" << typeid(*i).name() << "]" << std::endl;
  mpl_error("illegal index 1");
  return 0;
}

Vector *column_index(Matrix *a, Real *i) {
  int c = round(i->value());
  std::gslice ai = a->index();
  if (c < 0 || c >= (int)ai.size()[0])
    mpl_error("index out of range");
  return new Vector(a->data(), std::slice(ai.start() + c*ai.stride()[0], ai.size()[1], ai.stride()[1]));
}

Matrix *column_index(Matrix *a, Slice *i) {
  std::slice c = i->slice();
  std::gslice ai = a->index();
  std::size_t lengths[] { c.size(), ai.size()[1] };
  std::size_t strides[] { c.stride()*ai.stride()[0], ai.stride()[1] };
  return new Matrix(a->data(), std::gslice(ai.start() + i->start()*ai.stride()[0],
					   std::valarray<std::size_t>(lengths,2),
					   std::valarray<std::size_t>(strides,2)));
}  

CVector *column_index(CMatrix *a, Real *i) {
  int c = round(i->value());
  std::gslice ai = a->index();
  if (c < 0 || c >= (int)ai.size()[0])
    mpl_error("index out of range");
  return new CVector(a->data(), std::slice(ai.start() + c*ai.stride()[0], ai.size()[1], ai.stride()[1]));
}

CMatrix *column_index(CMatrix *a, Slice *i) {
  std::slice c = i->slice();
  std::gslice ai = a->index();
  std::size_t lengths[] { c.size(), ai.size()[1] };
  std::size_t strides[] { c.stride()*ai.stride()[0], ai.stride()[1] };
  return new CMatrix(a->data(), std::gslice(ai.start() + i->start()*ai.stride()[0],
					   std::valarray<std::size_t>(lengths,2),
					   std::valarray<std::size_t>(strides,2)));
}  

Value_ptr column_index(Value *a, Value *i) {
  if (typeid(*a) == typeid(Vector)) {
    if (typeid(*i) == typeid(Real))
      return Value_ptr(simple_index((Vector *)a, (Real *)i));
    if (typeid(*i) == typeid(Slice))
      return Value_ptr(simple_index((Vector *)a, (Slice *)i));
  }
  else if (typeid(*a) == typeid(CVector)) {
    if (typeid(*i) == typeid(Real))
      return Value_ptr(simple_index((CVector *)a, (Real *)i));
    if (typeid(*i) == typeid(Slice))
      return Value_ptr(simple_index((CVector *)a, (Slice *)i));
  }
  else if (typeid(*a) == typeid(Matrix)) {
    if (typeid(*i) == typeid(Real))
      return Value_ptr(column_index((Matrix *)a, (Real *)i));
    if (typeid(*i) == typeid(Slice))
      return Value_ptr(column_index((Matrix *)a, (Slice *)i));
  }
  else if (typeid(*a) == typeid(CMatrix)) {
    if (typeid(*i) == typeid(Real))
      return Value_ptr(column_index((CMatrix *)a, (Real *)i));
    if (typeid(*i) == typeid(Slice))
      return Value_ptr(column_index((CMatrix *)a, (Slice *)i));
  }
  std::cout << typeid(*a).name() << "[" << typeid(*i).name() << "]" << std::endl;
  mpl_error("illegal index 2");
  return 0;
}

Real *simple_index(Vector *a, Real *i) {
  int n = round(i->value());
  if (n < 0 || n >= (int)a->index().size())
    mpl_error("index out of range");
  return new Real(a->data(), a->index().stride()*n + a->index().start());
}

Vector *simple_index(Vector *a, Slice *i) {
  return new Vector(a->data(), compose(a->index(), i->slice()));
}

Complex *simple_index(CVector *a, Real *i) {
  int n = round(i->value());
  if (n < 0 || n >= (int)a->index().size())
    mpl_error("index out of range");
  return new Complex(a->data(), a->index().stride()*n + a->index().start());
}
  
CVector *simple_index(CVector *a, Slice *i) {
  return new CVector(a->data(), compose(a->index(), i->slice()));
}

String *simple_index(String *a, Real *i) {
  int n = round(i->value());
  if (n < 0 || n >= (int)a->index().size())
    mpl_error("index out of range");
  std::slice s = a->index();
  string_ptr d = a->data();
  return new String(d, std::slice(s.start()+n, 1, 1));
}
  
String *simple_index(String *a, Slice *i) {
  return new String(a->data(), compose(a->index(), i->slice()));
}

List_Member *simple_index(List *lst, Real *i) {
  int n = round(i->value());
  if (n < 0 || n >= (int)lst->index().size())
    mpl_error("index out of range");
  return new List_Member(lst->data(), lst->index().start() + n*lst->index().stride());
}

List *simple_index(List *lst, Slice *i) {
  return new List(lst->data(), compose(lst->index(), i->slice()));
}

Value_ptr simple_index(Value *a, Value *i) {
  if (typeid(*a) == typeid(Vector)) {
    if (typeid(*i) == typeid(Real))
      return Value_ptr(simple_index((Vector *)a, (Real *)i));
    if (typeid(*i) == typeid(Slice))
      return Value_ptr(simple_index((Vector *)a, (Slice *)i));
  }
  else if (typeid(*a) == typeid(CVector)) {
    if (typeid(*i) == typeid(Real))
      return Value_ptr(simple_index((CVector *)a, (Real *)i));
    if (typeid(*i) == typeid(Slice))
      return Value_ptr(simple_index((CVector *)a, (Slice *)i));
  }
  else if (typeid(*a) == typeid(Matrix)) {
    if (typeid(*i) == typeid(Real))
      return Value_ptr(row_index((Matrix *)a, (Real *)i));
    if (typeid(*i) == typeid(Slice))
      return Value_ptr(row_index((Matrix *)a, (Slice *)i));
  }
  else if (typeid(*a) == typeid(CMatrix)) {
    if (typeid(*i) == typeid(Real))
      return Value_ptr(row_index((CMatrix *)a, (Real *)i));
    if (typeid(*i) == typeid(Slice))
      return Value_ptr(row_index((CMatrix *)a, (Slice *)i));
  }
  else if (typeid(*a) == typeid(String)) {
    if (typeid(*i) == typeid(Real))
      return Value_ptr(simple_index((String *)a, (Real *)i));
    if (typeid(*i) == typeid(Slice))
      return Value_ptr(simple_index((String *)a, (Slice *)i));
  }
  else if (typeid(*a) == typeid(List)) {
    if (typeid(*i) == typeid(Real))
      return Value_ptr(simple_index((List *)a, (Real *)i));
    if (typeid(*i) == typeid(Slice))
      return Value_ptr(simple_index((List *)a, (Slice *)i));
  }
  std::cout << typeid(*a).name() << "[" << typeid(*i).name() << "]" << std::endl;
  mpl_error("illegal index 3");
  return 0;
}

// when u is part of a vector or matrix

void assign(Real *u, Real *v) {
  u->set(v->value());
}

void assign(Complex *u, Complex *v) {
  u->set(v->value());
}

void assign(Vector *u, Vector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("vector size mismatch");
  (*u->data().get())[u->index()] = (*v->data().get())[v->index()];
}

void assign(CVector *u, CVector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("vector size mismatch");
  (*u->data().get())[u->index()] = (*v->data().get())[v->index()];
}

void assign(Matrix *u, Matrix *v) {
  if (u->index().size()[0] != v->index().size()[0]
      || u->index().size()[1] != v->index().size()[1])
    mpl_error("matrix size mismatch");
  (*u->data().get())[u->index()] = (*v->data().get())[v->index()];
}

void assign(CMatrix *u, CMatrix *v) {
  if (u->index().size()[0] != v->index().size()[0]
      || u->index().size()[1] != v->index().size()[1])
    mpl_error("matrix size mismatch");
  (*u->data().get())[u->index()] = (*v->data().get())[v->index()];
}

void assign(String *u, String *v) {
  string_ptr udata = u->data();
  std::slice uindex = u->index();
  string_ptr vdata = v->data();
  std::slice vindex = v->index();
  int minsize = std::min(uindex.size(), vindex.size());
  uindex = std::slice(uindex.start(), minsize, uindex.stride());
  vindex = std::slice(vindex.start(), minsize, vindex.stride());    
  (*udata.get())[uindex] = (*vdata.get())[vindex];
}
    
void assign(List *u, Value_ptr v) { 
  (*u->data().get())[u->index().start()] = v;
}

void assign(List *u, List *v) {
  list_ptr udata = u->data();
  std::slice uindex = u->index();
  list_ptr vdata = v->data();
  std::slice vindex = v->index();
  int minsize = std::min(uindex.size(), vindex.size());
  for (int i = 0; i < minsize; i++) {
    int j = uindex.stride()*i + uindex.start();
    int k = vindex.stride()*i + vindex.start();
    (*udata.get())[j] = (*vdata.get())[k];
  }
}

void assign(List_Member *u, Value_ptr v) {
  (*u->data().get())[u->index()] = v;
}

// v must be a Value_ptr so we can assign it to
// a List_Member! (if needed)

void assign(Value *u, Value_ptr v) {
  if (typeid(*u) == typeid(Memory_Reference))
    ((Memory_Reference *)u)->assign(v);
  else {
    if (typeid(*u) == typeid(Real) && typeid(*v) == typeid(Real))
      assign((Real *)u, (Real *)v.get());
    else if (typeid(*u) == typeid(Complex) && typeid(*v) == typeid(Complex))
      assign((Complex *)u, (Complex *)v.get());
    else if (typeid(*u) == typeid(Vector) && typeid(*v) == typeid(Vector))
      assign((Vector *)u, (Vector *)v.get());
    else if (typeid(*u) == typeid(CVector) && typeid(*v) == typeid(CVector))
      assign((CVector *)u, (CVector *)v.get());
    else if (typeid(*u) == typeid(Matrix) && typeid(*v) == typeid(Matrix))
      assign((Matrix *)u, (Matrix *)v.get());
    else if (typeid(*u) == typeid(CMatrix) && typeid(*v) == typeid(CMatrix))
      assign((CMatrix *)u, (CMatrix *)v.get());
    else if (typeid(*u) == typeid(String) && typeid(*v) == typeid(String))
      assign((String *)u, (String *)v.get());
    else if (typeid(*u) == typeid(List) && typeid(*v) == typeid(List))
      assign((List *)u, (List *)v.get());
    else if (typeid(*u) == typeid(List_Member))
      assign((List_Member *)u, v);
    else {
      std::cout << typeid(*u).name() << " = " << typeid(*v.get()).name() << std::endl;
      mpl_error("illegal assignment");
    }
  }
}
