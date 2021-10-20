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
#include <random>
#include <chrono>
#include "value.hh"
#include "code.hh"
#include "function.hh"
#include "mpl.hh"

Function::~Function() { }

Value_ptr User_Function::execute(int args) {
  if (args != params)
    mpl_error("function parameter size mismatch");
  stack_pointer += locals;
  if (body) {
    Value_ptr rv = body->execute();
    if (rv)
      return value(rv);
  }
  return 0;
}

Value_ptr Builtin_Function::execute(int args) {
  if (args != params)
    mpl_error("function parameter size mismatch");
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
  return std::make_shared<Real>(y);
}

Value_ptr mpl_gfmt() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) != typeid(Real))
    mpl_error("type error in gfmt");
  std::ostringstream o;
  o << ((Real *)x)->value();
  return std::make_shared<String>(o.str());
}

Value_ptr mpl_ffmt() {
  Value *x = value(read_memory(frame_pointer)).get();
  Value *y = value(read_memory(frame_pointer+1)).get();
  if (typeid(*x) != typeid(Real) || typeid(*y) != typeid(Real))
    mpl_error("type error in ffmt");
  std::ostringstream o;
  o << std::fixed << std::setprecision(round(((Real *)y)->value())) << ((Real *)x)->value();
  return std::make_shared<String>(o.str());
}

Value_ptr mpl_sfmt() {
  Value *x = value(read_memory(frame_pointer)).get();
  Value *y = value(read_memory(frame_pointer+1)).get();
  if (typeid(*x) != typeid(Real) || typeid(*y) != typeid(Real))
    mpl_error("type error in ffmt");
  std::ostringstream o;
  o << std::scientific << std::setprecision(round(((Real *)y)->value())) << ((Real *)x)->value();
  return std::make_shared<String>(o.str());
}

Value_ptr mpl_hfmt() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) != typeid(Real))
    mpl_error("type error in hfmt");
  std::ostringstream o;
  double t = ((Real *)x)->value();
  o << '#' << std::hex << *(unsigned long *)&t;
  return std::make_shared<String>(o.str());
}

Value_ptr mpl_matvec() {
  Character tst;
  
  Value_ptr x = value(read_memory(frame_pointer));
  if (typeid(*x) == typeid(List)) {
    List *u = (List *)x.get();
    characterize(u, &tst);
    if (tst.mv == Character::VECTOR) {
      if (tst.rc == Character::REAL)
	return std::make_shared<Vector>(u, tst.r);
      return std::make_shared<CVector>(u, tst.r);
    }
    if (tst.rc == Character::REAL)
      return std::make_shared<Matrix>(u, tst.r, tst.c);
    return std::make_shared<CMatrix>(u, tst.r, tst.c);
  }
  mpl_error("type error in matvec function");
}


Value_ptr mpl_list() {
  Value_ptr x = value(read_memory(frame_pointer));
  if (typeid(*x) == typeid(Real)) {
    Real *xx = (Real *)x.get();
    int n = round(xx->value());
    return std::make_shared<List>(n);
  }
  mpl_error("type error in list");
}

Value_ptr mpl_vector() {
  Value_ptr x = value(read_memory(frame_pointer));
  if (typeid(*x) == typeid(Real)) {
    Real *xx = (Real *)x.get();
    int n = round(xx->value());
    return std::make_shared<Vector>(n);
  }
  mpl_error("type error");
}

Value_ptr mpl_cvector() {
  Value_ptr x = value(read_memory(frame_pointer));
  if (typeid(*x) == typeid(Real)) {
    Real *xx = (Real *)x.get();
    int n = round(xx->value());
    return std::make_shared<CVector>(n);
  }
  mpl_error("type error in cvector()");
}

Value_ptr mpl_matrix() {
  Value_ptr r = value(read_memory(frame_pointer));
  Value_ptr c = value(read_memory(frame_pointer+1));
  if (typeid(*r) == typeid(Real) && typeid(*c) == typeid(Real)) {
    int rr = round(((Real *)r.get())->value());
    int cc = round(((Real *)c.get())->value());
    return std::make_shared<Matrix>(rr, cc);
  }
  mpl_error("type error in matrix()");
}

Value_ptr mpl_cmatrix() {
  Value_ptr r = value(read_memory(frame_pointer));
  Value_ptr c = value(read_memory(frame_pointer+1));
  if (typeid(*r) == typeid(Real) && typeid(*c) == typeid(Real)) {
    int rr = round(((Real *)r.get())->value());
    int cc = round(((Real *)c.get())->value());
    return std::make_shared<CMatrix>(rr, cc);
  }
  mpl_error("type error in cmatrix()");
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
}

