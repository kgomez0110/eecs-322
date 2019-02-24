#include <string>
#include <vector>
#include <set>
#include <unordered_map>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include <L2.h>


namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;


namespace L2 {
  
  std::vector<std::string> parsed_registers;

  std::vector<std::string> arguments = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
  std::vector<std::string> caller_save = {"r10", "r11", "r8", "r9", "rax", "rcx", "rdi", "rdx", "rsi"};
  std::vector<std::string> callee_save = {"r12", "r13", "r14", "r15", "rbp", "rbx"};
  std::vector<std::string> gp_registers = {"r10", "r11", "r8", "r9", "rax", "rcx", "rdi", "rdx", "rsi", "r12", "r13", "r14", "r15", "rbp", "rbx"};

  bool isRegister(std::string reg){
    return (reg == "rdi" || reg == "rsi" || reg == "rdx" || reg == "rcx" || reg == "r8" || reg == "r9" || reg == "rax" || reg == "rbx" || reg == "rbp" || 
        reg == "r10" || reg == "r11" || reg == "r12" || reg == "r13" || reg == "r14" || reg == "r15" || reg == "rsp");
  }

  bool isVar(std::string var){
    return (var[0] == '%' || isRegister(var));
  }
  void addVarGen(std::string var, L2::Function *f, L2::Instruction *i){
    if (isVar(var) && var != "rsp"){
      if (f->variables.count(var)){
        f->variables[var]->instructions.push_back(i);
        i->variables_read.insert(f->variables[var]);
      }
      else{
        auto var_obj = new Variable(var);
        var_obj->instructions.push_back(i);
        i->variables_read.insert(var_obj);
        f->variables[var] = var_obj;
      }
      
    }
    return;
  }

  void addVarKill(std::string var, L2::Function *f, L2::Instruction *i){
    if (isVar(var) && var != "rsp"){
      if (f->variables.count(var)){
          f->variables[var]->instructions.push_back(i);
          i->variables_killed.insert(f->variables[var]);
        }
        else{
          auto var_obj = new Variable(var);
          var_obj->instructions.push_back(i);
          i->variables_killed.insert(var_obj);
          f->variables[var] = var_obj;
        }
      }
    return;
  }

  void addVarGenKill(std::string var, L2::Function *f, L2::Instruction *i){
    if (isVar(var) && var != "rsp"){
      if (f->variables.count(var)){
        f->variables[var]->instructions.push_back(i);
        i->variables_read.insert(f->variables[var]);
        i->variables_killed.insert(f->variables[var]);
      }
      else{
        auto var_obj = new Variable(var);
        var_obj->instructions.push_back(i);
        i->variables_read.insert(var_obj);
        i->variables_killed.insert(var_obj);
        f->variables[var] = var_obj;
      }
    }
    return;
  }

  

  /* 
   * Grammar rules from now on.
   */
  struct name:
    pegtl::seq<
      pegtl::plus< 
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >
        >
      >,
      pegtl::star<
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >,
          pegtl::digit
        >
      >
    > {};

  struct label:
    pegtl::seq<
      pegtl::one<':'>,
      name
    > {};

  struct number:
    pegtl::seq<
      pegtl::opt<
        pegtl::sor<
          pegtl::one< '-' >,
          pegtl::one< '+' >
        >
      >,
      pegtl::plus< 
        pegtl::digit
      >
    >{};

  struct function_name:
    label {};

  struct argument_number:
    number {};

  struct local_number:
    number {} ;

  struct comment: 
    pegtl::disable< 
      TAOCPP_PEGTL_STRING( "//" ), 
      pegtl::until< pegtl::eolf > 
    > {};

