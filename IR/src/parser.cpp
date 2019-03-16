#include <string>
#include <vector>
#include <set>
#include <unordered_map>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include <IR.h>


namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;


namespace IR {

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

  

  struct str_return : TAOCPP_PEGTL_STRING( "return" ) {};
  struct print : TAOCPP_PEGTL_STRING("print") {};
  struct void_key : TAOCPP_PEGTL_STRING("void") {};
  struct len : TAOCPP_PEGTL_STRING("length") {};
  struct array_error : TAOCPP_PEGTL_STRING("array-error") {};
  struct call : TAOCPP_PEGTL_STRING("call") {};
  struct br : TAOCPP_PEGTL_STRING("br") {};
  struct define : TAOCPP_PEGTL_STRING("define") {};
  struct add : TAOCPP_PEGTL_STRING("+") {};
  struct sub : TAOCPP_PEGTL_STRING("-") {};
  struct mult : TAOCPP_PEGTL_STRING("*") {};
  struct and_op : TAOCPP_PEGTL_STRING("&") {};
  struct left_shift : TAOCPP_PEGTL_STRING("<<") {};
  struct right_shift : TAOCPP_PEGTL_STRING(">>") {};
  struct less : TAOCPP_PEGTL_STRING("<") {};
  struct less_eq : TAOCPP_PEGTL_STRING("<=") {};
  struct equal : TAOCPP_PEGTL_STRING("=") {};
  struct greater : TAOCPP_PEGTL_STRING(">") {};
  struct greater_eq : TAOCPP_PEGTL_STRING(">=") {};
  struct new_key : TAOCPP_PEGTL_STRING("new") {};
  struct array : TAOCPP_PEGTL_STRING("Array") {};
  struct tuple : TAOCPP_PEGTL_STRING("Tuple") {};
  struct type_tuple : TAOCPP_PEGTL_STRING("tuple") {};
  struct int_type : TAOCPP_PEGTL_STRING("int64") {};
  struct code : TAOCPP_PEGTL_STRING("code") {};
  struct move : TAOCPP_PEGTL_STRING("<-") {};

  struct op_rule :
    pegtl::sor<
      add,
      sub,
      mult,
      and_op,
      left_shift,
      right_shift,
      less_eq,
      greater_eq,
      equal,
      less,
      greater
    > {};
  struct Label_rule : 
    label {};
  
  struct Number_rule :
    number {}; 

  struct var :
    pegtl::seq<
      pegtl::one<'%'>,
      name
    > {};

  struct var_copy :
    pegtl::seq<
      pegtl::one<'%'>,
      name
    > {};

  
  
  struct array_declare_rule :
    pegtl::plus<
      pegtl::seq<
        pegtl::one<'['>,
        seps,
        pegtl::one<']'>
      >
    >{};

  struct type_rule :
    pegtl::sor<
      int_type,
      type_tuple,
      code
    > {};

  struct full_type_rule :
    pegtl::sor<
      pegtl::seq<
        type_rule,
        seps,
        array_declare_rule
      >,
      type_rule
    > {};

  struct big_type_rule :
    pegtl::sor<
      full_type_rule,
      void_key
    > {};

  struct u_rule :
    pegtl::sor <
      var,
      Label_rule
    > {};

  struct u_rule_copy :
    pegtl::sor <
      var_copy,
      label
    > {};

  struct t_rule :
    pegtl::sor <
      var,
      Number_rule
    > {};

  struct s_rule :
    pegtl::sor<
      t_rule,
      Label_rule
    > {};

  struct callee_rule :
    pegtl::sor<
      u_rule_copy,
      print,
      array_error
    > {};

  struct vars_rule :
      pegtl::seq<
        var,
        seps,
        pegtl::opt<pegtl::plus<pegtl::one<','>, seps, var, seps>>
      > {};

  struct args_rule :
    pegtl::seq<
      t_rule,
      seps,
      pegtl::opt<pegtl::plus<pegtl::one<','>, seps, t_rule, seps>>
    > {};

  struct array_access_rule :
    pegtl::seq<
      var,
      seps,
      pegtl::plus<
        pegtl::seq<
          pegtl::one<'['>, 
          seps, 
          t_rule, 
          seps, 
          pegtl::one<']'>, 
          seps
        >
      >
    > {};
  struct Instruction_return_rule :
    str_return {};
  
