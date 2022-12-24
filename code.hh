class Code;

typedef std::unique_ptr<Code> Code_Pointer;

class Code {
public:
  virtual ~Code() { }
  virtual Value_ptr execute() = 0;
};

class Assign : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  Assign(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~Assign() { }
  Value_ptr execute();
};
  
class Print : public Code {
private:
  Code_Pointer code;
public:
  Print(Code_Pointer c) : code(std::move(c)) { }
  ~Print() { }
  Value_ptr execute();
};

class Include : public Code {
private:
  Code_Pointer code;
public:
  Include(Code_Pointer c) : code(std::move(c)) { }
  ~Include() { }
  Value_ptr execute();
};

class Statement : public Code {
private:
  Code_Pointer stmt;
  Code_Pointer next;
public:
  Statement(Code_Pointer s, Code_Pointer n) : stmt(std::move(s)), next(std::move(n)) { }
  ~Statement() { }
  Value_ptr execute();
};

class If : public Code {
private:
  Code_Pointer condition;
  Code_Pointer true_part;
  Code_Pointer false_part;
public:
  If(Code_Pointer c, Code_Pointer t, Code_Pointer f) : condition(std::move(c)),
                                                       true_part(std::move(t)),
                                                       false_part(std::move(f)) { }
  ~If() { }
  Value_ptr execute();
};

class While : public Code {
private:
  Code_Pointer condition;
  Code_Pointer body;
public:
  While(Code_Pointer c, Code_Pointer b) : condition(std::move(c)), body(std::move(b)) { }
  ~While() { }
  Value_ptr execute();
};

class Repeat : public Code {
private:
  Code_Pointer condition;
  Code_Pointer body;
public:
  Repeat(Code_Pointer c, Code_Pointer b) : condition(std::move(c)), body(std::move(b)) { }
  ~Repeat() { }
  Value_ptr execute();
};

class For : public Code {
private:
  Code_Pointer variable;
  Code_Pointer range;
  Code_Pointer body;
public:
  For(Code_Pointer v, Code_Pointer r, Code_Pointer b) : variable(std::move(v)),
                                                        range(std::move(r)),
                                                        body(std::move(b)) { }
  ~For() { }
  Value_ptr execute();
};

class Parameter : public Code {
private:
  Code_Pointer param;
  Code_Pointer next;
public:
  Parameter(Code_Pointer p, Code_Pointer n) : param(std::move(p)), next(std::move(n)) { }
  ~Parameter() { }
  Value_ptr execute();
};

class Return : public Code {
private:
  Code_Pointer expr;
public:
  Return(Code_Pointer e) : expr(std::move(e)) { }
  ~Return() { }
  Value_ptr execute();
};

class RealX : public Code {
private:
  double x;
public:
  RealX(std::string const &s);
  ~RealX() { }
  Value_ptr execute();
};

class ComplexX : public Code {
  Code_Pointer real;
  Code_Pointer imag;
public:
  ComplexX(Code_Pointer r, Code_Pointer i) : real(std::move(r)), imag(std::move(i)) { }
  ~ComplexX() { }
  Value_ptr execute();
};

class StringX : public Code {
private:
  std::string str;
public:
  StringX(std::string const &s) { fix_string(s); }
  ~StringX() { }
  Value_ptr execute();
  void fix_string(std::string const &);
};

// ListX is code that takes ListParts and forms a List

class ListX : public Code {
private:
  Code_Pointer list;
public:
  ListX(Code_Pointer l) : list(std::move(l)) { }
  ~ListX() { }
  Value_ptr execute();
};

// ListPart is code for forming Lists, Vectors, Matricies

class ListPart : public Code {
private:
  Code_Pointer member;
  Code_Pointer next;
public:
  ListPart(Code_Pointer m, Code_Pointer n) : member(std::move(m)), next(std::move(n)) { }
  ~ListPart() { }
  Value_ptr execute();
};

class Variable : public Code {
private:
  int index;
public:
  Variable(std::string const &);
  ~Variable() { };
  Value_ptr execute();
};

