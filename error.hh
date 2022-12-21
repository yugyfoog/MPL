// -*- C++ -*-

class mpl_exception {
private:
  std::string msg;
public:
  mpl_exception(std::string const &m) : msg(m) { }
  ~mpl_exception() { }
  char const *what() const noexcept { return msg.c_str(); }
};



