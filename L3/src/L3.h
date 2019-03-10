#pragma once

#include <vector>
#include <set>
#include <unordered_map>
#include <string>

namespace L3 {

  const std::vector<std::string> arguments = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

  struct Instruction;
  struct Function;
  struct Program;
  struct Item;

  struct Item {
    std::string value;
  };

  struct Function {
    std::string name;
    std::vector<Instruction*> instructions;
    std::vector<Item> arguments;
  };

  struct Program {
    std::vector<Function*> functions;
  };

  struct Instruction {
    virtual std::string toL2() = 0;
  };

  // return t
  struct Instruction_return : Instruction {
    Item t;

    /*
      rax <- t
      return
    */
    std::string toL2() {
      std::string result = "";
      result.append("rax <- " + t.value + "\n");
      result.append("return");
      return result;
    }

    Instruction_return(Item val) : t(val) {}
  };

  // return 
  struct Instruction_just_return : Instruction {
    std::string toL2() {
      return "return";
    }

    Instruction_just_return() {}
  };

  // var <- call callee (args)
  struct Instruction_var_call : Instruction {
    Item var, callee;
    std::vector<Item> args;
    int count;

    std::string toL2() {
      std::string result = "";
      std::string ret_label = "";
      if (callee.value != "print" && callee.value != "allocate" && callee.value != "array-error"){
        if (callee.value[0] == '%')
          ret_label = ":" + callee.value.substr(1) + "_ret" + std::to_string(count);
        else 
          ret_label = callee.value + "_ret" + std::to_string(count);
        result.append("mem rsp -8 <- " + ret_label + "\n");
      }
      for (int ii = 0; ii < this->args.size(); ii++) {
        if (ii < 6) {
          result.append(arguments[ii] + " <- " + args[ii].value + "\n");
        }
        else {
          int stack_offset = ((args.size() - 6) + (ii-6)) * 8;
          result.append("mem rsp -" + std::to_string(stack_offset) + " <- " + args[ii].value + "\n");
        }
      }
      result.append("call " + callee.value + " " + std::to_string(args.size()) + "\n");
      if (callee.value != "print" && callee.value != "allocate" && callee.value != "array-error"){
        result.append(ret_label + "\n");
      }
      result.append(var.value + " <- rax");
      return result;
    }

    Instruction_var_call(std::vector<Item> items, int c) {
      this->var = items[0];
      this->callee = items[1];
      this->count = c;
      for (int ii = 2; ii<items.size(); ii++){
        this->args.push_back(items[ii]);
      }
    }
  };

  // call callee (args)
  struct Instruction_call : Instruction {
    Item callee;
    std::vector<Item> args;
    int count;

    std::string toL2(){
      std::string result = "";
      std::string ret_label = "";
      if (callee.value != "print" && callee.value != "allocate" && callee.value != "array-error"){
        if (callee.value[0] == '%')
          ret_label = ":" + callee.value.substr(1) + "_ret" + std::to_string(count);
        else 
          ret_label = callee.value + "_ret" + std::to_string(count);
        result.append("mem rsp -8 <- " + ret_label + "\n");
      }
      for (int ii = 0; ii < this->args.size(); ii++) {
        if (ii < 6) {
          result.append(arguments[ii] + " <- " + args[ii].value + "\n");
        }
        else {
          int stack_offset = ((args.size() - 5) - (ii-6)) * 8;
          result.append("mem rsp -" + std::to_string(stack_offset) + " <- " + args[ii].value + "\n");
        }
      }
      result.append("call " + callee.value + " " + std::to_string(args.size()) + "\n");
      if (callee.value != "print" && callee.value != "allocate" && callee.value != "array-error"){
        result.append("\n" + ret_label);
      }
      return result;
    }

    Instruction_call(std::vector<Item> items, int c) {
      this->callee = items[0];
      this->count = c;
      for (int ii = 1; ii<items.size(); ii++){
        this->args.push_back(items[ii]);
      }
    }
  };

  // br var label
  struct Instruction_br_var : Instruction {
    Item var, label;

    std::string toL2() {
      return "cjump " + var.value + " = 1 " + label.value;
    }

    Instruction_br_var(Item v, Item l) : var(v), label(l) {}
  };

  // br label
  struct Instruction_br : Instruction {
    Item label;
    
    std::string toL2() {
      return "goto " + label.value;
    }

    Instruction_br(Item l) : label(l) {}
  };

  // var <- load var
  struct Instruction_load : Instruction {
    Item var1, var2;

    std::string toL2() {
      return var1.value + " <- mem " + var2.value + " 0";
    }

    Instruction_load(Item v1, Item v2) : var1(v1), var2(v2) {}
  };

  // store var <- s
  struct Instruction_store : Instruction {
    Item var, s;

    std::string toL2() {
      return "mem " + var.value + " 0 <- " + s.value;
    }

    Instruction_store(Item v, Item s1) : var(v), s(s1)  {}
  };

  // var <- t op t
  struct Instruction_three_op : Instruction {
    Item var, t1, op, t2;

    std::string toL2() {
      std::string result = "";
      if (var.value == t1.value) {
        result.append(var.value + " " + op.value + "= " + t2.value);
        return result;
      }
      if (var.value == t2.value) {
        result.append(var.value + " " + op.value + "= " + t1.value);
        return result;
      }
      result.append(var.value + " <- " + t1.value + "\n");
      result.append(var.value + " " + op.value + "= " + t2.value);
      return result;
    }

    Instruction_three_op(Item v, Item t_1, Item o, Item t_2) : var(v), t1(t_1), t2(t_2), op(o) {}

  };

  // var <- t cmp t
  struct Instruction_cmp : Instruction {
    Item var, t1, cmp, t2;

    std::string toL2() {
      if (cmp.value == "=" || cmp.value == "<" || cmp.value == "<=")
        return var.value + " <- " + t1.value + " " + cmp.value + " " + t2.value;
      if (cmp.value == ">")
        return var.value + " <- " + t2.value + " < " + t1.value;
      if (cmp.value == ">=")
        return var.value + " <- " + t2.value + " <= " + t1.value;
    }

    Instruction_cmp(Item v, Item t_1, Item o, Item t_2) : var(v), t1(t_1), t2(t_2), cmp(o) {}
  };

  // var <- s
  struct Instruction_move : Instruction {
    Item var, s;

    std::string toL2() {
      return var.value + " <- " + s.value; 
    }

    Instruction_move(Item v, Item s1) : var(v), s(s1) {};
  };

  // label
  struct Instruction_label : Instruction {
    Item label;

    std::string toL2() {
      return label.value;
    }

    Instruction_label(Item l) : label(l) {}
  };









  





  
}