  /* 
   * Keywords.
   */
  struct str_return : TAOCPP_PEGTL_STRING( "return" ) {};
  struct rax : TAOCPP_PEGTL_STRING("rax") {};
  struct rbx : TAOCPP_PEGTL_STRING("rbx") {};
  struct rbp : TAOCPP_PEGTL_STRING("rbp") {};
  struct r10 : TAOCPP_PEGTL_STRING("r10") {};
  struct r11 : TAOCPP_PEGTL_STRING("r11") {};
  struct r12 : TAOCPP_PEGTL_STRING("r12") {};
  struct r13 : TAOCPP_PEGTL_STRING("r13") {};
  struct r14 : TAOCPP_PEGTL_STRING("r14") {};
  struct r15 : TAOCPP_PEGTL_STRING("r15") {};
  struct rdi : TAOCPP_PEGTL_STRING("rdi") {};
  struct rsi : TAOCPP_PEGTL_STRING("rsi") {};
  struct rdx : TAOCPP_PEGTL_STRING("rdx") {};
  struct rcx : TAOCPP_PEGTL_STRING("rcx") {};
  struct rsp : TAOCPP_PEGTL_STRING("rsp") {};
  struct r8  : TAOCPP_PEGTL_STRING("r8") {};
  struct r9  : TAOCPP_PEGTL_STRING("r9") {};

  struct inc : TAOCPP_PEGTL_STRING("++") {};
  struct dec : TAOCPP_PEGTL_STRING("--") {};
  struct at  : TAOCPP_PEGTL_STRING("@") {};
  struct call : TAOCPP_PEGTL_STRING("call") {};
  struct print : TAOCPP_PEGTL_STRING("print") {};
  struct allocate : TAOCPP_PEGTL_STRING("allocate") {};
  struct array_error : TAOCPP_PEGTL_STRING("array-error") {};
  struct move : TAOCPP_PEGTL_STRING("<-") {};
  struct go_to : TAOCPP_PEGTL_STRING("goto") {};
  struct cjump : TAOCPP_PEGTL_STRING("cjump") {};
  struct mem : TAOCPP_PEGTL_STRING("mem") {};
  struct pluseq : TAOCPP_PEGTL_STRING("+=") {};
  struct mineq : TAOCPP_PEGTL_STRING("-=") {};
  struct multeq : TAOCPP_PEGTL_STRING("*=") {};
  struct andeq : TAOCPP_PEGTL_STRING("&=") {};
  struct left_shift : TAOCPP_PEGTL_STRING("<<=") {};
  struct right_shift : TAOCPP_PEGTL_STRING(">>=") {};
  struct less : TAOCPP_PEGTL_STRING("<"){};
  struct less_eq : TAOCPP_PEGTL_STRING("<=") {};
  struct eq : TAOCPP_PEGTL_STRING("=") {};

  struct stack_arg : TAOCPP_PEGTL_STRING("stack-arg") {};

  struct seps: 
    pegtl::star< 
      pegtl::sor< 
        pegtl::ascii::space, 
        comment 
      > 
    > {};

  struct operator_rule:
    pegtl::sor<
      move,
      pluseq,
      mineq,
      multeq,
      andeq,
      left_shift,
      right_shift,
      at,
      inc,
      dec
    > {};

  struct runtime_rule:
    pegtl::sor<
      print,
      allocate,
      array_error
    > {};
  
  struct cmp_rule:
    pegtl::sor<
      less_eq,
      less,
      eq
    > {};

  struct inc_dec_rule:
    pegtl::sor<
      inc,
      dec
    > {};

  struct variable_rule:
    pegtl::seq<
      pegtl::one< '%' >,
      name
    > {};

  struct register_rule:
    pegtl::sor<
      rax, rbx, rbp, r10, r11, r12, r13, r14, r15, 
      rdi, rsi, rdx, rcx, rsp, r8, r9, variable_rule > {};

  struct number_rule:
    number {};

  struct Label_rule:
    label {};

  struct t_rule:
    pegtl::sor<
      number_rule,
      register_rule
    > {};
  
  struct s_rule: 
    pegtl::sor<
      t_rule,
      Label_rule
    > {};

  struct u_rule:
    pegtl::sor<
      register_rule,
      Label_rule
    > {};

  struct Instruction_return_rule:
    pegtl::seq<
      seps,
      str_return,
      seps
    > { };
  
  struct Instruction_three_op_rule:
    pegtl::seq<
      register_rule,
      seps,
      operator_rule,
      seps,
      s_rule
    > {};

  struct Instruction_mem_left_rule:
    pegtl::seq<
      mem,
      seps,
      register_rule,
      seps,
      number_rule,
      seps,
      operator_rule,
      seps,
      s_rule
    > {};

