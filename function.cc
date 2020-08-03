#include <iostream>
#include <iomanip>
#include <list>
#include <map>
#include <stack>
#include <vector>
#include <string>
#include <complex>
#include <algorithm>
#include <valarray>
#include <memory>
#include <typeinfo>
#include "value.hh"
#include "code.hh"
#include "function.hh"
#include "mpl.hh"

Function::~Function() { }

Value_ptr User_Function::execute(int args) {
  if (args != params) {
    std::cout << "user parameter size mismatch" << std::endl;
    std::cout << "have " << args << ", needed " << params << std::endl;
    exit(1);
  }
  stack_pointer += locals;
  if (body) {
    Value_ptr rv = body->execute();
    if (rv)
      return value(rv);
  }
  return 0;
}

Value_ptr Builtin_Function::execute(int args) {
  if (args != params) {
    std::cout << "builtin parameter size mismatch" << std::endl;
    std::cout << "have " << args << ", needed " << params << std::endl;
    exit(1);
  }
  return (*func)();
}

struct Character {
  enum {MATRIX, VECTOR} mv;
  enum {REAL, COMPLEX} rc;
  int r, c;
};

void characterize(List *lst, Character *chr) {
  std::vector<Value_ptr> *u = lst->data().get();
  std::slice su = lst->index();
  
  chr->rc = Character::REAL;
  chr->r = su.size();
  chr->c = 0;
  if (typeid(*(*u)[su.start()]) == typeid(List)) {
    chr->mv = Character::MATRIX;
    for (int i = 0; i < (int)su.size(); i++) {
      int ii = su.stride()*i + su.start();
      Value *v = (*u)[ii].get();
      if (typeid(*v) != typeid(List))
	mpl_error("illegal list for matvec");
      std::vector<Value_ptr> *vv = ((List *)v)->data().get();
      std::slice sv = ((List *)v)->index();
      if (chr->c < (int)sv.size())
	chr->c = sv.size();
      for (int j = 0; j < (int)sv.size(); j++) {
	int jj = sv.stride()*j + sv.start();
	Value *w = (*vv)[jj].get();
	if (typeid(*w) == typeid(Real))
	  /* ok */;
	else if (typeid(*w) == typeid(Complex))
	  chr->rc = Character::COMPLEX;
	else
	  mpl_error("illegal list for matvec");
      }
    }
  }
  else {
    chr->mv = Character::VECTOR;
    for (int i = 0; i < (int)su.size(); i++) {
      int ii = su.stride()*i + su.start();
      Value *w = (*u)[ii].get();
      if (typeid(*w) == typeid(Real))
	/* ok */;
      else if (typeid(*w) == typeid(Complex))
	chr->rc = Character::COMPLEX;
      else
	mpl_error("illegal list for matvec");
    }
  }
}

Value_ptr mpl_stor() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) != typeid(String))
    mpl_error("string type required");
  std::valarray<char> a(((String *)x)->index().size());
  a[std::slice(0, ((String *)x)->index().size(), 1)] = (*((String *)x)->data().get())[((String *)x)->index()];
  std::string s(&a[0], a.size());
  std::istringstream i(s);
  double y;
  i >> y;
  return Value_ptr(new Real(y));
}

Value_ptr mpl_gfmt() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) != typeid(Real))
    mpl_error("formating requires real");
  std::ostringstream o;
  o << ((Real *)x)->value();
  return Value_ptr(new String(o.str()));
}

Value_ptr mpl_ffmt() {
  Value *x = value(read_memory(frame_pointer)).get();
  Value *y = value(read_memory(frame_pointer+1)).get();
  if (typeid(*x) != typeid(Real) || typeid(*y) != typeid(Real))
    mpl_error("type error in ffmt");
  std::ostringstream o;
  o << std::fixed << std::setprecision(round(((Real *)y)->value())) << ((Real *)x)->value();
  return Value_ptr(new String(o.str()));
}

Value_ptr mpl_sfmt() {
  Value *x = value(read_memory(frame_pointer)).get();
  Value *y = value(read_memory(frame_pointer+1)).get();
  if (typeid(*x) != typeid(Real) || typeid(*y) != typeid(Real))
    mpl_error("type error in ffmt");
  std::ostringstream o;
  o << std::scientific << std::setprecision(round(((Real *)y)->value())) << ((Real *)x)->value();
  return Value_ptr(new String(o.str()));
}
  
