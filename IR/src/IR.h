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

  struct Program {
    std::vector<Function*> functions;
  };

  struct Function {
    std::string name;
    std::string type;
    std::vector<Instruction*> instructions;
    std::vector<Item> arguments;
  };

  struct Instruction {
    virtual std::string toL3() = 0;
  };

  struct Item {
    std::string value;
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
      result.append("%tempv <- 8 * " + t.value + "\n");
      result.append("%tempv0 <- %tempv + 16\n");
      result.append("%tempv1 <- " + var2.value + " + %tempv0\n");
      result.append(var1.value + " <- load %tempv1");
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
    Item var1, array;
    std::vector<Item> dimensions;
    std::string toL3() {
      int offset = 16 + (dimensions.size() * 8);
      std::vector<std::string> size_address;
      std::vector<std::string> sizes;
      std::vector<std::string> new_vars;
      std::string address_var = "%ADDR_";
      std::string size_var = "%SIZE_";
      std::string new_var = "%newVar_";
      std::string final_address = "%final_addy";
      std::string offset_var = "%Offset";
      std::string result = "";
      for (int ii = 0; ii<dimensions.size(); ii++){
        size_address.push_back(address_var + std::to_string(ii));
        sizes.push_back(size_var + std::to_string(ii));
        new_vars.push_back(new_var + std::to_string(ii));
      }

      // grabs the size of each dimension and loads them into the size vars
      for (int ii = 0; ii<dimensions.size()-1; ii++){
        int size_offset = 16 + (8*(1+ii)); // the location in memory the size of the dimensions is stored
        result.append(size_address[ii] + " <- " + array.value + " + " + std::to_string(size_offset) + "\n");
        result.append(sizes[ii] + " <- load " + size_address[ii] + "\n");
        result.append(sizes[ii] + " <- " + sizes[ii] + " >> 1\n");
      }
      for (int ii = dimensions.size()-1; ii>=0; ii--) {
        if (ii == dimensions.size() - 1) {
          result.append(final_address + " <- " + dimensions[ii].value + "\n");
        }
        else {
          result.append(new_vars[ii] + " <- " + dimensions[ii].value + "\n");
          for (int jj = ii+1; jj<dimensions.size(); jj++){
            result.append(new_vars[ii] + " <- " + new_vars[ii] + " * " + sizes[ii] + "\n");
          }
          result.append(final_address + " <- " + final_address + " + " + new_vars[ii] + "\n");
        }
      }

      result.append(final_address + " <- " + final_address + " * 8\n");
      result.append(final_address + " <- " + final_address + " + " + std::to_string(offset) + "\n");
      result.append(final_address + " <- " + final_address + " + " + array.value + "\n");
      
      result.append(var1.value + " <- load " + final_address);
      return result;
    }

    Instruction_load_array(std::vector<Item> args) {
      this->var1 = args[0];
      this->array = args[1];
      for (int ii = 2; ii<args.size(); ii++){
        this->dimensions.push_back(args[ii]);
      }
    }
  };

  // var[t]* <- var
  struct Instruction_store_array : Instruction {
    Item var1, array;
    std::vector<Item> dimensions;

    std::string toL3() {
      int offset = 16 + (dimensions.size() * 8);
      std::vector<std::string> size_address;
      std::vector<std::string> sizes;
      std::vector<std::string> new_vars;
      std::string address_var = "%ADDR_";
      std::string size_var = "%SIZE_";
      std::string new_var = "%newVar_";
      std::string final_address = "%final_addy";
      std::string offset_var = "%Offset";
      std::string result = "";
      for (int ii = 0; ii<dimensions.size(); ii++){
        size_address.push_back(address_var + std::to_string(ii));
        sizes.push_back(size_var + std::to_string(ii));
        new_vars.push_back(new_var + std::to_string(ii));
      }

      // grabs the size of each dimension and loads them into the size vars
      for (int ii = 0; ii<dimensions.size()-1; ii++){
        int size_offset = 16 + (8*(1+ii)); // the location in memory the size of the dimensions is stored
        result.append(size_address[ii] + " <- " + array.value + " + " + std::to_string(size_offset) + "\n");
        result.append(sizes[ii] + " <- load " + size_address[ii] + "\n");
        result.append(sizes[ii] + " <- " + sizes[ii] + " >> 1\n");
      }

      for (int ii = dimensions.size()-1; ii>=0; ii--) {
        if (ii == dimensions.size() - 1) {
          result.append(final_address + " <- " + dimensions[ii].value + "\n");
        }
        else {
          result.append(new_vars[ii] + " <- " + dimensions[ii].value + "\n");
          for (int jj = ii+1; jj<dimensions.size(); jj++){
            result.append(new_vars[ii] + " <- " + new_vars[ii] + " * " + sizes[ii] + "\n");
          }
          result.append(final_address + " <- " + final_address + " + " + new_vars[ii] + "\n");
        }
      }

      result.append(final_address + " <- " + final_address + " * 8\n");
      result.append(final_address + " <- " + final_address + " + " + std::to_string(offset) + "\n");
      result.append(final_address + " <- " + final_address + " + " + array.value + "\n");
      
      result.append("store " + final_address + " <- " + var1.value);
      return result;

    }

    Instruction_store_array(std::vector<Item> args) {
      this->var1 = args[args.size()-1];
      this->array = args[0];
      for (int ii = 1; ii<args.size()-1; ii++){
        this->dimensions.push_back(args[ii]);
      }
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
      std::string arg_temp = "%pd";
      int arg_counter = 0;
      int temp_counter = 0;
      std::string temp = "%vd";
      std::string result = "";
      std::string v0 = makeVar(temp, temp_counter);
      // save # of dimensions for 1st argument
      result.append(v0 + " <- " + args[0].value + " >> 1\n");
      // for the rest of the dimensions, multiply it to get total size
      for (int ii = 1; ii<args.size(); ii++){
        std::string pd = makeVar(arg_temp, arg_counter);
        result.append(pd + " <- " + args[ii].value + " >> 1\n");
        result.append(v0 + " <- " + v0 + " * " + pd + "\n");
      }
      result.append(v0 + " <- " + v0 + " << 1\n");
      result.append(v0 + " <- " + v0 + " + 1\n");
      // this now has enough space for all the elements, plus 1 space to stroe # of dimensions and stores each of those dimensions
      result.append(v0 + " <- " + v0 + " + " + std::to_string((args.size()+1)*2) + "\n");
      
     // std::string a = makeVar(temp, temp_counter); // this is gonna be the array
      std::string a = var.value;
      result.append(a + " <- call allocate(" + v0 + ", 1)\n");
      std::string v1 = makeVar(temp, temp_counter);
      result.append(v1 + " <- " + a + " + 8\n");

      // this stores the number of dimensions
      result.append("store " + v1 + " <- " + std::to_string((args.size()*2)+1) + "\n");

      // stores each indiviudal dimension size
      int dim = 16;
      for (int ii = 0; ii<args.size(); ii++) {
        std::string v2 = makeVar(temp, temp_counter);
        result.append(v2 + " <- " + a + " + " + std::to_string(dim) + "\n");
        result.append("store " + v2 + " <- " + args[ii].value + "\n");
        dim += 8;
      }

      return result;
    }

    Instruction_make_array(std::vector<Item> items) {
      this->var = items[0];
      for (int ii = 1; ii <items.size(); ii++){
        this->args.push_back(items[ii]);
      }
    }

  };

  // var <- new Tuple(s)
  struct Instruction_make_tuple : Instruction {
    Item var, s;

    std::string toL3() {
      return var.value + " <- call allocate (" + s.value + ", 1)";
    }
    Instruction_make_tuple (std::vector<Item> items) {
      this->var = items[0];
      this->s = items[1];
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

  // var[t] <- var
  struct Instruction_store_tuple : Instruction { 
    Item tuple, var, index;

    std::string toL3() {
      std::string result = "";
      std::string temp = "%temp_wooo";
      result.append(temp + " <- 8 * " + index.value + "\n");
      result.append(temp + " <- 8 + " + temp + "\n");
      result.append(temp + " <- " + tuple.value + " + " + temp + "\n");
      result.append("store " + temp + " <- " + var.value);
      return result;
    }

    Instruction_store_tuple(std::vector<Item> items) {
      this->tuple = items[0];
      this->index = items[1];
      this->var = items[2];
    }

  };

  // var <- var[t]
  struct Instruction_load_tuple : Instruction { 
    Item var, tuple, index;

    std::string toL3() {
      std::string result = "";
      std::string temp = "%temp_woooo";
      result.append(temp + " <- 8 * " + index.value + "\n");
      result.append(temp + " <- 8 + " + temp + "\n");
      result.append(temp + " <- " + tuple.value + " + " + temp + "\n");
      result.append(var.value + " <- load " + temp);
      return result;
    }

    Instruction_load_tuple(std::vector<Item> items) {
      this->var = items[0];
      this->tuple = items[1];
      this->index = items[2];
    }
  };

}