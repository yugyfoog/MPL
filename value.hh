class Value;

typedef std::shared_ptr<Value> Value_ptr;
typedef std::shared_ptr<std::valarray<char>> string_ptr;
typedef std::shared_ptr<std::valarray<double>> double_ptr;
typedef std::shared_ptr<std::valarray<std::complex<double>>> complex_ptr;
typedef std::shared_ptr<std::vector<Value_ptr>> list_ptr;

class Value {
public:
  Value() { }
  virtual ~Value() { };
  virtual std::string print() const = 0;
};

class Real;
class Complex;
class Vector;
class CVector;
class Matrix;
class CMatrix;
class String;
class List;

// Null is used for functions that don't return a value

class Null : public Value {
public:
  Null() { }
  ~Null() { }
  std::string print() const { return "null"; }
};

class Real : public Value {
private:
  double_ptr base;
  int indx;
public:
  Real(double x) {
    base = double_ptr(new std::valarray<double>(x, 1));
    indx = 0;
  }
  Real(double_ptr b, int i) : base(b), indx(i) { }
  ~Real() { }
  std::string print() const;
  double value() const { return (*base.get())[indx]; }
  void set(double x) { (*base)[indx] = x; }
};

class Complex : public Value {
private:
  complex_ptr base;
  int indx;
public:
  Complex(std::complex<double> const &x) {
    base = complex_ptr(new std::valarray<std::complex<double>>(x, 1));
    indx = 0;
  }
  Complex(complex_ptr b, int i) : base(b), indx(i) { }
  ~Complex() { }
  std::string print() const;
  std::complex<double> value() const { return (*base)[indx]; }
  void set(std::complex<double> const &x) { (*base)[indx] = x; }
};

class Vector : public Value {
private:
  double_ptr base;
  std::slice indx;
public:
  Vector(int);
  Vector(double_ptr b, std::slice i) : base(b), indx(i) { }
  Vector(List *, int);
  ~Vector() { }
  std::string print() const;
  double_ptr data() { return base; }
  std::slice index() { return indx; }
  int size() { return indx.size(); }
};

class CVector : public Value {
private:
  complex_ptr base;
  std::slice indx;
public:
  CVector(int);
  CVector(complex_ptr b, std::slice i) : base(b), indx(i) { }
  CVector(List *, int);
  ~CVector() { }
  std::string print() const;
  complex_ptr data() { return base; }
  std::slice index() { return indx; }
};

class Matrix : public Value {
private:
  double_ptr base;
  std::gslice indx;
public:
  Matrix(int, int);
  Matrix(List *, int, int);
  Matrix(double_ptr b, std::gslice const &i) : base(b), indx(i) { }
  ~Matrix() { }
  std::string print() const;
  double_ptr data() { return base; }
  std::gslice index() { return indx; }
  std::gslice *rindex() { return &indx; }
  void transpose();
};

class CMatrix : public Value {
private:
  complex_ptr base;
  std::gslice indx;
public:
  CMatrix(int, int);
  CMatrix(List *, int, int);
  CMatrix(complex_ptr b, std::gslice const &i) : base(b), indx(i) { }
  ~CMatrix() { }
  std::string print() const;
  complex_ptr data() { return base; }
  std::gslice index() { return indx; }
  std::gslice *rindex() { return &indx; }
  void transpose();
};

class String : public Value {
private:
  string_ptr base;
  std::slice i;
public:
  String(std::string const &s);
  String(std::valarray<char> const &, std::slice const &);
  String(string_ptr a, std::slice const &ai) : base(a), i(ai) { };
  ~String() { }
  std::string print() const;
  string_ptr data() { return base; }
  std::slice index() { return i; }
};

class List : public Value {
private:
  list_ptr base;
  std::slice i;
public:
  List();
  List(int);
  List(list_ptr b, std::slice const &s) : base(b), i(s) { }
  List(Value_ptr);
  ~List() { }
  std::string print() const;
  list_ptr data() { return base; }
  std::slice index() { return i; }
  void add_to_front(Value_ptr);
};

// returned by list[i], not list[i:i] nor list[i:i:i]
// List member is not a list but a value in a list

class List_Member : public Value {
private:
  list_ptr base;
  int indx;
public:
  List_Member(list_ptr b, int i) : base(b), indx(i) { }
  ~List_Member() { }
  std::string print() const;
  list_ptr data() { return base; }
  int index() { return indx; }
  Value_ptr value() { return (*base.get())[indx]; }
};

class Slice : public Value {
private:
  int _start;
  int _size;
  int _stride;
public:
  Slice(Value_ptr, Value_ptr, Value_ptr);
  std::string print() const;
  ~Slice() { }
  int start() const { return _start; }
  int size() const { return _size; }
  int stride() const { return _stride; }
  std::slice slice() const { return std::slice(_start, _size, _stride); }
};

class Memory_Reference : public Value {
private:
  unsigned index;
public:
  Memory_Reference(int i);
  ~Memory_Reference() { }
  void assign(Value_ptr);
  std::string print() const;
  Value_ptr value();
};