Value_ptr mpl_size() {
  Value_ptr x = value(read_memory(frame_pointer));
  if (typeid(*x) == typeid(Real))
    return std::make_shared<Real>(1.0);
  if (typeid(*x) == typeid(Complex))
    return std::make_shared<Real>(1.0);
  if (typeid(*x) == typeid(Vector))
    return std::make_shared<Real>(((Vector *)x.get())->size());
  if (typeid(*x) == typeid(CVector))
    return std::make_shared<Real>(((CVector *)x.get())->size());
  if (typeid(*x) == typeid(Matrix)) {
    auto lst = std::make_shared<std::vector<Value_ptr>>(2);
    (*lst)[0] = std::make_shared<Real>(((Matrix *)x.get())->rows());
    (*lst)[1] = std::make_shared<Real>(((Matrix *)x.get())->cols());
    return std::make_shared<List>(list_ptr(lst), std::slice(0, 2, 1));
  }
  if (typeid(*x) == typeid(CMatrix)) {
    auto lst = std::make_shared<std::vector<Value_ptr>>(2);
    (*lst)[0] = std::make_shared<Real>(((CMatrix *)x.get())->rows());
    (*lst)[1] = std::make_shared<Real>(((CMatrix *)x.get())->cols());
    return std::make_shared<List>(list_ptr(lst), std::slice(0, 2, 1));
  }
  if (typeid(*x) == typeid(String)) {
    String *u = (String *)x.get();
    std::slice s = u->index();
    return std::make_shared<Real>(s.size());
  }
  if (typeid(*x) == typeid(List)) {
    List *u = (List *)x.get();
    std::slice s = u->index();
    return std::make_shared<Real>(s.size());
  }
  // that should cover everything, but just in case...
  mpl_error("type error in size function");
}

Value_ptr mpl_exit() {
  Value_ptr x = value(read_memory(frame_pointer));
  if (typeid(*x) == typeid(Real)) {
    Real *xx = (Real *)x.get();
    exit(round(xx->value()));
  }
  mpl_error("type error in exit function"); // lost cause?
}

Value_ptr mpl_read() {
  std::string line;
  std::getline(std::cin, line);
  return std::make_shared<String>(line);
}

Value_ptr mpl_eof() {
  return std::make_shared<Real>(std::cin.eof());
}

Value_ptr mpl_type() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real))
    return std::make_shared<Real>(1.0);
  if (typeid(*x) == typeid(Complex))
    return std::make_shared<Real>(2.0);
  if (typeid(*x) == typeid(Vector))
    return std::make_shared<Real>(3.0);
  if (typeid(*x) == typeid(CVector))
    return std::make_shared<Real>(4.0);
  if (typeid(*x) == typeid(Matrix))
    return std::make_shared<Real>(5.0);
  if (typeid(*x) == typeid(CMatrix))
    return std::make_shared<Real>(6.0);
  if (typeid(*x) == typeid(String))
    return std::make_shared<Real>(7.0);
  if (typeid(*x) == typeid(List))
    return std::make_shared<Real>(8.0);
  // should never get here
  mpl_error("illegal type in type function");
}

Value_ptr mpl_ascii() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(String)) {
    String *xx = (String *)x;
    if (xx->index().size() == 0)
      return std::make_shared<Real>(0.0);
    int c = (*xx->data())[xx->index().start()];
    return std::make_shared<Real>(c);
  }
  mpl_error("type error");
}

Value_ptr mpl_char() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    std::string s(1, (char)round(((Real *)x)->value()));
    return std::make_shared<String>(s);
  }
  mpl_error("type error in char()");
}  

std::default_random_engine generator;

Value_ptr mpl_random() {
  std::uniform_real_distribution<double> distribution(0.0, 1.0);
  return std::make_shared<Real>(distribution(generator));
}

Value_ptr mpl_randomize() {
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  generator.seed(seed);
  return 0;
}

Value_ptr mpl_floor() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(floor(u->value()));
  }
  mpl_error("type error in floor function");
}

Value_ptr mpl_ceil() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(ceil(u->value()));
  }
  mpl_error("type error in ceil function");
}

Value_ptr mpl_round() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(floor(u->value()+0.5));
  }
  mpl_error("type error in round function");
}

Value_ptr mpl_real() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Real>(u->value().real());
  }
  mpl_error("type error in real function");
}

Value_ptr mpl_imag() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Real>(u->value().imag());
  }
  mpl_error("type error in imag function");
}

