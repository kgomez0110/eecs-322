#include <string>
#include <vector>
#include <set>
#include <unordered_map>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include <L3.h>


namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;


namespace L3 {

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
  
  struct var:
    pegtl::seq<
      pegtl::one<'%'>,
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
    
  struct comment: 
    pegtl::disable< 
      TAOCPP_PEGTL_STRING( "//" ), 
      pegtl::until< pegtl::eolf > 
    > {};

  struct seps: 
    pegtl::star< 
      pegtl::sor< 
        pegtl::ascii::space, 
        comment 
      > 
    > {};

  struct Function_name:
    label { };
  
  struct Label_rule:
    label {};
  
  struct Number_rule :
    number {};


  struct str_return : TAOCPP_PEGTL_STRING( "return" ) {};
  struct print : TAOCPP_PEGTL_STRING("print") {};
  struct allocate : TAOCPP_PEGTL_STRING("allocate") {};
  struct array_error : TAOCPP_PEGTL_STRING("array-error") {};
  struct call : TAOCPP_PEGTL_STRING("call") {};
  struct br : TAOCPP_PEGTL_STRING("br") {};
  struct define : TAOCPP_PEGTL_STRING("define") {};
  struct load : TAOCPP_PEGTL_STRING("load") {}; 
  struct store : TAOCPP_PEGTL_STRING("store") {};
  struct plus : TAOCPP_PEGTL_STRING("+") {};
  struct minus : TAOCPP_PEGTL_STRING("-") {};
  struct times : TAOCPP_PEGTL_STRING("*") {};
  struct move : TAOCPP_PEGTL_STRING("<-") {};
  struct and_op : TAOCPP_PEGTL_STRING("&") {};
  struct left_shift : TAOCPP_PEGTL_STRING("<<") {};
  struct right_shift : TAOCPP_PEGTL_STRING(">>") {};
  struct less : TAOCPP_PEGTL_STRING("<") {};
  struct less_equal : TAOCPP_PEGTL_STRING("<=") {};
  struct equal : TAOCPP_PEGTL_STRING("=") {};
  struct greater : TAOCPP_PEGTL_STRING(">") {};
  struct greater_equal : TAOCPP_PEGTL_STRING(">=") {};

  struct op_rule:
    pegtl::sor<
      plus,
      minus,
      times,
      and_op,
      left_shift,
      right_shift
    > {};

  struct cmp_rule:
    pegtl::sor<
      less_equal,
      greater_equal,
      less,
      greater,
      equal
    > {};

  struct u_rule:
    pegtl::sor<
      var,
      Label_rule
    > {};
  
  struct function_call:
      pegtl::seq<
        pegtl::one<':'>,
        name
      > {};
  
  struct function_u_rule:
    pegtl::sor<
      var,
      function_call
    > {};

  struct t_rule:
    pegtl::sor<
      var,
      Number_rule
    > {};

  struct s_rule:
    pegtl::sor<
      t_rule,
      Label_rule
    > {};

  struct function_s_rule:
    pegtl::sor<
      t_rule,
      function_call
    > {};



  struct callee_rule:
    pegtl::sor<
      function_u_rule,
      print,
      allocate,
      array_error
    > {};

  struct s_with_callee:
    pegtl::sor<
      t_rule,
      callee_rule
    > {};
    
  struct vars :
    pegtl::sor<
      pegtl::seq<
        var,
        seps,
        pegtl::opt<pegtl::plus<pegtl::one<','>, seps, var, seps>>
      >,
      seps
    > {};

  struct args :
    pegtl::seq<
      t_rule,
      seps,
      pegtl::opt<pegtl::plus<pegtl::one<','>, seps, t_rule, seps>>
    > {};

  struct Instruction_move_rule :
    pegtl::seq<
      var,
      seps,
      move,
      seps,
      s_with_callee
    > {};

  struct Instruction_three_op_rule:
    pegtl::seq<
      var,
      seps,
      move,
      seps,
      t_rule,
      seps,
      op_rule,
      seps,
      t_rule
    > {};

  struct Instruction_cmp_rule:
    pegtl::seq<
      var,
      seps,
      move,
      seps,
      t_rule,
      seps,
      cmp_rule,
      seps,
      t_rule
    > {};  

  struct Instruction_load_rule:
    pegtl::seq<
      var,
      seps,
      move,
      seps,
      load,
      seps,
      var
    > {};

  struct Instruction_store_rule:
    pegtl::seq<
      store,
      seps,
      var,
      seps,
      move,
      seps,
      function_s_rule
    > {};

  struct Instruction_just_return_rule:
    str_return {};
  
  struct Instruction_return_rule:
    pegtl::seq<
      str_return,
      seps,
      t_rule
    > {};

  struct Instruction_label_rule:
    pegtl::seq<
      Label_rule 
    >{};
  
  struct Instruction_br_rule :
    pegtl::seq<
      br,
      seps,
      Label_rule
    > {};

  struct Instruction_br_var_rule :
    pegtl::seq<
      br,
      seps,
      var,
      seps,
      Label_rule
    > {};

  struct Instruction_call_rule:
    pegtl::seq<
      call,
      seps,
      callee_rule,
      seps,
      pegtl::one<'('>,
      pegtl::opt<args>,
      pegtl::one<')'>
    > {};
  
  struct Instruction_var_call_rule:
    pegtl::seq<
      var,
      seps,
      move,
      seps,
      call,
      seps,
      callee_rule,
      seps,
      pegtl::one<'('>,
      pegtl::opt<args>,
      pegtl::one<')'>
    > {};

