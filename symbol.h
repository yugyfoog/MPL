#ifndef _SYMBOL_H_ // -*- C++ -*-
#define _SYMBOL_H_

class Symbol {
};

class Local : public Symbol {
};

class Global : public Symbol {
};

class Function : public Symbol {
public:
  int args;
};


#endif