Value_ptr mpl_matvec() {
  Character tst;
  
  Value_ptr x = value(read_memory(frame_pointer));
  if (typeid(*x) == typeid(List)) {
    List *u = (List *)x.get();
    characterize(u, &tst);
    if (tst.mv == Character::VECTOR) {
      if (tst.rc == Character::REAL)
	return Value_ptr(new Vector(u, tst.r));
      return Value_ptr(new CVector(u, tst.r));
    }
    if (tst.rc == Character::REAL)
      return Value_ptr(new Matrix(u, tst.r, tst.c));
    return Value_ptr(new CMatrix(u, tst.r, tst.c));
  }
  mpl_error("type error");
  return 0;
}


Value_ptr mpl_list() {
  Value_ptr x = value(read_memory(frame_pointer));
  if (typeid(*x) == typeid(Real)) {
    Real *xx = (Real *)x.get();
    int n = round(xx->value());
    return Value_ptr(new List(n));
  }
  mpl_error("type error");
  return 0;
}

Value_ptr mpl_vector() {
  Value_ptr x = value(read_memory(frame_pointer));
  if (typeid(*x) == typeid(Real)) {
    Real *xx = (Real *)x.get();
    int n = round(xx->value());
    return Value_ptr(new Vector(n));
  }
  mpl_error("type error");
  return 0;
}

Value_ptr mpl_concat() {
  Value_ptr x = value(read_memory(frame_pointer));
  Value_ptr y = value(read_memory(frame_pointer+1));
  if (typeid(*x) == typeid(List)
      && typeid(*y) == typeid(List)) {
    List *xx = (List *)x.get();
    List *yy = (List *)y.get();
    List *zz = new List(xx->index().size() + yy->index().size());
    int i = 0;
    for (int j = 0; j < (int)xx->index().size(); j++, i++) {
      int k = xx->index().stride()*j + xx->index().start();
      (*zz->data())[i] = (*xx->data())[k];
    }
    for (int j = 0; j < (int)yy->index().size(); j++, i++) {
      int k = yy->index().stride()*j + yy->index().start();
      (*zz->data())[i] = (*yy->data())[k];
    }
    return Value_ptr(zz);
  }
  mpl_error("type error");
  return 0;
}

Value_ptr mpl_size() {
  Value_ptr x = value(read_memory(frame_pointer));
  if (typeid(*x) == typeid(Real))
    return Value_ptr(new Real(1.0));
  if (typeid(*x) == typeid(Complex))
    return Value_ptr(new Real(1.0));
  if (typeid(*x) == typeid(Vector))
    return Value_ptr(new Real(((Vector *)x.get())->size()));
  
  // some more types go here
  
  if (typeid(*x) == typeid(String)) {
    String *u = (String *)x.get();
    std::slice s = u->index();
    return Value_ptr(new Real(s.size()));
  }
  if (typeid(*x) == typeid(List)) {
    List *u = (List *)x.get();
    std::slice s = u->index();
    return Value_ptr(new Real(s.size()));
  }
  mpl_error("type error");
  return 0;
}

Value_ptr mpl_exit() {
  Value_ptr x = value(read_memory(frame_pointer));
  if (typeid(*x) == typeid(Real)) {
    Real *xx = (Real *)x.get();
    exit(round(xx->value()));
  }
  mpl_error("type error");
  return 0;
}

Value_ptr mpl_read() {
  std::string line;
  std::getline(std::cin, line);
  return Value_ptr(new String(line));
}

Value_ptr mpl_eof() {
  return Value_ptr(new Real(std::cin.eof()));
}

Value_ptr mpl_type() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real))
    return Value_ptr(new Real(1.0));
  if (typeid(*x) == typeid(Complex))
    return Value_ptr(new Real(2.0));
  if (typeid(*x) == typeid(Vector))
    return Value_ptr(new Real(3.0));
  if (typeid(*x) == typeid(CVector))
    return Value_ptr(new Real(4.0));
  if (typeid(*x) == typeid(Matrix))
    return Value_ptr(new Real(5.0));
  if (typeid(*x) == typeid(CMatrix))
    return Value_ptr(new Real(6.0));
  if (typeid(*x) == typeid(String))
    return Value_ptr(new Real(7.0));
  if (typeid(*x) == typeid(List))
    return Value_ptr(new Real(8.0));
  std::cout << "internal error: illegal type" << std::endl;
  exit(1);
  return 0;
}

Value_ptr mpl_ascii() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(String)) {
    String *xx = (String *)x;
    if (xx->index().size() == 0)
      return Value_ptr(new Real(0.0));
    int c = (*xx->data())[xx->index().start()];
    return Value_ptr(new Real(c));
  }
  mpl_error("type error");
  return 0;
}

