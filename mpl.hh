extern bool in_function;

extern int locals;
extern int stack_pointer;
extern int frame_pointer;

extern std::stack<std::istream *> file_stack;

class Value;
class Code;
class Function;

typedef std::list<std::string const *> Token_List;

typedef std::map<std::string,int> Symbol_Table;
typedef std::map<std::string,Function *> Function_Table;

extern Symbol_Table local_symbol_table;
extern Symbol_Table global_symbol_table;
extern Function_Table function_table;

extern bool trace_flag;

Value_ptr read_memory(unsigned);
void write_memory(unsigned, Value_ptr);

Value_ptr value(Value_ptr);

[[noreturn]] void mpl_error(std::string const &);
