#ifndef _INSTRUCTION_H_ // -*- C++ -*-
#define _INSTRUCTION_H_

class Instruction {
};

class Print : public Instruction {
};

class Assign : public Instruction {
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

typedef std::list<Instruction *> Code;

#endif
