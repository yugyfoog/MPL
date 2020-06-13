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
  for (int i = 0; i < n; i++) {
    int j = u->index().stride()*i + u->index().start();
    int k = v->index().stride()*i + v->index().start();
    if ((*u->data())[j] != (*v->data())[k])
      return new Real(0.0);
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
      return Value_ptr(eq((Vector *)v, (Vector *)v));
  }
  else if (typeid(*u) == typeid(String)) {
    if (typeid(*v) == typeid(String))
      return Value_ptr(eq((String *)u, (String *)v));
  }
  std::cout << "don't know how to eq " << type_name(u) << " and " << type_name(v) << std::endl;
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
  for (int i = 0; i < n; i++) {
    int j = u->index().stride()*i + u->index().start();
    int k = v->index().stride()*i + v->index().start();
    if ((*u->data())[j] != (*v->data())[k])
      return new Real(1.0);
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

String *add(String *u, String *v) {
  int size = u->index().size() + v->index().size();
  std::valarray<char> x(size);
  x[std::slice(0, u->index().size(), 1)] = (*u->data())[u->index()];
  x[std::slice(u->index().size(), size, 1)] = (*v->data())[v->index()];
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
  }
  else if (typeid(*u) == typeid(String)) {
    if (typeid(*v) == typeid(String))
      return Value_ptr(add((String *)u, (String *)v));
  }
  std::cout << "don't know how to add " << type_name(u) << " and " << type_name(v) << std::endl;
  mpl_error("type error");
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
    mpl_error("sizes unequal in vector addition");
  std::valarray<double> *w = new std::valarray<double>((*u->data())[u->index()]);
  *w -= (*v->data())[v->index()];
  return new Vector(double_ptr(w), std::slice(0, w->size(), 1));
}

String *subtract(String *u, String *v) {
  int size = u->index().size() + v->index().size();
  std::valarray<char> x(size);
  x[std::slice(0, u->index().size(), 1)] = (*u->data())[u->index()];
  x[std::slice(u->index().size(), size, 1)] = (*v->data())[std::slice(u->index().start()
							      + (u->index().size() - 1)*u->index().stride(),
							      u->index().size(), -u->index().stride())];
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
  }
  else if (typeid(*u) == typeid(String)) {
    if (typeid(*v) == typeid(String))
      return Value_ptr(subtract((String *)u, (String *)v));
  }
  mpl_error("type error");
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
  std::valarray<double> *w = new std::valarray<double>((*v->data())[v->index()]);
  *w *= u->value();
  return new Vector(double_ptr(w), std::slice(0, w->size(), 1));
}

Real *multiply(Vector *u, Vector *v) {
  if (u->index().size() != v->index().size())
    mpl_error("sizes unequal in vector multiplication");
  int n = u->index().size();
  double p = 0.0;
  for (int i = 0; i < n; i++) {
    int j = u->index().stride()*i + u->index().start();
    int k = v->index().stride()*i + v->index().start();
    p += (*u->data())[j]*(*v->data())[k];
  }
  return new Real(p);
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
    if (typeid(*v) == typeid(String))
      return Value_ptr(multiply((Real *)u, (String *)v));
  }
  else if (typeid(*u) == typeid(Complex)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(multiply((Real *)v, (Complex *)u));
    if (typeid(*v) == typeid(Complex))
      return Value_ptr(multiply((Complex *)u, (Complex *)v));
  }
  else if (typeid(*u) == typeid(Vector)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(multiply((Real *)v, (Vector *)u));
    if (typeid(*v) == typeid(Vector))
      return Value_ptr(multiply((Vector *)u, (Vector *)v));
  }
  else if (typeid(*u) == typeid(String)) {
    if (typeid(*v) == typeid(Real))
      return Value_ptr(multiply((Real *)v, (String *)u));
  }
  std::cout << "don't know how to multiply " << type_name(u) << " and " << type_name(v) << std::endl;
  mpl_error("type error");
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
  }
  std::cout << "don't know how to divide " << type_name(u) << " and " << type_name(v) << std::endl;
  mpl_error("type_error");
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
  std::valarray<double> *w = new std::valarray<double>((*u->data())[u->index()]);
  *w = -(*w);
  return new Vector(double_ptr(w), std::slice(0, w->size(), 1));
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
  if (typeid(*u) == typeid(String))
    return Value_ptr(negate((String *)u));
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

std::slice compose(std::slice const &a, std::slice const &b) {
  int new_start = a.stride()*b.start() + a.start();
  int new_size = std::min((a.size() - b.start() - 1)/b.stride() + 1, b.size());
  int new_stride = a.stride()*b.stride();
  return std::slice(new_start, new_size, new_stride);
}

Real *simple_index(Vector *a, Real *i) {
  int n = round(i->value());
  if (n >= (int)a->index().size())
    mpl_error("index out of range");
  return new  Real((*a->data())[a->index().stride()*n + a->index().start()]);
}

Vector *simple_index(Vector *a, Slice *i) {
  return new Vector(a->data(), compose(a->index(), i->slice()));
}

String *simple_index(String *a, Real *i) {
  int n = round(i->value());
  if (n >= (int)a->index().size())
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
  mpl_error("illegal index: " + type_name(a) + "[" + type_name(i) + "]");
  return 0;
}

// when u is part of a vector or matrix

void assign(Real *u, Real *v) {
  u->set(v->value());
}

void assign(Vector *u, Vector *v) {
  double_ptr udata = u->data();
  std::slice uindex = u->index();
  double_ptr vdata = v->data();
  std::slice vindex = v->index();
  int minsize = std::min(uindex.size(), vindex.size());
  uindex = std::slice(uindex.start(), minsize, uindex.stride());
  vindex = std::slice(vindex.start(), minsize, uindex.stride());
  (*udata.get())[uindex] = (*vdata.get())[vindex];
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
// a List_Member!

void assign(Value *u, Value_ptr v) {
  if (typeid(*u) == typeid(Memory_Reference))
    ((Memory_Reference *)u)->assign(v);
  else {
    if (typeid(*u) == typeid(Real) && typeid(*v) == typeid(Real))
      assign((Real *)u, (Real *)v.get());
    else if (typeid(*u) == typeid(Vector) && typeid(*v) == typeid(Vector))
      assign((Vector *)u, (Vector *)v.get());
    else if (typeid(*u) == typeid(String) && typeid(*v) == typeid(String))
      assign((String *)u, (String *)v.get());
    else if (typeid(*u) == typeid(List) && typeid(*v) == typeid(List))
      assign((List *)u, (List *)v.get());
    else if (typeid(*u) == typeid(List_Member))
      assign((List_Member *)u, v);
    else
      mpl_error(type_name(u) + " = " + type_name(v.get()));
  }
}
