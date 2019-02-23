#include <string>
#include <vector>
// #include <utility>
// #include <algorithm>
// #include <set>
// #include <iterator>
// #include <cstring>
// #include <cctype>
// #include <cstdlib>
// #include <stdint.h>
// #include <assert.h>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include <L1.h>
#include <parser.h>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace L1 {

  // bool isRegister(std::string reg){
  //   if (reg == "rdi" || reg == "rsi" || reg == "rdx" || reg == "rcx" || reg == "r8" || reg == "r9" || reg == "rax" || reg == "rbx" || reg == "rbp" || 
  //       reg == "r10" || reg == "r11" || reg == "r12" || reg == "r13" || reg == "r14" || reg == "r15" || reg == "rsp"){
  //           return true;
  //   }
  //   return false;
  // }

  bool isRegister(std::string reg){
    return reg[0] == '%';
  }
  std::string convertRegister(std::string reg){
    return "%" + reg;
  }

  std::string convertConstant(std::string str){
    return "$" + str;
  }

  std::string convertLabel(std::string label){
    return "$_" + label.substr(1);
  }

  

  bool isLabel(std::string reg){
      if (reg[0] == ':'){
          return true;
      }
      return false;
  }

  std::string convert8Bit(std::string reg) {
    if (reg == "%rsi"){
      return "%sil";
    }
    else if (reg == "%rdx"){
        return "%dl";
    }
    else if (reg == "%rdi"){
        return "%dil";
    }
    else if (reg == "%rcx"){
        return "%cl";
    }
    else if (reg == "%rbx"){
        return "%bl";
    }
    else if (reg == "%rbp"){
        return "%bpl";
    }
    else if (reg == "%rax"){
        return "%al";
    }
    else {
        return reg + "b";
    }
  }

  std::string convertMem(std::string reg, std::string offset){
    return offset.substr(1) + "(" + reg + ")";
  }

  /* 
   * Data required to parse
   */ 
  std::vector<std::string> parsed_registers;

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
      right_shift
    > {};

  struct runtime_rule:
    pegtl::sor<
      print,
      allocate,
      array_error
    > {};

  // struct operator_rule_copy:
  //   pegtl::sor<
  //     move,
  //     pluseq,
  //     mineq,
  //     multeq,
  //     andeq,
  //     left_shift,
  //     right_shift
  //   > {};
  
  struct cmp:
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

  struct register_rule:
    pegtl::sor<
      rax, rbx, rbp, r10, r11, r12, r13, r14, r15, 
      rdi, rsi, rdx, rcx, rsp, r8, r9> {};

  struct number_rule:
    number {};

  struct Label_rule:
    label {};

  // struct register_rule:
  //   pegtl::sor<
  //     rax, rbx, rbp, r10, r11, r12, r13, r14, r15, 
  //     rdi, rsi, rdx, rcx, rsp, r8, r9> {};

  // struct number_rule:
  //   number {};

  // struct Label_rule:
  //   label {};
 

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
      cmp,
      seps,
      t_rule
    > {};

  struct Instruction_cjump2_rule:
    pegtl::seq<
      cjump,
      seps,
      t_rule,
      seps,
      cmp,
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
      cmp,
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

  /* 
   * Actions attached to grammar rules.
   */
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

  template<> struct action < Instruction_return_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_ret();
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Label_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
       parsed_registers.push_back(convertLabel(in.string()));
   //  parsed_registers.push_back(in.string());
    }
  };

  template<> struct action < number_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      parsed_registers.push_back(convertConstant(in.string()));
    }
  };

  template<> struct action < register_rule > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      parsed_registers.push_back(convertRegister(in.string()));
    }
  };

  template<> struct action < operator_rule > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      parsed_registers.push_back(in.string());
    }
  };

  template<> struct action < cmp > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      parsed_registers.push_back(in.string());
    }
  };

  template<> struct action < Instruction_Label_rule > {
    template < typename Input >
    static void apply(const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_label();
      i->label = parsed_registers[0];
      currentF->instructions.push_back(i);
      parsed_registers = {};
    }
  };

  // mem reg num op t
  template<> struct action < Instruction_mem_left_rule > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      auto currentF = p.functions.back();
      std::string dest = convertMem(parsed_registers[0], parsed_registers[1]);
      std::string source = parsed_registers[3];
      std::string op = parsed_registers[2];
      if (op == "<-"){
        auto i = new Instruction_mov();
        i->source = source;
        i->destination = dest;
        currentF->instructions.push_back(i);
        parsed_registers = {};
      }
      else if (op == "+="){
        auto i = new Instruction_add();
        i->source = source;
        i->destination = dest;
        currentF->instructions.push_back(i);
        parsed_registers = {};
      }
      else if (op == "-="){
        auto i = new Instruction_sub();
        i->source = source;
        i->destination = dest;
        currentF->instructions.push_back(i);
        parsed_registers = {};
      }
    }
  };

  // reg op mem reg num
  // reg op reg num
  template<> struct action < Instruction_mem_right_rule > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      auto currentF = p.functions.back();
      std::string source = convertMem(parsed_registers[2], parsed_registers[3]);
      std::string dest = parsed_registers[0];
      std::string op = parsed_registers[1];
      if (op == "<-"){
        auto i = new Instruction_mov();
        i->source = source;
        i->destination = dest;
        currentF->instructions.push_back(i);
        parsed_registers = {};
      }
      else if (op == "+="){
        auto i = new Instruction_add();
        i->source = source;
        i->destination = dest;
        currentF->instructions.push_back(i);
        parsed_registers = {};
      }
      else if (op == "-="){
        auto i = new Instruction_sub();
        i->source = source;
        i->destination = dest;
        currentF->instructions.push_back(i);
        parsed_registers = {};
      }
    }
  };

  // dest op source
  template<> struct action < Instruction_three_op_rule > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      auto currentF = p.functions.back();
      std::string source = parsed_registers[2];
      std::string dest = parsed_registers[0];
      std::string op = parsed_registers[1];
      if (op == "<-"){
        auto i = new Instruction_mov();
        i->source = source;
        i->destination = dest;
        currentF->instructions.push_back(i);
        parsed_registers = {};
      }
      else if (op == "+="){
        auto i = new Instruction_add();
        i->source = source;
        i->destination = dest;
        currentF->instructions.push_back(i);
        parsed_registers = {};
      }
      else if (op == "-="){
        auto i = new Instruction_sub();
        i->source = source;
        i->destination = dest;
        currentF->instructions.push_back(i);
        parsed_registers = {};
      }
      else if (op == "*="){
        auto i = new Instruction_mult();
        i->source = source;
        i->destination = dest;
        currentF->instructions.push_back(i);
        parsed_registers = {};
      }
      else if (op == "&="){
        auto i = new Instruction_and();
        i->source = source;
        i->destination = dest;
        currentF->instructions.push_back(i);
        parsed_registers = {};
      }
      else if (op == "<<="){
        auto i = new Instruction_left_shift();
        if (source[0] == '%'){
          source = convert8Bit(source);
        }
        i->source = source;
        i->destination = dest;
        currentF->instructions.push_back(i);
        parsed_registers = {};
      }
      else if (op == ">>="){
        auto i = new Instruction_right_shift();
        if (source[0] == '%'){
          source = convert8Bit(source);
        }
        i->source = source;
        i->destination = dest;
        currentF->instructions.push_back(i);
        parsed_registers = {};
      }
    }
  };

  // t cmp t label label
  template<> struct action < Instruction_cjump2_rule > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_cjump_two();
      i->cmp = parsed_registers[1];
      if (isRegister(parsed_registers[0])){
        i->t1 = parsed_registers[0];
        i->t2 = parsed_registers[2];
        i->isFolded = false;
        i->isSwapped = false;
      }
      else if (!isRegister(parsed_registers[0]) && isRegister(parsed_registers[2])){
        i->t1 = parsed_registers[2];
        i->t2 = parsed_registers[0];
        i->isFolded = false;
        i->isSwapped = true;
      }
      else{
        if (i->cmp == "="){
          i->t1 = std::to_string(std::stoi(parsed_registers[0].substr(1)) == std::stoi(parsed_registers[2].substr(1)));
        }
        else if (i->cmp == "<="){
          i->t1 = std::to_string(std::stoi(parsed_registers[0].substr(1)) <= std::stoi(parsed_registers[2].substr(1)));
        }
        else{
          i->t1 = std::to_string(std::stoi(parsed_registers[0].substr(1)) < std::stoi(parsed_registers[2].substr(1)));
        }
        i->isFolded = true;
        i->isSwapped = false;
      }
      i->l1 = parsed_registers[3];
      i->l2 = parsed_registers[4];
      currentF->instructions.push_back(i);
      parsed_registers = {};
    }
  };

  template<> struct action < Instruction_cjump_rule > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_cjump_one();
      i->cmp = parsed_registers[1];
      if (isRegister(parsed_registers[0])){
        i->t1 = parsed_registers[0];
        i->t2 = parsed_registers[2];
        i->isFolded = false;
        i->isSwapped = false;
      }
      else if (!isRegister(parsed_registers[0]) && isRegister(parsed_registers[2])){
        i->t1 = parsed_registers[2];
        i->t2 = parsed_registers[0];
        i->isFolded = false;
        i->isSwapped = true;
      }
      else{
        if (i->cmp == "="){
          i->t1 = std::to_string(std::stoll(parsed_registers[0].substr(1)) == std::stoll(parsed_registers[2].substr(1)));
        }
        else if (i->cmp == "<="){
          i->t1 = std::to_string(std::stoll(parsed_registers[0].substr(1)) <= std::stoll(parsed_registers[2].substr(1)));
        }
        else{
          i->t1 = std::to_string(std::stoll(parsed_registers[0].substr(1)) < std::stoll(parsed_registers[2].substr(1)));
        }
        i->isFolded = true;
        i->isSwapped = false;
      }
      i->l1 = parsed_registers[3];
      currentF->instructions.push_back(i);
      parsed_registers = {};
    }
  };

  template<> struct action < Instruction_goto_rule > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_jump();
      i->label = parsed_registers[0];
      currentF->instructions.push_back(i);
      parsed_registers = {};
    }
  };

  template<> struct action < Instruction_call_runtime_rule > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_call_runtime();
      i->label = parsed_registers[0];
      currentF->instructions.push_back(i);
      parsed_registers = {};
    }
  };

  template<> struct action < runtime_rule > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      parsed_registers.push_back(in.string());
    }
  };

  template<> struct action < Instruction_call_rule > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_call();
      i->label = parsed_registers[0];
      i->arguments = std::stoi(parsed_registers[1].substr(1));
      currentF->instructions.push_back(i);
      parsed_registers = {};
    }
  };

  // reg t cmp t
  template<> struct action < Instruction_cmp_rule > {
    template< typename Input >
    static void apply(const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_cmp_code();
      i->destination = parsed_registers[0];
      i->inbetween = convert8Bit(parsed_registers[0]);
      i->cmp = parsed_registers[2];
      if (isRegister(parsed_registers[1])){
        i->t1 = parsed_registers[1];
        i->t2 = parsed_registers[3];
        i->isFolded = false;
        i->isSwapped = false;
      }
      else if (!isRegister(parsed_registers[1]) && isRegister(parsed_registers[3])){
        i->t1 = parsed_registers[1];
        i->t2 = parsed_registers[3];
        i->isFolded = false;
        i->isSwapped = true;
      }
      else{
        if (i->cmp == "="){
            i->t1 = "$" + std::to_string(std::stoll(parsed_registers[1].substr(1)) == std::stoll(parsed_registers[3].substr(1)));
        }
        else if (i->cmp == "<="){
            i->t1 = "$" + std::to_string(std::stoll(parsed_registers[1].substr(1)) <= std::stoll(parsed_registers[3].substr(1)));
        }
        else{
            i->t1 = "$" + std::to_string(std::stoll(parsed_registers[1].substr(1)) < std::stoll(parsed_registers[3].substr(1)));
        }
        i->isFolded = true;
        i->isSwapped = false;
      }
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

}
