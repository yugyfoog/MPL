class Code {
public:
  virtual ~Code() { }
  virtual Value_ptr execute() = 0;
};

class Assign : public Code {
private:
  Code *lhs, *rhs;
public:
  Assign(Code *l, Code *r) : lhs(l), rhs(r) { }
  ~Assign() { delete lhs; delete rhs; }
  Value_ptr execute();
};
  
class Print : public Code {
private:
  Code *code;
public:
  Print(Code *c) : code(c) { }
  ~Print() { delete code; }
  Value_ptr execute();
};

class Include : public Code {
private:
  Code *code;
public:
  Include(Code *c) : code(c) { }
  ~Include() { delete code; }
  Value_ptr execute();
};

class Statement : public Code {
private:
  Code *stmt;
  Code *next;
public:
  Statement(Code *s, Code *n) : stmt(s), next(n) { }
  ~Statement() { delete stmt; delete next; }
  Value_ptr execute();
};

class If : public Code {
private:
  Code *condition;
  Code *true_part;
  Code *false_part;
public:
  If(Code *c, Code *t, Code *f) : condition(c), true_part(t), false_part(f) { }
  ~If() { delete condition; delete true_part; delete false_part; }
  Value_ptr execute();
};

class While : public Code {
private:
  Code *condition;
  Code *body;
public:
  While(Code *c, Code *b) : condition(c), body(b) { }
  ~While() { delete condition; delete body; }
  Value_ptr execute();
};

class Repeat : public Code {
private:
  Code *condition;
  Code *body;
public:
  Repeat(Code *c, Code *b) : condition(c), body(b) { }
  ~Repeat() { delete condition; delete body; }
  Value_ptr execute();
};

class For : public Code {
private:
  Code *variable;
  Code *range;
  Code *body;
public:
  For(Code *v, Code *r, Code *b) : variable(v), range(r), body(b) { }
  ~For() { delete variable; delete range; delete body; }
  Value_ptr execute();
};

class Parameter : public Code {
private:
  Code *param;
  Code *next;
public:
  Parameter(Code *p, Code *n) : param(p), next(n) { }
  ~Parameter() { delete param; delete next; }
  Value_ptr execute();
};

class Return : public Code {
private:
  Code *expr;
public:
  Return(Code *e) : expr(e) { }
  ~Return() { delete expr; }
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
  Code *real;
  Code *imag;
public:
  ComplexX(Code *r, Code *i) : real(r), imag(i) { }
  ~ComplexX() { delete real; delete imag; }
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
  Code *list;
public:
  ListX(Code *l) : list(l) { }
  ~ListX() { delete list; }
  Value_ptr execute();
};

// ListPart is code for forming Lists, Vectors, Matricies

class ListPart : public Code {
private:
  Code *member;
  Code *next;
public:
  ListPart(Code *m, Code *n) : member(m), next(n) { }
  ~ListPart() { delete member; delete next; }
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
  Code *base;
  Code *index;
public:
  ColumnIndex(Code *b, Code *i) : base(b), index(i) { }
  ~ColumnIndex() { delete base; delete index; }
  Value_ptr execute();
};

class RowIndex : public Code {
private:
  Code *base;
  Code *index;
public:
  RowIndex(Code *b, Code *i) : base(b), index(i) { }
  ~RowIndex() { delete base; delete index; }
  Value_ptr execute();
};

/*
class MatrixIndex : public Code {
private:
  Code *base;
  Code *row_index;
  Code *col_index;
public:
  MatrixIndex(Code *b, Code *r, Code *c) : base(b), row_index(r), col_index(c) { }
  ~MatrixIndex() { delete base; delete row_index; delete col_index; }
  Value_ptr execute();
};
*/

class Index : public Code {
private:
  Code *base;
  Code *index;
public:
  Index(Code *b, Code *i) : base(b), index(i) { }
  ~Index() { delete base; delete index; }
  Value_ptr execute();
};

class Range : public Code {
private:
  Code *start;
  Code *length;
  Code *stride;
public:
  Range(Code *srt, Code *len, Code *str) : start(srt), length(len), stride(str) { }
  ~Range() { delete start; delete length; delete stride; };
  Value_ptr execute();
};