  struct Instruction_return_t_rule :
    pegtl::seq<
      str_return,
      seps,
      t_rule
    > {};
  
  struct Instruction_br_rule :
    pegtl::seq<
      br,
      seps,
      Label_rule
    > {};

  struct Instruction_br_t_rule :
    pegtl::seq<
      br,
      seps,
      t_rule,
      seps,
      Label_rule,
      seps,
      Label_rule
    > {};
  
  struct Instruction_declare_var_rule :
    pegtl::seq<
      big_type_rule,
      seps,
      var
    > {};

  struct Instruction_move_rule : 
    pegtl::seq<
      var,
      seps,
      move,
      seps,
      s_rule
    > {};
  
  struct Instruction_op_rule :
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

  struct Instruction_load_array_rule :
    pegtl::seq<
      var,
      seps,
      move,
      seps,
      array_access_rule
    > {};
  
  struct Instruction_store_array_rule :
    pegtl::seq<
      array_access_rule,
      seps,
      move,
      seps,
      s_rule
    > {};

  struct Instruction_get_length_rule :
    pegtl::seq<
      var,
      seps,
      move,
      seps,
      len,
      seps,
      var,
      seps,
      t_rule
    > {};

  struct Instruction_call_rule :
    pegtl::seq<
      call,
      seps,
      callee_rule,
      seps,
      pegtl::one<'('>,
      pegtl::opt<args_rule>,
      pegtl::one<')'>
    > {};

  struct Instruction_save_call_rule :
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
      pegtl::opt<args_rule>,
      pegtl::one<')'>
    > {};

  struct Instruction_make_array_rule :
    pegtl::seq<
      var,
      seps,
      move,
      seps,
      new_key,
      seps,
      array,
      seps,
      pegtl::one<'('>,
      args_rule,
      pegtl::one<')'>
    > {};

  struct Instruction_make_tuple_rule :
    pegtl::seq<
      var,
      seps,
      move,
      seps,
      new_key,
      seps,
      tuple,
      seps,
      pegtl::one<'('>,
      seps,
      t_rule,
      seps,
      pegtl::one<')'>
    > {};

  struct Instruction_label_rule:
    pegtl::seq<
      Label_rule
    > {};

  struct Instruction_rule:
    pegtl::sor<
      pegtl::seq< pegtl::at<Instruction_return_t_rule>, Instruction_return_t_rule>,
      pegtl::seq< pegtl::at<Instruction_return_rule>, Instruction_return_rule>,
      pegtl::seq< pegtl::at<Instruction_br_t_rule>, Instruction_br_t_rule>,
      pegtl::seq< pegtl::at<Instruction_br_rule>, Instruction_br_rule>,
      pegtl::seq< pegtl::at<Instruction_get_length_rule>, Instruction_get_length_rule>,
      pegtl::seq< pegtl::at<Instruction_save_call_rule>, Instruction_save_call_rule>,
      pegtl::seq< pegtl::at<Instruction_call_rule>, Instruction_call_rule>,
      pegtl::seq< pegtl::at<Instruction_load_array_rule>, Instruction_load_array_rule>,
      pegtl::seq< pegtl::at<Instruction_store_array_rule>, Instruction_store_array_rule>,
      pegtl::seq< pegtl::at<Instruction_op_rule>, Instruction_op_rule>,
      pegtl::seq< pegtl::at<Instruction_make_array_rule>, Instruction_make_array_rule>,
      pegtl::seq< pegtl::at<Instruction_make_tuple_rule>, Instruction_make_tuple_rule>,
      pegtl::seq< pegtl::at<Instruction_move_rule>, Instruction_move_rule>,
      pegtl::seq< pegtl::at<Instruction_declare_var_rule>, Instruction_declare_var_rule>,
      pegtl::seq< pegtl::at<Instruction_label_rule>, Instruction_label_rule>
    > {};

  struct Instructions_rule:
    pegtl::plus<
      seps,
      Instruction_rule,
      seps
    > {};

  struct type_var :
    pegtl::seq<
      seps,
      big_type_rule,
      seps,
      var
    > {};
  
  struct type_vars:
    pegtl::sor<
      pegtl::seq<
        type_var,
        seps,
        pegtl::opt<pegtl::plus<pegtl::one<','>, seps, type_var, seps>>
      >,
      seps
    > {};

  struct Function_vars:
    type_vars {};

  struct Function_name:
    label {};

  struct Function_type:
    big_type_rule {};

