#ifndef _INSTRUCTION_H_ // -*- C++ -*-
#define _INSTRUCTION_H_

#include <list>
#include <string>

class Instruction {
};

class Print : public Instruction {
};

class Assign : public Instruction {
};

class MakeComplex : public Instruction {
};

class MatVec : public Instruction {
};

class RealConstant : public Instruction {
  double x;
public:
  RealConstant(std::string const &);
};

class StringConstant : public Instruction {
public:
  StringConstant(std::string const &);
};

class Variable : public Instruction {
public:
  Variable(Symbol *);
};

class Call : public Instruction {
public:
  Call(Function *);
};

class Or : public Instruction {
};

class And : public Instruction {
};

class Not : public Instruction {
};

class EQ : public Instruction {
};

class NE : public Instruction {
};

class LE : public Instruction {
};

class LT : public Instruction {
};

class GE : public Instruction {
};

class GT : public Instruction {
};

class Add : public Instruction {
};

class Subtract : public Instruction {
};

class Multiply : public Instruction {
};

class Divide : public Instruction {
};

class Modulus : public Instruction {
};

class Power : public Instruction {
};

class Negate : public Instruction {
};

class Index : public Instruction {
};

class ColumnIndex : public Instruction {
};

class RowIndex : public Instruction {
};

class MatrixIndex : public Instruction {
};

typedef std::list<Instruction *> Code;

#endif