class Call : public Code {
  std::string func;
  Code *args;
public:
  Call(std::string const &s, Code *c) : func(s), args(c) { }
  ~Call() { delete args; }
  Value_ptr execute();
};

class Or : public Code {
private:
  Code *lhs, *rhs;
public:
  Or(Code *l, Code *r) : lhs(l), rhs(r) { }
  ~Or() { delete lhs; delete rhs; }
  Value_ptr execute();
};

class And : public Code {
private:
  Code *lhs, *rhs;
public:
  And(Code *l, Code *r) : lhs(l), rhs(r) { }
  ~And() { delete lhs; delete rhs; }
  Value_ptr execute();
};

class Not : public Code {
  Code *code;
public:
  Not(Code *c) : code(c) { }
  ~Not() { delete code; }
  Value_ptr execute();
};

class EQ : public Code {
private:
  Code *lhs, *rhs;
public:
  EQ(Code *l, Code *r) : lhs(l), rhs(r) { }
  ~EQ() { delete lhs; delete rhs; }
  Value_ptr execute();
};

class NE : public Code {
private:
  Code *lhs, *rhs;
public:
  NE(Code *l, Code *r) : lhs(l), rhs(r) { }
  ~NE() { delete lhs; delete rhs; }
  Value_ptr execute();
};

class LT : public Code {
private:
  Code *lhs, *rhs;
public:
  LT(Code *l, Code *r) : lhs(l), rhs(r) { }
  ~LT() { delete lhs; delete rhs; }
  Value_ptr execute();
};

class LE : public Code {
private:
  Code *lhs, *rhs;
public:
  LE(Code *l, Code *r) : lhs(l), rhs(r) { }
  ~LE() { delete lhs; delete rhs; }
  Value_ptr execute();
};

class GT : public Code {
private:
  Code *lhs, *rhs;
public:
  GT(Code *l, Code *r) : lhs(l), rhs(r) { }
  ~GT() { delete lhs; delete rhs; }
  Value_ptr execute();
};

class GE : public Code {
private:
  Code *lhs, *rhs;
public:
  GE(Code *l, Code *r) : lhs(l), rhs(r) { }
  ~GE() { delete lhs; delete rhs; }
  Value_ptr execute();
};

class Add : public Code {
private:
  Code *lhs, *rhs;
public:
  Add(Code *l, Code *r) : lhs(l), rhs(r) { }
  ~Add() { delete lhs; delete rhs; }
  Value_ptr execute();
};

class Subtract : public Code {
private:
  Code *lhs, *rhs;
public:
  Subtract(Code *l, Code *r) : lhs(l), rhs(r) { }
  ~Subtract() { delete lhs; delete rhs; }
  Value_ptr execute();
};

class Multiply : public Code {
private:
  Code *lhs, *rhs;
public:
  Multiply(Code *l, Code *r) : lhs(l), rhs(r) { }
  ~Multiply() { delete lhs; delete rhs; }
  Value_ptr execute();
};

class Divide : public Code {
private:
  Code *lhs, *rhs;
public:
  Divide(Code *l, Code *r) : lhs(l), rhs(r) { }
  ~Divide() { delete lhs; delete rhs; }
  Value_ptr execute();
};

class Floored_Divide : public Code {
private:
  Code *lhs, *rhs;
public:
  Floored_Divide(Code *l, Code *r) : lhs(l), rhs(r) { }
  ~Floored_Divide() { delete lhs; delete rhs; }
  Value_ptr execute();
};

class Modulo : public Code {
private:
  Code *lhs, *rhs;
public:
  Modulo(Code *l, Code *r) : lhs(l), rhs(r) { }
  ~Modulo() { delete lhs; delete rhs; }
  Value_ptr execute();
};

class Negate : public Code {
  Code *code;
public:
  Negate(Code *c) : code(c) { }
  ~Negate() { delete code; }
  Value_ptr execute();
};

class Exponent : public Code {
private:
  Code *lhs, *rhs;
public:
  Exponent(Code *l, Code *r) : lhs(l), rhs(r) { }
  ~Exponent() { delete lhs; delete rhs; }
  Value_ptr execute();
};