  struct Instruction_mem_right_rule:
    pegtl::seq<
      register_rule,
      seps,
      operator_rule,
      seps,
      mem,
      seps,
      register_rule,
      seps,
      number_rule
    > {};

  struct Instruction_cmp_rule:
    pegtl::seq<
      register_rule,
      seps,
      move,
      seps,
      t_rule,
      seps,
      cmp_rule,
      seps,
      t_rule
    > {};

  struct Instruction_cjump2_rule:
    pegtl::seq<
      cjump,
      seps,
      t_rule,
      seps,
      cmp_rule,
      seps,
      t_rule,
      seps,
      Label_rule,
      seps,
      Label_rule
    > {};

  struct Instruction_cjump_rule:
    pegtl::seq<
      cjump,
      seps,
      t_rule,
      seps,
      cmp_rule,
      seps,
      t_rule,
      seps,
      Label_rule
    > {};

  struct Instruction_Label_rule:
    pegtl::seq<
      Label_rule
    > {};
  
  struct Instruction_goto_rule:
    pegtl::seq<
      go_to,
      seps,
      Label_rule
    > {};

  struct Instruction_call_runtime_rule:
    pegtl::seq<
      call,
      seps,
      runtime_rule,
      seps,
      number_rule
    > {};

  struct Instruction_call_rule:
    pegtl::seq<
      call,
      seps,
      u_rule,
      seps,
      number_rule
    > {};

  struct Instruction_inc_dec_rule:
    pegtl::seq<
      register_rule,
      seps,
      inc_dec_rule
    > {};

  struct Instruction_at_rule:
    pegtl::seq<
      register_rule,
      seps,
      at,
      seps,
      register_rule,
      seps,
      register_rule,
      seps,
      number_rule
    > {};

  struct Instruction_stack_arg_rule:
    pegtl::seq<
      register_rule,
      seps,
      move,
      seps,
      stack_arg,
      seps,
      number_rule
    > {};


  struct Instruction_rule:
    pegtl::sor<
      pegtl::seq< pegtl::at<Instruction_return_rule>, Instruction_return_rule>,
      pegtl::seq< pegtl::at<Instruction_mem_left_rule>, Instruction_mem_left_rule>,
      pegtl::seq< pegtl::at<Instruction_mem_right_rule>, Instruction_mem_right_rule>,
      pegtl::seq< pegtl::at<Instruction_cjump2_rule>, Instruction_cjump2_rule>,
      pegtl::seq< pegtl::at<Instruction_cjump_rule>, Instruction_cjump_rule>,
      pegtl::seq< pegtl::at<Instruction_goto_rule>, Instruction_goto_rule>,
      pegtl::seq< pegtl::at<Instruction_call_runtime_rule>, Instruction_call_runtime_rule>,
      pegtl::seq< pegtl::at<Instruction_call_rule>, Instruction_call_rule>,
      pegtl::seq< pegtl::at<Instruction_stack_arg_rule>, Instruction_stack_arg_rule>,
      pegtl::seq< pegtl::at<Instruction_inc_dec_rule>, Instruction_inc_dec_rule>,
      pegtl::seq< pegtl::at<Instruction_at_rule>, Instruction_at_rule>,
      pegtl::seq< pegtl::at<Instruction_cmp_rule>, Instruction_cmp_rule>,
      pegtl::seq< pegtl::at<Instruction_three_op_rule>, Instruction_three_op_rule>,
      pegtl::seq< pegtl::at<Instruction_Label_rule>, Instruction_Label_rule>
    > { };

  struct Instructions_rule:
    pegtl::plus<
      pegtl::seq<
        seps,
        Instruction_rule,
        seps
      >
    > { };

  struct Function_rule:
    pegtl::seq<
      seps,
      pegtl::one< '(' >,
      seps,
      function_name,
      seps,
      argument_number,
      seps,
      local_number,
      seps,
      Instructions_rule,
      seps,
      pegtl::one< ')' >,
      seps
    > {};

  struct spill_file_rule:
    pegtl::seq<
      seps,
      Function_rule,
      seps
    > {};

  struct Functions_rule:
    pegtl::plus<
      seps,
      Function_rule,
      seps
    > {};