  struct Function_rule:
    pegtl::seq<
      define,
      seps,
      Function_type,
      seps,
      Function_name,
      seps,
      pegtl::one<'('>,
      pegtl::opt<Function_vars>,
      pegtl::one<')'>,
      seps,
      pegtl::one<'{'>,
      Instructions_rule,
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
  std::unordered_map<std::string, bool> isTuple;


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

  template<> struct action < Function_vars > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->arguments = items;
      for (int ii = 0; ii<items.size(); ii+=2) {
        if (items[ii].value == "tuple") {
          isTuple[items[ii+1].value] = true;
        }
        else {
          isTuple[items[ii+1].value] = false;
        }
      }
      items = {};
    }
  };

  template<> struct action < Function_type > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->type = in.string();
      items = {};
    }
  };

  template<> struct action < var > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      Item i;
      i.value = in.string();
      items.push_back(i);
    }
  };

  template<> struct action < Label_rule > {
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

  template<> struct action < big_type_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      Item i;
      i.value = in.string();
      items.push_back(i);
    }
  };

  // label
  template<> struct action < Instruction_label_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto inst = new Instruction_label(items[0]);
      currentF->instructions.push_back(inst);
      items = {};
    }
  }; 

  // return
  template<> struct action < Instruction_return_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto inst = new Instruction_return();
      currentF->instructions.push_back(inst);
      items = {};
    }
  };

  // return t
  template<> struct action < Instruction_return_t_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto inst = new Instruction_return_t(items[0]);
      currentF->instructions.push_back(inst);
      items = {};
    }
  };

  // br t label label
  template<> struct action < Instruction_br_t_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto inst = new Instruction_br_t(items[0], items[1], items[2]);
      currentF->instructions.push_back(inst);
      items = {};
    }
  };

  // br label
  template<> struct action < Instruction_br_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto inst = new Instruction_br(items[0]);
      currentF->instructions.push_back(inst);
      items = {};
    }
  };

  // var <- call callee (args)
  template<> struct action < Instruction_save_call_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto inst = new Instruction_save_call(items);
      currentF->instructions.push_back(inst);
      items = {};
    }
  };

  // call callee (args)
  template<> struct action < Instruction_call_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto inst = new Instruction_call(items);
      currentF->instructions.push_back(inst);
      items = {};
    }
  };

  // var <- t op t
  template<> struct action < Instruction_op_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto inst = new Instruction_op(items[0], items[1], items[2], items[3]);
      currentF->instructions.push_back(inst);
      items = {};
    }
  };

  // var <- s
  template<> struct action < Instruction_move_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto inst = new Instruction_move(items[0], items[1]);
      currentF->instructions.push_back(inst);
      items = {};
    }
  };

  // type var
  template<> struct action < Instruction_declare_var_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      // auto currentF = p.functions.back();
      // auto inst = new Instruction_move(items[0], items[1]);
      // currentF->instructions.push_back(inst);
      if (items[0].value == "tuple") {
        isTuple[items[1].value] = true;
      }
      else {
        isTuple[items[1].value] = false;
      }
      items = {};
    }
  };

  // var <- length var t
  template<> struct action < Instruction_get_length_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto inst = new Instruction_get_length(items[0], items[1], items[2]);
      currentF->instructions.push_back(inst);
      items = {};
    }
  };

  // var <- new Array(args)
  template<> struct action < Instruction_make_array_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto inst = new Instruction_make_array(items);
      currentF->instructions.push_back(inst);
      items = {};
    }
  };

  // var <- new Tuple(arg)
  template<> struct action < Instruction_make_tuple_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto inst = new Instruction_make_tuple(items);
      currentF->instructions.push_back(inst);
      items = {};
    }
  };

  // var <- var[t]*
  template<> struct action < Instruction_load_array_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      if (isTuple[items[1].value]){
        auto inst = new Instruction_load_tuple(items);
        currentF->instructions.push_back(inst);
      }
      else {
        auto inst = new Instruction_load_array(items);
        currentF->instructions.push_back(inst);
      }
      items = {};
    }
  };

  // var[t]* <- var
  template<> struct action < Instruction_store_array_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      if (isTuple[items[0].value]){
        auto inst = new Instruction_store_tuple(items);
        currentF->instructions.push_back(inst);
      }
      else {
        auto inst = new Instruction_store_array(items);
        currentF->instructions.push_back(inst);
      }
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