#pragma once

#include <vector>
#include <set>
#include <string>
#include <unordered_map>


namespace IR {
  struct Function;
  struct Instruction;
  struct Program;
  struct Item;
  struct Array;

  struct Program {
    std::vector<Function*> functions;
  };

  struct Function {
    std::string name;
    std::string type;
    std::vector<Instruction*> instructions;
    std::vector<Item> arguments;
    std::unordered_map<std::string, Array*> arrays;
  };

  struct Instruction {
    virtual std::string toL3() = 0;
  };

  struct Item {
    std::string value;
  };

  struct Array {
    std::vector<int> dimensions;
  //  std::string getIndex(){};
    Array(std::vector<Item> items) {
      for (Item ii : items){
        this->dimensions.push_back(std::stoi(ii.value));
      }
    }
  };



  // return
  struct Instruction_return : Instruction {

    std::string toL3() {
      return "return";
    }
  };


  // return t
  struct Instruction_return_t : Instruction {
    Item t;
    std::string toL3() {
      return "return " + t.value;
    }
    Instruction_return_t(Item i) : t(i) {}
  };

  // label
  struct Instruction_label : Instruction {
    Item label;
    std::string toL3() {
      return label.value;
    }

    Instruction_label(Item l) : label(l) {}
  };

  // br t label label
  struct Instruction_br_t : Instruction {
    Item t, label1, label2;

    std::string toL3() {
      return "br " + t.value + " " + label1.value + "\nbr " + label2.value;
    }

    Instruction_br_t(Item temp, Item l1, Item l2) : t(temp), label1(l1), label2(l2) {}
  };

  // br label
  struct Instruction_br : Instruction {
    Item label;
    std::string toL3() {
      return "br " + label.value;
    }

    Instruction_br(Item l) : label(l) {}
  };

  // var <- length var t
  struct Instruction_get_length : Instruction {
    Item var1, var2, t;

    std::string toL3() {
      std::string result = "";
      result.append("%tempvlksjdlkjdsf <- 8 * " + t.value + "\n");
      result.append("%tempvlksjdlkjdsf0 <- %tempvlksjdlkjdsf + 16\n");
      result.append("%tempvlksjdlkjdsf1 <- " + var2.value + "%tempvlksjdlkjdsf0\n");
      result.append(var1.value + " <- load %tempvlksjdlkjdsf1");
      return result;
    }

    Instruction_get_length(Item v1, Item v2, Item t1) : var1(v1), var2(v2), t(t1) {}
  };

  // var <- call callee (args)
  struct Instruction_save_call : Instruction {
    Item var, callee;
    std::vector<Item> args;

    std::string toL3() {
      std::string result = "";
      result.append(var.value + " <- call " + callee.value + "(");
      for (int ii = 0; ii<args.size(); ii++){
        if (ii == 0)
          result.append(args[ii].value);
        else
          result.append(", " + args[ii].value);
      }
      result.append(")");
      return result;
    }

    Instruction_save_call (std::vector<Item> a) {
      this->var = a[0];
      this->callee = a[1];
      for (int ii = 2; ii < a.size(); ii++){
        this->args.push_back(a[ii]);
      }
    }
  };

  // call callee (args)
  struct Instruction_call : Instruction {
    Item callee;
    std::vector<Item> args;
    std::string toL3() {
      std::string result = "";
      result.append("call " + callee.value + "(");
      for (int ii = 0; ii<args.size(); ii++){
        if (ii == 0)
          result.append(args[ii].value);
        else
          result.append(", " + args[ii].value);
      }
      result.append(")");
      return result;
    }

    Instruction_call (std::vector<Item> a) {
      this->callee = a[0];
      for (int ii = 1; ii < a.size(); ii++){
        this->args.push_back(a[ii]);
      }
    }
  };

  // var <- var[t]*
  struct Instruction_load_array : Instruction {

    std::string toL3() {
      return "";
    }
  };

  // var[t]* <- var
  struct Instruction_store_array : Instruction {

    std::string toL3() {
      return "";
    }
  };

  // var <- t op t
  struct Instruction_op : Instruction {
    Item var, t1, t2, op;
    std::string toL3() {
      return var.value + " <- " + t1.value + " " + op.value + " " + t2.value;
    }  

    Instruction_op(Item v, Item t, Item o, Item ot) : var(v), t1(t), t2(ot), op(o) {}
  };

  // var <- new Array(args)
  struct Instruction_make_array : Instruction {
    Item var;
    std::vector<Item> args;
    std::string makeVar(std::string prefix, int &count){
      count++;
      return prefix + std::to_string(count);
    }
    std::string toL3() {
      std::string arg_temp = "%dkljfkjafeifhew";
      int arg_counter = 0;
      int temp_counter = 0;
      std::string temp = "%dslfjsdjflsdjf";
      std::string result = "";
      std::string v0 = makeVar(temp, temp_counter)
      result.append(v0 + " <- " + args[0].value + " >> 1\n");
      for (int ii = 1; ii<args.size(); ii++){
        std::string pd = makeVar(arg_temp, arg_counter);
        result.append(pd + " <- " + args[ii].value + " >> 1\n");
        result.append(v0 + " <- " + v0 + " * " + pd + "\n");
      }
      result.append(v0 + " <- " + v0 + " << 1\n");
      result.append(v0 + " <- " + v0 + " 1\n");
  };

  // var <- new Tuple(t);
  struct Instruction_make_tuple : Instruction {

    std::string toL3() {
      return "";
    } 
  };

  // var <- s
  struct Instruction_move : Instruction {
    Item var, s;
    std::string toL3() {
      return var.value + " <- " + s.value;
    } 

    Instruction_move(Item v, Item s1) : var(v), s(s1) {}
  };

  // type var
  struct Instruction_declare_var : Instruction {

    std::string toL3() {
      return "";
    } 
  };
}