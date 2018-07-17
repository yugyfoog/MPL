#ifndef _INSTRUCTION_H_ // -*- C++ -*-
#define _INSTRUCTION_H_

class Instruction {
};

class Print : public Instruction {
};

class Assign : public Instruction {
};



typedef std::list<Instruction *> Code;

#endif