  struct Instruction_rule:
    pegtl::sor<
      pegtl::seq< pegtl::at<Instruction_return_rule>, Instruction_return_rule>,
      pegtl::seq< pegtl::at<Instruction_just_return_rule>, Instruction_just_return_rule>,
      pegtl::seq< pegtl::at<Instruction_var_call_rule>, Instruction_var_call_rule>,
      pegtl::seq< pegtl::at<Instruction_call_rule>, Instruction_call_rule>,
      pegtl::seq< pegtl::at<Instruction_br_var_rule>, Instruction_br_var_rule>,
      pegtl::seq< pegtl::at<Instruction_br_rule>, Instruction_br_rule>,
      pegtl::seq< pegtl::at<Instruction_load_rule>, Instruction_load_rule>,
      pegtl::seq< pegtl::at<Instruction_store_rule>, Instruction_store_rule>,
      pegtl::seq< pegtl::at<Instruction_three_op_rule>, Instruction_three_op_rule>,
      pegtl::seq< pegtl::at<Instruction_cmp_rule>, Instruction_cmp_rule>,
      pegtl::seq< pegtl::at<Instruction_move_rule>, Instruction_move_rule>,
      pegtl::seq< pegtl::at<Instruction_label_rule>, Instruction_label_rule>
    > {};

  struct Instructions_rule:
    pegtl::plus<
      seps,
      Instruction_rule,
      seps
    > {};

  struct Function_vars:
    vars { };

  struct Function_rule:
    pegtl::seq<
      define,
      seps,
      Function_name,
      seps,
      pegtl::one<'('>,
      pegtl::opt<Function_vars>,
      pegtl::one<')'>,
      seps,
      pegtl::one<'{'>,
      seps,
      Instructions_rule,
      seps,
      pegtl::one<'}'>
    > {};

  struct Functions_rule:
    pegtl::plus<
      seps,
      Function_rule,
      seps
    > {};

  struct entry_point_rule:
    pegtl::seq<
      Functions_rule
    > {};

  struct grammar : 
    pegtl::must< 
      entry_point_rule
    > {};
  
  std::vector<Item> items;
  int line = 0;


   template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  template<> struct action < define > {
    template< typename Input > 
    static void apply( const Input & in, Program & p){
      auto newF = new Function();
      p.functions.push_back(newF);
    }
  };

  template<> struct action < Function_name > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->name = in.string();
      items = {};
    }
  };

  template<> struct action < Function_vars> {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->arguments = items;
      items = {};
    }
  };
    
  template<> struct action < var > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      Item i;
      i.value = in.string();
      items.push_back(i);
    }
  };

  template<> struct action < Label_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      Item i;
      i.value = in.string() + "_" + p.functions.back()->name.substr(1);
      items.push_back(i);
    }
  };

  template<> struct action < Number_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        Item i;
        i.value = in.string();
        items.push_back(i);
      }
  };

  template<> struct action < op_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        Item i;
        i.value = in.string();
        items.push_back(i);
      }
  };

  template<> struct action < callee_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        Item i;
        i.value = in.string();
        items.push_back(i);
      }
  };

  template<> struct action < cmp_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        Item i;
        i.value = in.string();
        items.push_back(i);
      }
  };

  // return t
  template<> struct action < Instruction_return_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto instruct = new Instruction_return(items[0]);
      currentF->instructions.push_back(instruct);
      items = {};
    }
  };

 // return
 template<> struct action < Instruction_just_return_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){ 
      auto currentF = p.functions.back();
      auto instruct = new Instruction_just_return();
      currentF->instructions.push_back(instruct);
    }
  };

  // var <- call callee (args)
  template<> struct action < Instruction_var_call_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto instruct = new Instruction_var_call(items, line);
      currentF->instructions.push_back(instruct);
      line++;
      items = {};
    }
  };

  // call callee (args)
  template<> struct action < Instruction_call_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto instruct = new Instruction_call(items, line);
      currentF->instructions.push_back(instruct);
      line++;
      items = {};
    }
  };

  // br var label
  template<> struct action < Instruction_br_var_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto instruct = new Instruction_br_var(items[0], items[1]);
      currentF->instructions.push_back(instruct);
      items = {};
    }
  };

  // br label
  template<> struct action < Instruction_br_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto instruct = new Instruction_br(items[0]);
      currentF->instructions.push_back(instruct);
      items = {};
    }
  };

  // var <- load var
  template<> struct action < Instruction_load_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto instruct = new Instruction_load(items[0], items[1]);
      currentF->instructions.push_back(instruct);
      items = {};
    }
  };

  // store var <- s
  template<> struct action < Instruction_store_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto instruct = new Instruction_store(items[0], items[1]);
      currentF->instructions.push_back(instruct);
      items = {};
    }
  };

  // var <- t op t
  template<> struct action < Instruction_three_op_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto instruct = new Instruction_three_op(items[0], items[1], items[2], items[3]);
      currentF->instructions.push_back(instruct);
      items = {};
    }
  };

  // var <- t cmp t
  template<> struct action < Instruction_cmp_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto instruct = new Instruction_cmp(items[0], items[1], items[2], items[3]);
      currentF->instructions.push_back(instruct);
      items = {};
    }
  };

  // var <- s
  template<> struct action < Instruction_move_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto instruct = new Instruction_move(items[0], items[1]);
      currentF->instructions.push_back(instruct);
      items = {};
    }
  };

  // label 
  template<> struct action < Instruction_label_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto instruct = new Instruction_label(items[0]);
      currentF->instructions.push_back(instruct);
      items = {};
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