class Function {
public:
  virtual ~Function();
  virtual Value_ptr execute(int) = 0;
};

class User_Function : public Function {
private:
  int params;
  int locals;
  Code *body;
public:
  User_Function() : params(0), locals(0), body(0) { }
  User_Function(int p, int l, Code *b) : params(p), locals(l), body(b) { }
  ~User_Function() { delete body; }
  Value_ptr execute(int);
};

class Builtin_Function : public Function {
private:
  int params;
  Value_ptr (*func)(void);
public:
  Builtin_Function() : params(0), func(0) { }
  Builtin_Function(int p, Value_ptr (*f)(void)) : params(p), func(f) { }
  ~Builtin_Function() { }
  Value_ptr execute(int);
};

Value_ptr mpl_stor();
Value_ptr mpl_gfmt();
Value_ptr mpl_ffmt();
Value_ptr mpl_sfmt();

Value_ptr mpl_matvec();
Value_ptr mpl_list();
Value_ptr mpl_vector();
Value_ptr mpl_concat();
Value_ptr mpl_size();
Value_ptr mpl_exit();
Value_ptr mpl_read();
Value_ptr mpl_eof();
Value_ptr mpl_type();
Value_ptr mpl_ascii();

Value_ptr mpl_floor();
Value_ptr mpl_ceil();
Value_ptr mpl_round();
Value_ptr mpl_real();
Value_ptr mpl_imag();
Value_ptr mpl_abs();
Value_ptr mpl_arg();
Value_ptr mpl_norm();
Value_ptr mpl_conj();
Value_ptr mpl_polar();

Value_ptr mpl_sqrt();
Value_ptr mpl_exp();
Value_ptr mpl_expm1();
Value_ptr mpl_log();
Value_ptr mpl_log1p();
Value_ptr mpl_log10();

Value_ptr mpl_sin();
Value_ptr mpl_cos();
Value_ptr mpl_tan();
Value_ptr mpl_asin();
Value_ptr mpl_acos();
Value_ptr mpl_atan();
Value_ptr mpl_atan2();
Value_ptr mpl_sinh();
Value_ptr mpl_cosh();
Value_ptr mpl_tanh();
Value_ptr mpl_asinh();
Value_ptr mpl_acosh();
Value_ptr mpl_atanh();
Value_ptr mpl_gamma();
Value_ptr mpl_lgamma();

Value_ptr mpl_tr();