class ColumnIndex : public Code {
private:
  Code_Pointer base;
  Code_Pointer index;
public:
  ColumnIndex(Code_Pointer b, Code_Pointer i) : base(std::move(b)), index(std::move(i)) { }
  ~ColumnIndex() { }
  Value_ptr execute();
};

class RowIndex : public Code {
private:
  Code_Pointer base;
  Code_Pointer index;
public:
  RowIndex(Code_Pointer b, Code_Pointer i) : base(std::move(b)), index(std::move(i)) { }
  ~RowIndex() { }
  Value_ptr execute();
};

class Index : public Code {
private:
  Code_Pointer base;
  Code_Pointer index;
public:
  Index(Code_Pointer b, Code_Pointer i) : base(std::move(b)), index(std::move(i)) { }
  ~Index() { }
  Value_ptr execute();
};

class Range : public Code {
private:
  Code_Pointer start;
  Code_Pointer length;
  Code_Pointer stride;
public:
  Range(Code_Pointer srt, Code_Pointer len, Code_Pointer str) : start(std::move(srt)),
                                                                length(std::move(len)),
                                                                stride(std::move(str)) { }
  ~Range() { };
  Value_ptr execute();
};

class Call : public Code {
  std::string func;
  Code_Pointer args;
public:
  Call(std::string const &s, Code_Pointer c) : func(s), args(std::move(c)) { }
  ~Call() { }
  Value_ptr execute();
};

class Or : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  Or(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~Or() { }
  Value_ptr execute();
};

class BOr : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  BOr(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~BOr() { }
  Value_ptr execute();
};

class BXor : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  BXor(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~BXor() { }
  Value_ptr execute();
};

class And : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  And(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~And() { }
  Value_ptr execute();
};

class BAnd : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  BAnd(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~BAnd() { }
  Value_ptr execute();
};

class Not : public Code {
  Code_Pointer code;
public:
  Not(Code_Pointer c) : code(std::move(c)) { }
  ~Not() { }
  Value_ptr execute();
};

class BNot : public Code {
  Code_Pointer code;
public:
  BNot(Code_Pointer c) : code(std::move(c)) { }
  ~BNot() { }
  Value_ptr execute();
};

class EQ : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  EQ(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~EQ() { }
  Value_ptr execute();
};

class NE : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  NE(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~NE() { }
  Value_ptr execute();
};

class LT : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  LT(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~LT() { }
  Value_ptr execute();
};

class LE : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  LE(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~LE() { }
  Value_ptr execute();
};

class GT : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  GT(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~GT() { }
  Value_ptr execute();
};

class GE : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  GE(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~GE() { }
  Value_ptr execute();
};

class Add : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  Add(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~Add() { }
  Value_ptr execute();
};

class Subtract : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  Subtract(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~Subtract() { }
  Value_ptr execute();
};

class Multiply : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  Multiply(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~Multiply() { }
  Value_ptr execute();
};

class Pointwise_Multiply : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  Pointwise_Multiply(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~Pointwise_Multiply() { }
  Value_ptr execute();
};

class Divide : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  Divide(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~Divide() { }
  Value_ptr execute();
};

class Pointwise_Divide : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  Pointwise_Divide(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~Pointwise_Divide() { }
  Value_ptr execute();
};

class Floored_Divide : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  Floored_Divide(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~Floored_Divide() { }
  Value_ptr execute();
};

class Modulo : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  Modulo(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~Modulo() { }
  Value_ptr execute();
};

class Negate : public Code {
  Code_Pointer code;
public:
  Negate(Code_Pointer c) : code(std::move(c)) { }
  ~Negate() { }
  Value_ptr execute();
};

class Exponent : public Code {
private:
  Code_Pointer lhs;
  Code_Pointer rhs;
public:
  Exponent(Code_Pointer l, Code_Pointer r) : lhs(std::move(l)), rhs(std::move(r)) { }
  ~Exponent() { }
  Value_ptr execute();
};

class Noop : public Code {
public:
  Noop() { }
  ~Noop() { }
  Value_ptr execute() { return std::make_shared<Null>(); }
};
