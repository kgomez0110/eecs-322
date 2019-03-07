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
  

  struct str_return : TAOCPP_PEGTL_STRING( "return" ) {};
  struct print : TAOCPP_PEGTL_STRING("print") {};
  struct allocate : TAOCPP_PEGTL_STRING("allocate") {};
  struct array_error : TAOCPP_PEGTL_STRING("array-error") {};
  struct call : TAOCPP_PEGTL_STRING("call") {};
  struct br : TAOCPP_PEGTL_STRING("br") {};
  struct define : TAOCPP_PEGTL_STRING("define") {};
  struct load : TAOCPP_PEGTL_STRING("load") {}; 
  struct store : TAOCPP_PEGTL_STRING("store") {};

  struct plus:
    pegtl::one<'+'> {};

  struct minus:
    pegtl::one<'-'> {};
  
  struct times:
    pegtl::one<'*'> {};
  
  struct and_op:
    pegtl::one<'&'> {};
  
  struct move:
    pegtl::seq<
      pegtl::one<'<'>,
      pegtl::one<'-'>
    > {};

  struct left_shift:
    pegtl::seq<
      pegtl::one<'<'>,
      pegtl::one<'<'>
    > {};
  
  struct right_shift:
    pegtl::seq<
      pegtl::one<'>'>,
      pegtl::one<'>'>
    > {};

  struct less:
    pegtl::one<'<'> {};

  struct less_equal:
    pegtl::seq<
      pegtl::one<'<'>,
      pegtl::one<'='>
    > {};

  struct greater:
    pegtl::one<'>'> {};
  
  struct greater_equal:
    pegtl::seq<
      pegtl::one<'>'>,
      pegtl::one<'='>
    > {};

  struct equal:
    pegtl::one<'='> {};

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
      less,
      greater,
      equal,
      greater_equal,
      less_equal
    > {};

  struct u_rule:
    pegtl::sor<
      var,
      label
    > {};
  
  struct t_rule:
    pegtl::sor<
      var,
      number
    > {};

  struct s_rule:
    pegtl::sor<
      t_rule,
      label
    > {};

  struct callee_rule:
    pegtl::sor<
      u_rule,
      print,
      allocate,
      array_error
    > {};

  struct vars:
    pegtl::sor<
      var,
      seps,
      pegtl::seq<
        var,
        seps,
        pegtl::plus< pegtl::one<','>, seps, var, seps>
      >
    > {};

  struct args:
    pegtl::sor<
      t_rule,
      seps,
      pegtl::seq<
        t_rule,
        seps,
        pegtl::plus< pegtl::one<','>, seps, t_rule, seps>
      >
    > {};
  
  struct Instruction_move_rule :
    pegtl::seq<
      var,
      seps,
      move,
      seps,
      s_rule
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
      s_rule
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
      label 
    >{};
  
  struct Instruction_br_rule :
    pegtl::seq<
      br,
      seps,
      label
    > {};

  struct Instruction_br_var_rule :
    pegtl::seq<
      br,
      seps,
      var,
      seps,
      label
    > {};

  struct Instruction_call_rule:
    pegtl::seq<
      call,
      seps,
      callee_rule,
      seps,
      pegtl::one<'('>,
      seps,
      args,
      seps,
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
      seps,
      args,
      seps,
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
      seps,
      Function_vars,
      seps,
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

  struct grammar : 
    pegtl::must< 
      Functions_rule
    > {};
  
  std::vector<Item> items;
  int line = 0;


   template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  template<> struct action < Function_name> {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto newF = new Function();
      newF->name = in.string();
      p.functions.push_back(newF);
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
      i.isVar = true;
      i.value = in.string();
      if (currentF->variables.count(i.value)){
        i.var = currentF->variables[i.value];
      }
      else {
        auto v = new Variable();
        v->name = i.value;
        currentF->variables[i.value] = v;
        i.var = v;
      }
      items.push_back(i);
    }
  };

  template<> struct action < label > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      Item i;
      i.isVar = false;
      i.value = in.string() + "_" + p.functions.back()->name.substr(1);
      items.push_back(i);
    }
  };

  template<> struct action < number > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        Item i;
        i.isVar = false;
        i.value = in.string();
        items.push_back(i);
      }
  };

  template<> struct action < op_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        Item i;
        i.isVar = false;
        i.value = in.string();
        items.push_back(i);
      }
  };

  template<> struct action < callee_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        Item i;
        i.isVar = false;
        i.value = in.string();
        items.push_back(i);
      }
  };

  // return t
  template<> struct action < Instruction_return_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto instruct = new Instruction_Return_T(items[0], line);
      if (items[0].isVar){
        instruct->gen.insert(items[0].var);
        items[0].var->instructions.push_back(instruct);
      }
      currentF->instructions.push_back(instruct);
      items = {};
      line++;
    }
  };

 // return
 template<> struct action < Instruction_just_return_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){ 
      auto currentF = p.functions.back();
      auto instruct = new Instruction_Return(line);
      currentF->instructions.push_back(instruct);
      line++;
    }
  };

  // var <- call callee (args)
  template<> struct action < Instruction_var_call_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      std::vector<Item>::iterator it = items.begin() + 2;
      std::vector<Item> arguments(it, items.end());
      auto instruct = new Instruction_var_call(items[0], items[1], arguments, line);
      instruct->kill.insert(items[0].var);
      items[0].var->instructions.push_back(instruct);
      for (Item ii : arguments){
        if (ii.isVar){
          instruct->gen.insert(ii.var);
          ii.var->instructions.push_back(instruct);
        }
      }
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
      std::vector<Item>::iterator it = items.begin() + 1;
      std::vector<Item> arguments(it, items.end());
      auto instruct = new Instruction_call(items[0], arguments, line);
      for (Item ii : arguments){
        if (ii.isVar){
          instruct->gen.insert(ii.var);
          ii.var->instructions.push_back(instruct);
        }
      }
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
      auto instruct = new Instruction_br_var(items[0], items[1], line);
      instruct->gen.insert(items[0].var);
      items[0].var->instructions.push_back(instruct);
      currentF->instructions.push_back(instruct);
      line++;
      items = {};
    }
  };

  // br label
  template<> struct action < Instruction_br_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto instruct = new Instruction_br(items[0], line);
      currentF->instructions.push_back(instruct);
      line++;
      items = {};
    }
  };

  // var <- load var
  template<> struct action < Instruction_load_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto instruct = new Instruction_load(items[0], items[1], line);
      instruct->kill.insert(items[0].var);
      instruct->gen.insert(items[1].var);
      items[0].var->instructions.push_back(instruct);
      items[1].var->instructions.push_back(instruct);
      currentF->instructions.push_back(instruct);
      line++;
      items = {};
    }
  };

  // store var <- s
  template<> struct action < Instruction_store_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto instruct = new Instruction_store(items[0], items[1], line);
      instruct->gen.insert(items[0].var);
      items[0].var->instructions.push_back(instruct);
      if (items[1].isVar){
        instruct->gen.insert(items[1].var);
        items[1].var->instructions.push_back(instruct);
      }
      currentF->instructions.push_back(instruct);
      line++;
      items = {};
    }
  };

  // var <- t op t
  template<> struct action < Instruction_three_op_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto instruct = new Instruction_op(items[0], items[1], items[2], items[3], line);
      instruct->kill.insert(items[0].var);
      items[0].var->instructions.push_back(instruct);
      if (items[1].isVar){
        instruct->gen.insert(items[1].var);
        items[1].var->instructions.push_back(instruct);
      }
      if (items[3].isVar){
        instruct->gen.insert(items[3].var);
        items[3].var->instructions.push_back(instruct);
      }
      currentF->instructions.push_back(instruct);
      line++;
      items = {};
    }
  };

  // var <- t cmp t
  template<> struct action < Instruction_cmp_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto instruct = new Instruction_cmp(items[0], items[1], items[2], items[3], line);
      instruct->kill.insert(items[0].var);
      items[0].var->instructions.push_back(instruct);
      if (items[1].isVar){
        instruct->gen.insert(items[1].var);
        items[1].var->instructions.push_back(instruct);
      }
      if (items[3].isVar){
        instruct->gen.insert(items[3].var);
        items[3].var->instructions.push_back(instruct);
      }
      currentF->instructions.push_back(instruct);
      line++;
      items = {};
    }
  };

  // var <- s
  template<> struct action < Instruction_move_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto instruct = new Instruction_move(items[0], items[1], line);
      instruct->kill.insert(items[0].var);
      items[0].var->instructions.push_back(instruct);
      if (items[1].isVar){
        instruct->gen.insert(items[1].var);
        items[1].var->instructions.push_back(instruct);
      }
      currentF->instructions.push_back(instruct);
      line++;
      items = {};
    }
  };

  // label 
  template<> struct action < Instruction_label_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      Instruction_label* instruct = new Instruction_label(items[0], line);
      currentF->instructions.push_back(instruct);
      line++;
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