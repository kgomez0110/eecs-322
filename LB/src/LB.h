#pragma once

#include <vector>
#include <string>
#include <unordered_map>


namespace LB {

  struct Function;

  struct Item {
    std::string value;
  };

  struct Scope {
    std::vector<Instruction*> instructions;
  };

  struct Instruction {
    virtual std::string toLA(int count, Function* f) = 0;
  };

  struct Function {
    std::string name;
    std::string type;
    std::vector<Item> arguments;
    std::unordered_map<Instruction*, std::string> begin_while;
    std::unordered_map<Instruction*, std::string> end_while;
    std::unordered_map<Instruction*, std::string> cond_labels;
    std::unordered_map<Instruction*, Instruction*> loop;
    std::vector<Instruction*> instructions;
    std::vector<Scope*> scopes;
  };

  struct Program {
    std::vector<Function*> functions;
  };

  // type names
  struct Instruction_declare : Instruction {
    Item type;
    std::vector<Item> names;

    Instruction_declare(std::vector<Item> items) {
      this->type = items[0];
      for (int ii = 1; ii<items.size(); ii++){
        this->names.push_back(items[ii]);
      }
    }
  };

  // name <- s
  struct Instruction_move : Instruction {
    Item name, s;

    std::string toLA(int count, Function* f) {
      return name.value + "_" + std::to_string(count) + " <- "
    }
    Instruction_move(std::vector<Item> items) {
      this->name = items[0];
      this->s = items[1];
    }
  };

  // name <- cond
  struct Instruction_cond : Instruction {
    Item name, t1, op, t2;

    Instruction_cond(std::vector<Item> items) {
      this->name = items[0];
      this->t1 = items[1];
      this->op = items[2];
      this->t2 = items[3];
    }
  };

  // label
  struct Instruction_label : Instruction {
    Item label;

    Instruction_label(std::vector<Item> items){
      this->label = items[0];
    }
  };

  // if (cond) label label
  struct Instruction_if : Instruction {
    Item t1, op, t2, label1, label2;

    Instruction_if(std::vector<Item> items) {
      this->t1 = items[0];
      this->op = items[1];
      this->t2 = items[2];
      this->label1 = items[3];
      this->label2 = items[4];
    }
  };

  // br label
  struct Instruction_br : Instruction {
    Item label;

    Instruction_br(std::vector<Item> items) {
      this->label = items[0];
    }
  };

  // return t?
  struct Instruction_return : Instruction {
    Item t;

    Instruction_return(std::vector<Item> items) {
      if (items.size() == 1) {
        this->t = items[0];
      }
      else {
        this->t.value = "no return here bub";
      }
    }
  };

  // while (cond) label label
  struct Instruction_while : Instruction {
    Item t1, op, t2, label1, label2;

    Instruction_while(std::vector<Item> items){
      this->t1 = items[0];
      this->op = items[1];
      this->t2 = items[2];
      this->label1 = items[3];
      this->label2 = items[4];
    }
  };

  // continue
  struct Instruction_continue : Instruction {

    Instruction_continue(std::vector<Item> items) {

    }
  };

  // break
  struct Instruction_break : Instruction {

    Instruction_break(std::vector<Item> items) {

    }
  };

  // name <- name[t]*
  struct Instruction_load_array : Instruction {
    Item name, array;
    std::vector<Item> indices;

    Instruction_load_array(std::vector<Item> items) {
      this->name = items[0];
      this->array = items[1];
      for (int ii = 2; ii<items.size(); ii++){
        this->indices.push_back(items[ii]);
      }
    }
  };

  // name[t]* <- s
  struct Instruction_store_array : Instruction {
    Item array, s;
    std::vector<Item> indices;

    Instruction_store_array(std::vector<Item> items) {
      this->s = items[items.size()-1];
      this->array = items[0];
      for (int ii = 1; ii<items.size()-1; ii++){
        this->indices.push_back(items[ii]);
      }
    } 
  };

  // name <- length name t
  struct Instruction_length : Instruction {
    Item name1, name2, t;

    Instruction_length(std::vector<Item> items) {
      this->name1 = items[0];
      this->name2 = items[1];
      this->t = items[2];
    }
  };

  // name(args?)
  struct Instruction_call : Instruction {
    Item callee;
    std::vector<Item> arguments;

    Instruction_call(std::vector<Item> items) {
      this->callee = items[0];
      for (int ii = 1; ii<items.size(); ii++){
        this->arguments.push_back(items[ii]);
      }
    }
  };

  // name <- name(args?)
  struct Instruction_save_call : Instruction {
    Item name, callee;
    std::vector<Item> arguments;

    Instruction_save_call(std::vector<Item> items) {
      this->name = items[0];
      this->callee = items[1];
      for (int ii = 2; ii<items.size(); ii++){
        this->arguments.push_back(items[ii]);
      }
    }
  };

  // print(t)
  struct Instruction_print : Instruction {
    Item t;

    Instruction_print(std::vector<Item> items){
      this->t = items[0];
    }
  };

  // name <- new Array(args)
  // name <- new Tuple(t)
  struct Instruction_make_array : Instruction {
    Item name;
    std::vector<Item> arguments;

    Instruction_make_array(std::vector<Item> items) {
      this->name = items[0];
      for (int ii = 1; ii<items.size(); ii++){
        this->arguments.push_back(items[ii]);
      }
    }
  };




}