Value_ptr mpl_floor() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(floor(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_ceil() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(ceil(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_round() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(floor(u->value()+0.5)));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_real() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) != typeid(Complex)) {
    std::cout << "type error" << std::endl;
    exit(1);
  }
  Complex *u = (Complex *)x;
  return Value_ptr(new Real(u->value().real()));
}

Value_ptr mpl_imag() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) != typeid(Complex)) {
    std::cout << "type error" << std::endl;
    exit(1);
  }
  Complex *u = (Complex *)x;
  return Value_ptr(new Real(u->value().imag()));
}

Value_ptr mpl_abs() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(abs(u->value())));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return Value_ptr(new Real(abs(u->value())));
  }
  std::cout << "type error" << std::endl;
  exit(1);
  return 0;
}

Value_ptr mpl_arg() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) != typeid(Complex)) {
    std::cout << "type error" << std::endl;
    exit(1);
  }
  Complex *u = (Complex *)x;
  return Value_ptr(new Real(arg(u->value())));
}

Value_ptr mpl_norm() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) != typeid(Complex)) {
    std::cout << "type error" << std::endl;
    exit(1);
  }
  Complex *u = (Complex *)x;
  return Value_ptr(new Real(norm(u->value())));
}

Value_ptr mpl_conj() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) != typeid(Complex)) {
    std::cout << "type error" << std::endl;
    exit(1);
  }
  Complex *u = (Complex *)x;
  return Value_ptr(new Complex(conj(u->value())));
}

Value_ptr mpl_polar() {
  Value *x = value(read_memory(frame_pointer)).get();
  Value *y = value(read_memory(frame_pointer+1)).get();
  if (typeid(*x) != typeid(Real)  || typeid(*y) != typeid(Real)) {
    std::cout << "type error" << std::endl;
    exit(1);
  }
  Real *u = (Real *)x;
  Real *v = (Real *)y;
  return Value_ptr(new Complex(std::polar(u->value(), v->value())));
}

Value_ptr mpl_sqrt() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(sqrt(u->value())));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return Value_ptr(new Complex(sqrt(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_exp() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(exp(u->value())));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return Value_ptr(new Complex(exp(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_expm1() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(expm1(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_log() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(log(u->value())));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return Value_ptr(new Complex(log(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_log1p() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(log1p(u->value())));
  }
  std::cout << typeid(x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_log10() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(log10(u->value())));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return Value_ptr(new Complex(log10(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_sin() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(sin(u->value())));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return Value_ptr(new Complex(sin(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_cos() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(cos(u->value())));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return Value_ptr(new Complex(cos(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}


Value_ptr mpl_tan() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(tan(u->value())));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return Value_ptr(new Complex(tan(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_asin() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(asin(u->value())));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return Value_ptr(new Complex(asin(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_acos() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(acos(u->value())));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return Value_ptr(new Complex(acos(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_atan() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(atan(u->value())));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return Value_ptr(new Complex(atan(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_atan2() {
  Value *x = value(read_memory(frame_pointer)).get();
  Value *y = value(read_memory(frame_pointer+1)).get();
  if (typeid(*x) != typeid(Real)  || typeid(*y) != typeid(Real)) {
    std::cout << "type error" << std::endl;
    exit(1);
  }
  Real *u = (Real *)x;
  Real *v = (Real *)y;
  return Value_ptr(new Real(atan2(u->value(), v->value())));
}

Value_ptr mpl_sinh() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(sinh(u->value())));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return Value_ptr(new Complex(sinh(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_cosh() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(cosh(u->value())));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return Value_ptr(new Complex(cosh(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_tanh() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(tanh(u->value())));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return Value_ptr(new Complex(tanh(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_asinh() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(asinh(u->value())));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return Value_ptr(new Complex(asinh(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_acosh() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(acosh(u->value())));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return Value_ptr(new Complex(acosh(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_atanh() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(atanh(u->value())));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return Value_ptr(new Complex(atanh(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_gamma() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(tgamma(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_lgamma() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return Value_ptr(new Real(lgamma(u->value())));
  }
  std::cout << typeid(*x).name() << std::endl;
  std::cout << "type error" << std::endl;
  exit(1);
}

Value_ptr mpl_tr() {
  Value_ptr x = value(read_memory(frame_pointer));
  if (typeid(*x) == typeid(Matrix))
    ((Matrix *)x.get())->transpose();
  else if (typeid(*x) == typeid(CMatrix))
    ((CMatrix *)x.get())->transpose();
  else
    mpl_error("type error in tr()");
  return x;
}
