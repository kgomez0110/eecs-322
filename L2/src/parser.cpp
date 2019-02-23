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

  bool isVar(std::string var){
    return (var[0] == '%' || isRegister(var));
  }
  void addVarGen(std::string var, std::set<Variable*> &gen, L2::Function *f, L2::Instruction *i){
    if (isVar(var)){
      if (f->variables.count(var)){
        f->variables[var]->instructions.push_back(i);
        gen.insert(f->variables[var]);
      }
      else{
        auto var_obj = new Variable(var);
        var_obj->instructions.push_back(i);
        gen.insert(var_obj);
        f->variables[var] = var_obj;
      }
    }
  }

  void addVarKill(std::string var, std::set<Variable*> &kill, L2::Function *f, L2::Instruction *i){
   if (isVar(var)){
    if (f->variables.count(var)){
        f->variables[var]->instructions.push_back(i);
        kill.insert(f->variables[var]);
      }
      else{
        auto var_obj = new Variable(var);
        var_obj->instructions.push_back(i);
        kill.insert(var_obj);
        f->variables[var] = var_obj;
      }
   }
  }

  void addVarGenKill(std::string var, std::set<Variable*> &gen, std::set<Variable*> &kill, L2::Function *f, L2::Instruction *i){
    if (isVar(var)){
      if (f->variables.count(var)){
        f->variables[var]->instructions.push_back(i);
        gen.insert(f->variables[var]);
        kill.insert(f->variables[var]);
      }
      else{
        auto var_obj = new Variable(var);
        var_obj->instructions.push_back(i);
        gen.insert(f->variables[var]);
        kill.insert(var_obj);
        f->variables[var] = var_obj;
      }
    }
  }

  

  bool isRegister(std::string reg){
    return (reg == "rdi" || reg == "rsi" || reg == "rdx" || reg == "rcx" || reg == "r8" || reg == "r9" || reg == "rax" || reg == "rbx" || reg == "rbp" || 
        reg == "r10" || reg == "r11" || reg == "r12" || reg == "r13" || reg == "r14" || reg == "r15" || reg == "rsp");
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

  struct misc_keyword_rule:
    pegtl::sor<
      stack_arg,
      go_to,
      cjump,
      mem,
      call,
      str_return
    >{};

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
      str_return
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
      pegtl::one< ')' >
    > {};

  struct spill_file_rule:
    pegtl::seq<
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

  template<> struct action < misc_keyword_rule > {
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
     // currentF->instructions.push_back(i);
      parsed_registers = {};
    }
  };

  // mem reg num op s
  template<> struct action < Instruction_mem_left_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction(parsed_registers, L2::OTHER);
      std::set<Variable*> gen;
      std::set<Variable*> kill;
      addVarGen(parsed_registers[1], gen, currentF, i);
      addVarGen(parsed_registers[4], gen, currentF, i);
      if (isVar(parsed_registers[1])){
        if (currentF->variables.count(parsed_registers[1])){
          currentF->variables[parsed_registers[1]]->instructions.push_back(i);
          gen.insert(currentF->variables[parsed_registers[1]]);
        }
        else{
          auto var = new Variable(parsed_registers[1]);
          var->instructions.push_back(i);
          gen.insert(var);
          currentF->variables[parsed_registers[1]] = var;
        }
      }
      if (isVar(parsed_registers[4])){
        if (currentF->variables.count(parsed_registers[4])){
          currentF->variables[parsed_registers[4]]->instructions.push_back(i);
          gen.insert(currentF->variables[parsed_registers[4]]);
        }
        else{
          auto var = new Variable(parsed_registers[4]);
          var->instructions.push_back(i);
          gen.insert(var);
          currentF->variables[parsed_registers[4]] = var;
        }
      }
      i->variables_read = gen;
      i->variables_killed = kill;
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
      std::set<Variable*> gen;
      std::set<Variable*> kill;
      if (isVar(parsed_registers[0])){
        std::string var_name = parsed_registers[0];
        if (currentF->variables.count(var_name)){
          currentF->variables[var_name]->instructions.push_back(i);
          kill.insert(currentF->variables[var_name]);
        }
        else{
          auto var = new Variable(var_name);
          var->instructions.push_back(i);
          kill.insert(var);
          currentF->variables[var_name] = var;
        }
      }
      if (isVar(parsed_registers[3])){
        std::string var_name = parsed_registers[3];
        if (currentF->variables.count(var_name)){
          currentF->variables[var_name]->instructions.push_back(i);
          gen.insert(currentF->variables[var_name]);
        }
        else{
          auto var = new Variable(var_name);
          var->instructions.push_back(i);
          gen.insert(var);
          currentF->variables[parsed_registers[4]] = var;
        }
      }
      i->variables_read = gen;
      i->variables_killed = kill;
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
      std::set<Variable*> gen;
      std::set<Variable*> kill;
      if (isVar(parsed_registers[1])){
        std::string var_name = parsed_registers[1];
        if (currentF->variables.count(var_name)){
          currentF->variables[var_name]->instructions.push_back(i);
          gen.insert(currentF->variables[var_name]);
        }
        else{
          auto var = new Variable(var_name);
          var->instructions.push_back(i);
          gen.insert(var);
          currentF->variables[var_name] = var;
        }
      }
      if (isVar(parsed_registers[3])){
        std::string var_name = parsed_registers[3];
        if (currentF->variables.count(var_name)){
          currentF->variables[var_name]->instructions.push_back(i);
          gen.insert(currentF->variables[var_name]);
        }
        else{
          auto var = new Variable(var_name);
          var->instructions.push_back(i);
          gen.insert(var);
          currentF->variables[parsed_registers[4]] = var;
        }
      }
      i->variables_read = gen;
      i->variables_killed = kill;
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
      std::set<Variable*> gen;
      std::set<Variable*> kill;
      if (isVar(parsed_registers[1])){
        std::string var_name = parsed_registers[1];
        if (currentF->variables.count(var_name)){
          currentF->variables[var_name]->instructions.push_back(i);
          gen.insert(currentF->variables[var_name]);
        }
        else{
          auto var = new Variable(var_name);
          var->instructions.push_back(i);
          gen.insert(var);
          currentF->variables[var_name] = var;
        }
      }
      if (isVar(parsed_registers[3])){
        std::string var_name = parsed_registers[3];
        if (currentF->variables.count(var_name)){
          currentF->variables[var_name]->instructions.push_back(i);
          gen.insert(currentF->variables[var_name]);
        }
        else{
          auto var = new Variable(var_name);
          var->instructions.push_back(i);
          gen.insert(var);
          currentF->variables[parsed_registers[4]] = var;
        }
      }
      i->variables_read = gen;
      i->variables_killed = kill;
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
      // TODO STUFFFFF
      auto currentF = p.functions.back();
      auto i = new Instruction(parsed_registers, L2::OTHER);
      i->variables_read = {};
      i->variables_killed = {};
      currentF->instructions.push_back(i);
      parsed_registers = {}; 
    }
  };

  // call u num
  template<> struct action < Instruction_call_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      // TODO STUFF
      auto currentF = p.functions.back();
      auto i = new Instruction(parsed_registers, L2::OTHER);
      i->variables_read = {};
      i->variables_killed = {};
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
      std::set<Variable*> gen;
      std::set<Variable*> kill;
      if (isVar(parsed_registers[0])){
        std::string var_name = parsed_registers[0];
        if (currentF->variables.count(var_name)){
          currentF->variables[var_name]->instructions.push_back(i);
          kill.insert(currentF->variables[var_name]);
        }
        else{
          auto var = new Variable(var_name);
          var->instructions.push_back(i);
          kill.insert(var);
          currentF->variables[var_name] = var;
        }
      }
      i->variables_read = gen;
      i->variables_killed = kill;
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
      std::set<Variable*> gen;
      std::set<Variable*> kill;
      std::string var_name = parsed_registers[0];
      if (currentF->variables.count(var_name)){
        currentF->variables[var_name]->instructions.push_back(i);
        kill.insert(currentF->variables[var_name]);
        gen.insert(currentF->variables[var_name]);
      }
      else{
        auto var = new Variable(var_name);
        var->instructions.push_back(i);
        kill.insert(var);
        gen.insert(var);
        currentF->variables[var_name] = var;
      }
      i->variables_read = gen;
      i->variables_killed = kill;
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
      std::set<Variable*> gen;
      std::set<Variable*> kill;

      std::string var_name = parsed_registers[0];
      if (currentF->variables.count(var_name)){
        currentF->variables[var_name]->instructions.push_back(i);
        kill.insert(currentF->variables[var_name]);
      }
      else{
        auto var = new Variable(var_name);
        var->instructions.push_back(i);
        kill.insert(var);
        currentF->variables[var_name] = var;
      }

      var_name = parsed_registers[2];
      if (currentF->variables.count(var_name)){
        currentF->variables[var_name]->instructions.push_back(i);
        gen.insert(currentF->variables[var_name]);
      }
      else{
        auto var = new Variable(var_name);
        var->instructions.push_back(i);
        gen.insert(var);
        currentF->variables[var_name] = var;
      }

      var_name = parsed_registers[3];
      if (currentF->variables.count(var_name)){
        currentF->variables[var_name]->instructions.push_back(i);
        gen.insert(currentF->variables[var_name]);
      }
      else{
        auto var = new Variable(var_name);
        var->instructions.push_back(i);
        gen.insert(var);
        currentF->variables[var_name] = var;
      }

      i->variables_read = gen;
      i->variables_killed = kill;
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
      std::set<Variable*> gen;
      std::set<Variable*> kill;

      std::string var_name = parsed_registers[0];
      if (currentF->variables.count(var_name)){
        currentF->variables[var_name]->instructions.push_back(i);
        kill.insert(currentF->variables[var_name]);
      }
      else{
        auto var = new Variable(var_name);
        var->instructions.push_back(i);
        kill.insert(var);
        currentF->variables[var_name] = var;
      }

      if (isVar(parsed_registers[2])){
        var_name = parsed_registers[2];
        if (currentF->variables.count(var_name)){
          currentF->variables[var_name]->instructions.push_back(i);
          gen.insert(currentF->variables[var_name]);
        }
        else{
          auto var = new Variable(var_name);
          var->instructions.push_back(i);
          gen.insert(var);
          currentF->variables[var_name] = var;
        }
      }

      if (isVar(parsed_registers[4])){
        var_name = parsed_registers[4];
        if (currentF->variables.count(var_name)){
          currentF->variables[var_name]->instructions.push_back(i);
          gen.insert(currentF->variables[var_name]);
        }
        else{
          auto var = new Variable(var_name);
          var->instructions.push_back(i);
          gen.insert(var);
          currentF->variables[var_name] = var;
        }
      }
      i->variables_read = gen;
      i->variables_killed = kill;
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
      std::set<Variable*> gen;
      std::set<Variable*> kill;
      std::string var_name = parsed_registers[0];
      std::string op = parsed_registers[1];
      if (currentF->variables.count(var_name)){
        currentF->variables[var_name]->instructions.push_back(i);
        kill.insert(currentF->variables[var_name]);
        if (op != "<-"){
          gen.insert(currentF->variables[var_name]);
        }
      }
      else{
        auto var = new Variable(var_name);
        var->instructions.push_back(i);
        currentF->variables[var_name] = var;
        kill.insert(var);
        if (op != "<-"){
          gen.insert(var);
        }
      }

      if (isVar(parsed_registers[2])){
        var_name = parsed_registers[2];
        if (currentF->variables.count(var_name)){
          currentF->variables[var_name]->instructions.push_back(i);
          gen.insert(currentF->variables[var_name]);
        }
        else{
          auto var = new Variable(var_name);
          var->instructions.push_back(i);
          gen.insert(var);
          currentF->variables[var_name] = var;
        }
      }
      i->variables_read = gen;
      i->variables_killed = kill ;
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