  struct entry_point_rule:
    pegtl::seq<
      seps,
      pegtl::one< '(' >,
      seps,
      label,
      seps,
      Functions_rule,
      seps,
      pegtl::one< ')' >,
      seps
    > { };

  struct grammar : 
    pegtl::must< 
      entry_point_rule
    > {};

  struct grammar_function:
    pegtl::must<
      Function_rule
    > {};
  
  struct grammar_spill:
    pegtl::must<
      spill_file_rule
    > {};

  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  template<> struct action < label > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        if (p.entryPointLabel.empty()){
          p.entryPointLabel = "_" + in.string().substr(1);
        } else {
          abort();
        }
      }
    };

  template<> struct action < function_name > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto newF = new Function();
      newF->name = "_" + in.string().substr(1);
      p.functions.push_back(newF);
    }
  };

  template<> struct action < argument_number > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->arguments = std::stoll(in.string());
    }
  };

  template<> struct action < local_number > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->locals = std::stoll(in.string());
    }
  };

  template<> struct action < number_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      parsed_registers.push_back(in.string());
    }
  };

  template<> struct action < register_rule > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      parsed_registers.push_back(in.string());
    }
  };

  template<> struct action < operator_rule > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      parsed_registers.push_back(in.string());
    }
  };

  template<> struct action < cmp_rule > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      parsed_registers.push_back(in.string());
    }
  };

  template<> struct action < stack_arg > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      parsed_registers.push_back(in.string());
    }
  };
  template<> struct action < go_to > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      parsed_registers.push_back(in.string());
    }
  };
  template<> struct action < cjump > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      parsed_registers.push_back(in.string());
    }
  };
  template<> struct action < mem > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      parsed_registers.push_back(in.string());
    }
  };
  template<> struct action < call > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      parsed_registers.push_back(in.string());
    }
  };

  template<> struct action < runtime_rule > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      parsed_registers.push_back(in.string());
    }
  };

  template<> struct action < inc_dec_rule > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      parsed_registers.push_back(in.string());
    }
  };

  template<> struct action < at > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      parsed_registers.push_back(in.string());
    }
  };

  template<> struct action < Label_rule > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      parsed_registers.push_back(in.string());
    }
  };

  // return
  template<> struct action < Instruction_return_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      //TODO: store rax and callee saved registers
      auto i = new Instruction(parsed_registers, L2::RET);
      for (std::string reg : callee_save){
        addVarGen(reg, currentF, i);
      }
      addVarGen("rax", currentF, i);
      currentF->instructions.push_back(i);
      parsed_registers = {};
    }
  };

  // mem reg num op s
  template<> struct action < Instruction_mem_left_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction(parsed_registers, L2::OTHER);
      addVarGen(parsed_registers[1], currentF, i);
      addVarGen(parsed_registers[4], currentF, i);
      currentF->instructions.push_back(i);
      parsed_registers = {};
    }
  };

  // w op mem reg num
  template<> struct action < Instruction_mem_right_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction(parsed_registers, L2::OTHER);
      std::string op = parsed_registers[1];
      addVarKill(parsed_registers[0], currentF, i);
      if (op != "<-"){
        addVarGen(parsed_registers[0], currentF, i);
      }
      addVarGen(parsed_registers[3], currentF, i);
      currentF->instructions.push_back(i);
      parsed_registers = {};
    }
  };

  // cjump t cmp t label label
  template<> struct action < Instruction_cjump2_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction(parsed_registers, L2::CJUMP2);
      addVarGen(parsed_registers[1], currentF, i);
      addVarGen(parsed_registers[3], currentF, i);
      currentF->instructions.push_back(i);
      parsed_registers = {};
    } 
  };

  // cjump t cmp t label 
  template<> struct action < Instruction_cjump_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction(parsed_registers, L2::CJUMP);
      addVarGen(parsed_registers[1], currentF, i);
      addVarGen(parsed_registers[3],currentF, i);
      currentF->instructions.push_back(i);
      parsed_registers = {};
    }
  };

  template<> struct action < Instruction_goto_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction(parsed_registers, L2::GOTO);
      i->variables_read = {};
      i->variables_killed = {};
      currentF->instructions.push_back(i);
      parsed_registers = {};
    }
  };

  // call print 2
  template<> struct action < Instruction_call_runtime_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction(parsed_registers, L2::OTHER);
      // for (std::string reg : arguments){
      //   addVarGen(reg, currentF, i);
      // }
      for (int ii = 0; ii < std::stoi(parsed_registers[2]); ii++){
        addVarGen(arguments[ii], currentF, i);
      }
      for (std::string reg : caller_save){
        addVarKill(reg, currentF, i);
      }
      currentF->instructions.push_back(i);
      parsed_registers = {}; 
    }
  };

  // call u num
  template<> struct action < Instruction_call_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction(parsed_registers, L2::OTHER);
      int64_t num_args = std::stoi(parsed_registers[2]);
      for (int ii = 0; ii < num_args; ii++){
        addVarGen(arguments[ii], currentF, i);
      }
      addVarGen(parsed_registers[1], currentF, i);
      for (std::string reg : caller_save){
        addVarKill(reg, currentF, i);
      }
      currentF->instructions.push_back(i);
      parsed_registers = {}; 
    }
  };

  // w <- stack-arg num
  template<> struct action < Instruction_stack_arg_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction(parsed_registers, L2::STACK);
      addVarKill(parsed_registers[0], currentF, i);
      currentF->instructions.push_back(i);
      parsed_registers = {}; 
    }
  };

  // reg++
  template<> struct action < Instruction_inc_dec_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction(parsed_registers, L2::OTHER);
      addVarGenKill(parsed_registers[0], currentF, i);
      currentF->instructions.push_back(i);
      parsed_registers = {}; 
    }
  };

  // w @ w w E
  template<> struct action < Instruction_at_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction(parsed_registers, L2::OTHER);
      addVarKill(parsed_registers[0], currentF, i);
      addVarGen(parsed_registers[2], currentF, i);
      addVarGen(parsed_registers[3], currentF, i);
      currentF->instructions.push_back(i);
      parsed_registers = {}; 
    }
  };

  // w <- t cmp t
  template<> struct action < Instruction_cmp_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction(parsed_registers, L2::OTHER);
      addVarKill(parsed_registers[0], currentF, i);
      addVarGen(parsed_registers[2], currentF, i);
      addVarGen(parsed_registers[4], currentF, i);
      currentF->instructions.push_back(i);
      parsed_registers = {}; 
    }
  };

  // w op t
  template<> struct action < Instruction_three_op_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction(parsed_registers, L2::OTHER);
      std::string op = parsed_registers[1];
      if (op != "<-"){
        addVarGenKill(parsed_registers[0], currentF, i);
      }
      else{
        addVarKill(parsed_registers[0], currentF, i);
      }
      addVarGen(parsed_registers[2], currentF, i);
      if ((op == "<<=" || op == ">>=") && isVar(parsed_registers[2])){
        i->inst = L2::SOPSX;
      }
      currentF->instructions.push_back(i);
      parsed_registers = {}; 
    }
  };
  
  // label
  template<> struct action < Instruction_Label_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      // enum label
      auto currentF = p.functions.back();
      auto i = new Instruction(parsed_registers, L2::LABEL);
      i->variables_killed = {};
      i->variables_read = {};
      currentF->labels[parsed_registers[0]] = i;
      currentF->instructions.push_back(i);
      parsed_registers = {};
    }
  };


  
  Program parse_file (char *fileName){

    /* 
     * Check the grammar for some possible issues.
     */
    pegtl::analyze< grammar >();

    /*
     * Parse.
     */   
    file_input< > fileInput(fileName);
    Program p;
    parse< grammar, action >(fileInput, p);

    return p;
  }

  Function parse_function (char *fileName){
    pegtl::analyze< grammar_function > ();

    file_input< > fileInput(fileName);
    Program p;
    parse< grammar_function, action >(fileInput, p);

    return *p.functions.back();
  }

  Function parse_spill_function (char *fileName){
    pegtl::analyze< grammar_spill > ();

    file_input< > fileInput(fileName);
    Program p;
    parse< grammar_spill, action >(fileInput, p);

    return *p.functions.back();
  }
}