Value_ptr mpl_abs() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(abs(u->value()));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Real>(abs(u->value()));
  }
  mpl_error("type error in abs function");
}

Value_ptr mpl_arg() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Real>(arg(u->value()));
  }    
  mpl_error("type error arg function");
}

Value_ptr mpl_norm() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Real>(norm(u->value()));
  }
  mpl_error("type error in norm function");
}

Value_ptr mpl_conj() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Complex>(conj(u->value()));
  }  
  mpl_error("type error in conj function");
}

Value_ptr mpl_polar() {
  Value *x = value(read_memory(frame_pointer)).get();
  Value *y = value(read_memory(frame_pointer+1)).get();
  if (typeid(*x) == typeid(Real) && typeid(*y) == typeid(Real)) {
    Real *u = (Real *)x;
    Real *v = (Real *)y;
    return std::make_shared<Complex>(std::polar(u->value(), v->value()));
  }
  mpl_error("type error  in polar function");
}

Value_ptr mpl_sqrt() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(sqrt(u->value()));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Complex>(sqrt(u->value()));
  }
  mpl_error("type error in sqrt function");
}

Value_ptr mpl_exp() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(exp(u->value()));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Complex>(exp(u->value()));
  }
  mpl_error("type error in exp function");
}

Value_ptr mpl_expm1() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(expm1(u->value()));
  }
  mpl_error("type error in expm1 function");
}

Value_ptr mpl_log() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(log(u->value()));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Complex>(log(u->value()));
  }
  mpl_error("type error in log function");
}

Value_ptr mpl_log1p() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(log1p(u->value()));
  }
  mpl_error("type error in log1p function");
}

Value_ptr mpl_log10() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(log10(u->value()));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Complex>(log10(u->value()));
  }
  mpl_error("type error in log10 function");
}

Value_ptr mpl_sin() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(sin(u->value()));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Complex>(sin(u->value()));
  }
  mpl_error("type error in sin function");
}

Value_ptr mpl_cos() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(cos(u->value()));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Complex>(cos(u->value()));
  }
  mpl_error("type error in cos function");
}

Value_ptr mpl_tan() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(tan(u->value()));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Complex>(tan(u->value()));
  }
  mpl_error("type error in tan function");
}

Value_ptr mpl_asin() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(asin(u->value()));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Complex>(asin(u->value()));
  }
  mpl_error("type error in asin function");
}

Value_ptr mpl_acos() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(acos(u->value()));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Complex>(acos(u->value()));
  }
  mpl_error("type error in acos function");
}

Value_ptr mpl_atan() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(atan(u->value()));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Complex>(atan(u->value()));
  }
  mpl_error("type error in atan function");
}

Value_ptr mpl_atan2() {
  Value *x = value(read_memory(frame_pointer)).get();
  Value *y = value(read_memory(frame_pointer+1)).get();
  if (typeid(*x) == typeid(Real) && typeid(*y) == typeid(Real)) {
    Real *u = (Real *)x;
    Real *v = (Real *)y;
    return std::make_shared<Real>(atan2(u->value(), v->value()));
  }
  mpl_error("type error in atan2 function");
}

Value_ptr mpl_sinh() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(sinh(u->value()));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Complex>(sinh(u->value()));
  }
  mpl_error("type error in sinh function");
}

Value_ptr mpl_cosh() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(cosh(u->value()));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Complex>(cosh(u->value()));
  }
  mpl_error("type error in cosh function");
}

Value_ptr mpl_tanh() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(tanh(u->value()));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Complex>(tanh(u->value()));
  }
  mpl_error("type error in tanh function");
}

Value_ptr mpl_asinh() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(asinh(u->value()));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Complex>(asinh(u->value()));
  }
  mpl_error("type error in asinh function");
}

Value_ptr mpl_acosh() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(acosh(u->value()));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Complex>(acosh(u->value()));
  }
  mpl_error("type error in acosh function");
}

Value_ptr mpl_atanh() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(atanh(u->value()));
  }
  if (typeid(*x) == typeid(Complex)) {
    Complex *u = (Complex *)x;
    return std::make_shared<Complex>(atanh(u->value()));
  }
  mpl_error("type error in atanh function");
}

Value_ptr mpl_gamma() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(tgamma(u->value()));
  }
  mpl_error("type error in gamma function");
}

Value_ptr mpl_lgamma() {
  Value *x = value(read_memory(frame_pointer)).get();
  if (typeid(*x) == typeid(Real)) {
    Real *u = (Real *)x;
    return std::make_shared<Real>(lgamma(u->value()));
  }
  mpl_error("type error in lgamma function");
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
