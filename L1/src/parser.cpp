#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <assert.h>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include <L1.h>
#include <parser.h>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace L1 {

  /* 
  
   * Data required to parse
   */ 
  std::vector<Item> parsed_registers;

  std::vector<std::string> split(std::string text){
    std::istringstream iss(text);
    std::vector<std::string> results(std::istream_iterator<std::string>{iss},
      std::istream_iterator<std::string>());
    return results;
  }

  std::string addSpaces(std::string text, std::string op){
    int index = text.find(op);
    std::string newString = text.substr(0, index) + " " + op + " " + text.substr(index + op.length(), text.length() - (op.length() + index));
    return newString;
  }


  bool isRegister(std::string reg){
    if (reg == "rdi" || reg == "rsi" || reg == "rdx" || reg == "rcx" || reg == "r8" || reg == "r9" || reg == "rax" || reg == "rbx" || reg == "rbp" || 
        reg == "r10" || reg == "r11" || reg == "r12" || reg == "r13" || reg == "r14" || reg == "r15" || reg == "rsp"){
            return true;
    }
    return false;
  }

  std::string convertRegister(std::string reg){
    try{
        std::stoll(reg);
        return "$" + reg;
    }
    catch (std::invalid_argument& e){
        if (isRegister(reg)){
            return "%" + reg;
        }
        else {
            return "_" + reg.substr(1);
        }
    }
  }

  bool isLabel(std::string reg){
      if (reg[0] == ':'){
          return true;
      }
      return false;
  }

  std::string convertJumpLabel(std::string label){
    return "_" + label.substr(1) + ":";
  }

  std::string convertLabel(std::string label){
    return "$_" + label.substr(1);
  }

  std::string convert8Bit(std::string reg) {
      if (reg == "rsi"){
        return "sil";
      }
      else if (reg == "rdx"){
          return "dl";
      }
      else if (reg == "rdi"){
          return "dil";
      }
      else if (reg == "rcx"){
          return "cl";
      }
      else if (reg == "rbx"){
          return "bl";
      }
      else if (reg == "rbp"){
          return "bpl";
      }
      else if (reg == "rax"){
          return "al";
      }
      else {
          return reg + "b";
      }

  }

  std::string convertMem(std::string reg, std::string offset){
    return offset + "(" + reg + ")";
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

  struct seps: 
    pegtl::star< 
      pegtl::sor< 
        pegtl::ascii::space, 
        comment 
      > 
    > {};

  struct Label_rule:
    label {};


  // struct Instruction_rule:
  //   pegtl::sor<
  //     pegtl::seq< pegtl::at<Instruction_return_rule>            , Instruction_return_rule             >
  //   > { };
  
  ////////////////// my stuff
  // registers
  struct rdi : TAOCPP_PEGTL_STRING("rdi") {};
  struct rsi : TAOCPP_PEGTL_STRING("rsi") {};
  struct rdx : TAOCPP_PEGTL_STRING("rdx") {};
  struct rcx : TAOCPP_PEGTL_STRING("rcx") {};
  struct r8  : TAOCPP_PEGTL_STRING("r8")  {};
  struct r9  : TAOCPP_PEGTL_STRING("r9")  {};
  struct rax : TAOCPP_PEGTL_STRING("rax") {};
  struct rbx : TAOCPP_PEGTL_STRING("rbx") {};
  struct rbp : TAOCPP_PEGTL_STRING("rbp") {};
  struct r10 : TAOCPP_PEGTL_STRING("r10") {};
  struct r11 : TAOCPP_PEGTL_STRING("r11") {};
  struct r12 : TAOCPP_PEGTL_STRING("r12") {};
  struct r13 : TAOCPP_PEGTL_STRING("r13") {};
  struct r14 : TAOCPP_PEGTL_STRING("r14") {};
  struct r15 : TAOCPP_PEGTL_STRING("r15") {};
  struct rsp : TAOCPP_PEGTL_STRING("rsp") {};

  // arithmetic operations 
  struct plus_eq : TAOCPP_PEGTL_STRING("+=") {};
  struct min_eq  : TAOCPP_PEGTL_STRING("-=") {};
  struct time_eq : TAOCPP_PEGTL_STRING("*=") {};
  struct eq_and  : TAOCPP_PEGTL_STRING("&=") {};

  // shift operations
  struct left_shift  : TAOCPP_PEGTL_STRING("<<=") {};
  struct right_shift : TAOCPP_PEGTL_STRING(">>=") {};

  // comparison operations
  struct less    : TAOCPP_PEGTL_STRING("<")  {};
  struct less_eq : TAOCPP_PEGTL_STRING("<=") {};
  struct eq      : TAOCPP_PEGTL_STRING("=")  {};

  // misc ops
  struct move : TAOCPP_PEGTL_STRING("<-") {};
  struct mem  : TAOCPP_PEGTL_STRING("mem") {};
  struct array_err : TAOCPP_PEGTL_STRING("array-error") {};
  struct allocate : TAOCPP_PEGTL_STRING("allocate") {};
  struct print : TAOCPP_PEGTL_STRING("print") {};
  struct call : TAOCPP_PEGTL_STRING("call") {};
  struct jump : TAOCPP_PEGTL_STRING("cjump") {};
  struct go_to : TAOCPP_PEGTL_STRING("goto") {};
  struct inc : TAOCPP_PEGTL_STRING("++") {};
  struct dec : TAOCPP_PEGTL_STRING("--") {}; 

  struct sx_rule:
    rcx {};

  struct a_rule:
    pegtl::sor<
        rdi,
        rsi,
        rdx,
        sx_rule,
        r8,
        r9
    > {};

  struct w_rule:
    pegtl::sor<
      a_rule,
      rax,
      rbx,
      rbp,
      r10,
      r11,
      r12,
      r13,
      r14,
      r15
    > {};

struct x_rule:
    pegtl::sor< 
      w_rule,
      rsp
    > {};

struct t_rule:
  pegtl::sor< 
    x_rule,
    number
  > {};

struct s_rule:
  pegtl::sor< 
      t_rule,
      Label_rule
    > {};

struct u_rule:
  pegtl::sor< 
    w_rule,
    Label_rule
  > {};


struct aop_rule:
  pegtl::sor< 
    plus_eq,
    min_eq,
    time_eq,
    eq_and
  > {};

struct sop_rule:
  pegtl::sor< 
    left_shift,
    right_shift
  > {};

struct cmp_rule:
  pegtl::sor< 
    less_eq,
    less,
    eq 
  > {};

  struct E_rule: pegtl::sor< 
    pegtl::one<'1'>, 
    pegtl::one<'2'>,
    pegtl::one<'4'>, 
    pegtl::one<'8'>
  > {};

  struct M_rule:
    number {};

struct Instruction_return_rule:
    pegtl::seq<
      str_return
    > { };

struct Instruction_s_move_w_rule:
  pegtl::seq< 
        w_rule,
        seps,
        move,
        seps,
        s_rule 
      > {};

struct Instruction_mem_move_w_rule:
  pegtl::seq< 
        w_rule,
        seps,
        move,
        seps,
        mem,
        seps,
        x_rule,
        seps,
        M_rule
      > {};

struct Instruction_mov_cmp_rule:
  pegtl::seq<
        w_rule,
        seps,
        move,
        seps,
        t_rule,
        seps,
        cmp_rule,
        seps,
        t_rule
      > {};


struct Instruction_s_move_mem_rule:
  pegtl::seq<
        mem,
        seps,
        x_rule,
        seps,
        M_rule,
        seps,
        move,
        seps,
        s_rule 
      > {};

struct Instruction_aop_rule:
  pegtl::seq<
        w_rule,
        seps,
        aop_rule,
        seps,
        t_rule
      > {};

struct Instruction_sop_sx_rule:
  pegtl::seq<
       w_rule,
       seps,
       sop_rule,
       seps,
       sx_rule
      > {};

struct Instruction_sop_n_rule:
  pegtl::seq<
        w_rule,
        seps,
        sop_rule,
        seps,
        number
      > {};

struct Instruction_mem_pluseq_rule:
  pegtl::seq<
        mem,
        seps,
        x_rule,
        seps,
        M_rule,
        seps,
        plus_eq,
        seps,
        t_rule
      > {};

struct Instruction_mem_mineq_rule:
  pegtl::seq<
        mem,
        seps,
        x_rule,
        seps,
        M_rule,
        seps,
        min_eq,
        seps,
        t_rule
      > {};

struct Instruction_w_pluseq_rule:
 pegtl::seq<
        w_rule,
        seps,
        plus_eq,
        seps,
        mem,
        seps,
        x_rule,
        seps,
        M_rule
      > {};

struct Instruction_w_mineq_rule:
  pegtl::seq<
        w_rule,
        seps,
        min_eq,
        seps,
        mem,
        seps,
        x_rule,
        seps,
        M_rule
      > {};

struct Instruction_cjump_two_rule:
  pegtl::seq<
        jump,
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

struct Instruction_cjump_one_rule:
  pegtl::seq<
       jump,
       seps,
       t_rule,
       seps,
       cmp_rule,
       seps,
       t_rule,
       seps,
       Label_rule
      >{};

struct Instruction_label_rule:
  Label_rule {};

struct Instruction_goto_rule:
  pegtl::seq< go_to, seps, Label_rule> {};
 
struct Instruction_call_rule:
  pegtl::seq<
        call,
        seps,
        u_rule,
        seps,
        number
      > {};

struct Instruction_call_print_rule:
  pegtl::seq<
        call,
        seps,
        print,
        seps,
        pegtl::one<'1'>
      > {};

struct Instruction_call_allocate_rule:
  pegtl::seq<
        call,
        seps,
        allocate,
        seps,
        pegtl::one<'2'>
      >{};

struct Instruction_array_error_rule:
  pegtl::seq<
        call,
        seps,
        array_err,
        seps,
        pegtl::one<'2'>
      >{};

struct Instruction_increment_rule:
  pegtl::sor<
    pegtl::seq< w_rule, inc >,
    pegtl::seq< w_rule, seps, inc >
  > {};

struct Instruction_decrement_rule:
  pegtl::sor<
    pegtl::seq< w_rule, dec >,
    pegtl::seq< w_rule, seps, dec > 
  >{};

struct Instruction_email_rule:
  pegtl::seq<
        w_rule,
        seps,
        pegtl::one<'@'>,
        seps,
        w_rule,
        seps,
        w_rule,
        seps,
        E_rule
      >{};

struct Instruction_rule:
    pegtl::sor<
      pegtl::seq< pegtl::at<Instruction_mov_cmp_rule>, Instruction_mov_cmp_rule>,
      pegtl::seq< pegtl::at<Instruction_s_move_w_rule> , Instruction_s_move_w_rule>,
      pegtl::seq< pegtl::at<Instruction_mem_move_w_rule>, Instruction_mem_move_w_rule>,
      pegtl::seq< pegtl::at<Instruction_s_move_mem_rule>, Instruction_s_move_mem_rule>,
      pegtl::seq< pegtl::at<Instruction_aop_rule>, Instruction_aop_rule>,
      pegtl::seq< pegtl::at<Instruction_sop_sx_rule>, Instruction_sop_sx_rule>,
      pegtl::seq< pegtl::at<Instruction_sop_n_rule>, Instruction_sop_n_rule>,
      pegtl::seq< pegtl::at<Instruction_mem_pluseq_rule>, Instruction_mem_pluseq_rule>,
      pegtl::seq< pegtl::at<Instruction_mem_mineq_rule>, Instruction_mem_mineq_rule>,
      pegtl::seq< pegtl::at<Instruction_w_pluseq_rule>, Instruction_w_pluseq_rule>,
      pegtl::seq< pegtl::at<Instruction_w_mineq_rule>, Instruction_w_mineq_rule>,
      pegtl::seq< pegtl::at<Instruction_cjump_two_rule>, Instruction_cjump_two_rule>,
      pegtl::seq< pegtl::at<Instruction_cjump_one_rule>, Instruction_cjump_one_rule>,
      pegtl::seq< pegtl::at<Instruction_return_rule>, Instruction_return_rule>,
      pegtl::seq< pegtl::at<Instruction_label_rule>, Instruction_label_rule>,
      pegtl::seq< pegtl::at<Instruction_goto_rule>, Instruction_goto_rule>,
      pegtl::seq< pegtl::at<Instruction_call_print_rule>, Instruction_call_print_rule>,
      pegtl::seq< pegtl::at<Instruction_call_allocate_rule>, Instruction_call_allocate_rule>,
      pegtl::seq< pegtl::at<Instruction_array_error_rule>, Instruction_array_error_rule>,
      pegtl::seq< pegtl::at<Instruction_call_rule>, Instruction_call_rule>,
      pegtl::seq< pegtl::at<Instruction_increment_rule>, Instruction_increment_rule>,
      pegtl::seq< pegtl::at<Instruction_decrement_rule>, Instruction_decrement_rule>,
      pegtl::seq< pegtl::at<Instruction_email_rule>, Instruction_email_rule>
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

  template<> struct action < str_return > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_ret();
      i->arguments = currentF->arguments;
      i->locals = currentF->locals;
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Label_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      Item i;
      i.labelName = in.string();
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < M_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      if (std::stoll(in.string()) % 8 != 0){
        abort();
      }
    }
  };

  template<> struct action < Instruction_s_move_w_rule > {
    template< typename Input >
    static void apply ( const Input & in, Program & p) {
      auto currentF = p.functions.back();
      std::vector<std::string> tokens = split(addSpaces(in.string(), "<-"));
      auto i = new Instruction_mov();
      if (isLabel(tokens[2])){
        i->source = convertLabel(tokens[2]);
      }
      else{
        i->source = convertRegister(tokens[2]);
      }
      i->destination = convertRegister(tokens[0]);
      currentF->instructions.push_back(i);
    }
  };


  template<> struct action < Instruction_mem_move_w_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      auto currentF = p.functions.back();
      std::vector<std::string> tokens = split(addSpaces(in.string(), "<-"));
      auto i = new Instruction_mov();
      i->source = convertMem(convertRegister(tokens[3]), tokens[4]);
      i->destination = convertRegister(tokens[0]);
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_s_move_mem_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      std::vector<std::string> tokens = split(addSpaces(in.string(), "<-"));
      auto i = new Instruction_mov();
      if (isLabel(tokens[4])){
        i->source = convertLabel(tokens[4]);
      }
      else {
        i->source = convertRegister(tokens[4]);
      }
      i->destination = convertMem(convertRegister(tokens[1]), tokens[2]);
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_aop_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      if (in.string().find("+=") != std::string::npos){
        std::vector<std::string> tokens = split(addSpaces(in.string(), "+="));
        auto i = new Instruction_add();
        i->source = convertRegister(tokens[2]);
        i->destination = convertRegister(tokens[0]);
        currentF->instructions.push_back(i);
      }
      else if (in.string().find("-=") != std::string::npos){
        std::vector<std::string> tokens = split(addSpaces(in.string(), "-="));
        auto i = new Instruction_sub();
        i->source = convertRegister(tokens[2]);
        i->destination = convertRegister(tokens[0]);
        currentF->instructions.push_back(i);
      }
      else if (in.string().find("*=") != std::string::npos){
        std::vector<std::string> tokens = split(addSpaces(in.string(), "*="));
        auto i = new Instruction_mult();
        i->source = convertRegister(tokens[2]);
        i->destination = convertRegister(tokens[0]);
        currentF->instructions.push_back(i);
      }
      else{
        std::vector<std::string> tokens = split(addSpaces(in.string(), "&="));
        auto i = new Instruction_and();
        i->source = convertRegister(tokens[2]);
        i->destination = convertRegister(tokens[0]);
        currentF->instructions.push_back(i);
      }
      
    }
  };

  template<> struct action < Instruction_sop_sx_rule > {
    template< typename Input>
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      std::vector<std::string> tokens = split(in.string());
      if (in.string().find("<<=") != std::string::npos){
        std::vector<std::string> tokens = split(addSpaces(in.string(), "<<="));
        auto i = new Instruction_left_shift();
        if (isRegister(tokens[2])){
          i->k = "%" + convert8Bit(tokens[2]);
        }
        else{
          i->k = convertRegister(tokens[2]);
         }
        i->destination = convertRegister(tokens[0]);
        currentF->instructions.push_back(i);
      }
      else{
        std::vector<std::string> tokens = split(addSpaces(in.string(), ">>="));
        auto i = new Instruction_right_shift();
        if (isRegister(tokens[2])){
          i->k = "%" + convert8Bit(tokens[2]);
        }
        else{
          i->k = convertRegister(tokens[2]);
        }
        i->destination = convertRegister(tokens[0]);
        currentF->instructions.push_back(i);
      }
      
    }
  };

  template<> struct action < Instruction_sop_n_rule > {
    template < typename Input>
    static void apply( const Input & in, Program & p) {
      auto currentF = p.functions.back();
      std::vector<std::string> tokens = split(in.string());
      if (in.string().find("<<=") != std::string::npos){
        std::vector<std::string> tokens = split(addSpaces(in.string(), "<<="));
        auto i = new Instruction_left_shift();
        i->k = "$" + tokens[2];
        i->destination = convertRegister(tokens[0]);
        currentF->instructions.push_back(i);
      }
      else{
        std::vector<std::string> tokens = split(addSpaces(in.string(), ">>="));
        auto i = new Instruction_right_shift();
        i->k = "$" + tokens[2];
        i->destination = convertRegister(tokens[0]);
        currentF->instructions.push_back(i);
      }
      
    }
  };

  template<> struct action < Instruction_mem_pluseq_rule > {
    template < typename Input>
    static void apply( const Input & in, Program & p) {
      auto currentF = p.functions.back();
      std::vector<std::string> tokens = split(addSpaces(in.string(), "+="));
      auto i = new Instruction_add();
      i->source = convertRegister(tokens[4]);
      i->destination = convertMem(convertRegister(tokens[1]), tokens[2]);
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_mem_mineq_rule > {
    template < typename Input>
    static void apply( const Input & in, Program & p) {
      auto currentF = p.functions.back();
      std::vector<std::string> tokens = split(addSpaces(in.string(), "-="));
      auto i = new Instruction_sub();
      i->source = convertRegister(tokens[4]);
      i->destination = convertMem(convertRegister(tokens[1]), tokens[2]);
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_w_pluseq_rule > {
    template < typename Input>
    static void apply(const Input & in, Program & p) {
      auto currentF = p.functions.back();
      std::vector<std::string> tokens = split(addSpaces(in.string(), "+="));
      auto i = new Instruction_add();
      i->source = convertMem(convertRegister(tokens[3]), tokens[4]);
      i->destination = convertRegister(tokens[0]);
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_w_mineq_rule > {
    template < typename Input>
    static void apply(const Input & in, Program & p) {
      auto currentF = p.functions.back();
      std::vector<std::string> tokens = split(addSpaces(in.string(), "-="));
      auto i = new Instruction_sub();
      i->source = convertMem(convertRegister(tokens[3]), tokens[4]);
      i->destination = convertRegister(tokens[0]);
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_mov_cmp_rule > {
    template < typename Input>
    static void apply(const Input & in, Program & p) {
      auto currentF = p.functions.back();
      auto i = new Instruction_cmp_code();
      std::vector<std::string> tokens = split(in.string());
      i->destination = convertRegister(tokens[0]);
      i->inbetween = "%" + convert8Bit(tokens[0]);
      i->cmp = tokens[3];
      if (isRegister(tokens[2])){
        i->t1 = convertRegister(tokens[2]);
        i->t2 = convertRegister(tokens[4]);
        i->isFolded = false;
        i->isSwapped = false;
      }
      else if (!isRegister(tokens[2]) && isRegister(tokens[4])){
          i->t1 = convertRegister(tokens[4]);
          i->t2 = convertRegister(tokens[2]);
          i->isFolded = false;
          i->isSwapped = true;
      }
      else{
          if (i->cmp == "="){
              i->t1 = "$" + std::to_string(std::stoll(tokens[2]) == std::stoll(tokens[4]));
          }
          else if (i->cmp == "<="){
              i->t1 = "$" + std::to_string(std::stoll(tokens[2]) <= std::stoll(tokens[4]));
          }
          else{
              i->t1 = "$" + std::to_string(std::stoll(tokens[2]) < std::stoll(tokens[4]));
          }
          i->isFolded = true;
          i->isSwapped = false;
      }
      
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_cjump_two_rule > {
    template < typename Input >
    static void apply(const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_cjump_two();
      std::vector<std::string> tokens = split(in.string());
      i->cmp = tokens[2];
      if (isRegister(tokens[1])){
        i->t1 = convertRegister(tokens[1]);
        i->t2 = convertRegister(tokens[3]);
        i->isFolded = false;
        i->isSwapped = false;
      }
      else if (!isRegister(tokens[1]) && isRegister(tokens[3])){
          i->t1 = convertRegister(tokens[3]);
          i->t2 = convertRegister(tokens[1]);
          i->isFolded = false;
          i->isSwapped = true;
      }
      else{
          if (i->cmp == "="){
              i->t1 = std::to_string(std::stoll(tokens[1]) == std::stoll(tokens[3]));
          }
          else if (i->cmp == "<="){
              i->t1 = std::to_string(std::stoll(tokens[1]) <= std::stoll(tokens[3]));
          }
          else{
              i->t1 = std::to_string(std::stoll(tokens[1]) < std::stoll(tokens[3]));
          }
          i->isFolded = true;
          i->isSwapped = false;
      }
      i->l1 = convertRegister(tokens[4]);
      i->l2 = convertRegister(tokens[5]);
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_cjump_one_rule > {
    template < typename Input >
    static void apply(const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_cjump_one();
      std::vector<std::string> tokens = split(in.string());
      i->cmp = tokens[2];
      if (isRegister(tokens[1])){
        i->t1 = convertRegister(tokens[1]);
        i->t2 = convertRegister(tokens[3]);
        i->isFolded = false;
        i->isSwapped = false;
      }
      else if (!isRegister(tokens[3]) && isRegister(tokens[1])){
          i->t1 = convertRegister(tokens[3]);
          i->t2 = convertRegister(tokens[1]);
          i->isFolded = false;
          i->isSwapped = true;
      }
      else{
          if (i->cmp == "="){
              i->t1 = std::to_string(std::stoll(tokens[1]) == std::stoll(tokens[3]));
          }
          else if (i->cmp == "<="){
              i->t1 = std::to_string(std::stoll(tokens[1]) <= std::stoll(tokens[3]));
          }
          else{
              i->t1 = std::to_string(std::stoll(tokens[1]) < std::stoll(tokens[3]));
          }
          i->isFolded = true;
          i->isSwapped = false;
      }
      i->l1 = convertRegister(tokens[4]);
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_goto_rule > {
    template < typename Input >
    static void apply(const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_jump();
      std::vector<std::string> tokens = split(in.string());
      i->label = convertRegister(tokens[1]);
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_call_rule > {
    template < typename Input >
    static void apply(const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_call();
      std::vector<std::string> tokens = split(in.string());
      i->label = convertRegister(tokens[1]);
      i->arguments = std::stoll(tokens[2]);
      currentF->instructions.push_back(i);
    }
  };

template<> struct action < Instruction_call_print_rule > {
    template < typename Input >
    static void apply(const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_call_runtime();
      std::vector<std::string> tokens = split(in.string());
      i->label = "print";
      currentF->instructions.push_back(i);
    }
  };

template<> struct action < Instruction_call_allocate_rule > {
    template < typename Input >
    static void apply(const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_call_runtime();
      std::vector<std::string> tokens = split(in.string());
      i->label = "allocate";
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_array_error_rule > {
    template < typename Input >
    static void apply(const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_call_runtime();
      std::vector<std::string> tokens = split(in.string());
      i->label = "array_error";
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_increment_rule > {
    template < typename Input >
    static void apply(const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_inc();
      std::vector<std::string> tokens = split(in.string());
      if (tokens.size() == 1){
        i->destination = convertRegister(tokens[0].substr(0, tokens[0].length() - 2));
      }
      else{
        i->destination = convertRegister(tokens[0]);
      }
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_decrement_rule > {
    template < typename Input >
    static void apply(const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_dec();
      std::vector<std::string> tokens = split(in.string());
      if (tokens.size() == 1){
        i->destination = convertRegister(tokens[0].substr(0, tokens[0].length() - 2));
      }
      else{
        i->destination = convertRegister(tokens[0]);
      }
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_email_rule > {
    template < typename Input >
    static void apply(const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_email();
      std::vector<std::string> tokens = split(in.string());
      i->w1 = convertRegister(tokens[0]);
      i->w2 = convertRegister(tokens[2]);
      i->w3 = convertRegister(tokens[3]);
      i->e1 = tokens[4];
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_label_rule > {
      template < typename Input >
      static void apply(const Input & in, Program & p){
          auto currentF = p.functions.back();
          auto i = new Instruction_label();
          i->label = convertJumpLabel(in.string());
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
}
