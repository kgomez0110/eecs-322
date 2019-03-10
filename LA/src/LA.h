#pragma once

#include <vector>
#include <set>
#include <unordered_map>
#include <string>


namespace LA {

  const std::vector<std::string> types = {"int64", "void", "code"};
  struct Instruction;

  struct Item {
    std::string value;
  };

  struct Function {
    std::string name;
    std::string type;
    std::vector<Item> arguments;
    std::vector<Instruction*> instructions;
  };

  struct Program{
    std::vector<Function*> functions;
    std::vector<std::string>* function_names;
  };

  struct Instruction {
    virtual std::string toIR() = 0;
    std::vector<std::string>* f_names;
    bool isFunction(Item i) {
      std::vector<std::string> names = (*f_names);
      return (std::find(names.begin(), names.end(), i.value) != names.end());
    }
    bool isType (Item i) {
      return (i.value == "code" || i.value == "void" || i.value.substr(0,5) == "int64");
    }
    std::string convertVar(Item i, bool call_instruct = false) {
      try {
        std::stoi(i.value);
        return i.value;
      }
      catch (const std::exception& e){
        if (i.value[0] == ':' || isType(i))
          return i.value;
        else if (call_instruct && isFunction(i))
          return ":" + i.value;
        else
          return "%" + i.value;
      }
    }

    Item encodeConst(Item i) {
      try {
        int encoded = std::stoi(i.value);
        encoded <<= 1;
        encoded++;
        Item newItem;
        newItem.value = std::to_string(encoded);
        return newItem;
      }
      catch (const std::exception& e){
        return i;
      }
    }

    bool isVar(Item i) {
      try {
        std::stoi(i.value);
        return false;
      }
      catch (const std::exception& e){
        if (i.value[0] == ':')
          return false;
        else 
          return true;
      }
    }

    std::pair<std::string, Item> toDecode(Item i, int count = 0) {
      std::string result = "";
      std::string temp_var = "%dwoooootempboooys_" + std::to_string(count);
      Item new_item;
      new_item.value = temp_var;
      result.append(temp_var + " <- " + convertVar(i) + " >> 1\n");
      std::pair<std::string, Item> p;
      p.first = result;
      p.second = new_item;
      return p;
    }

    std::string toEncode(Item i) {
      std::string result = "";
      result.append(convertVar(i) + " <- " + convertVar(i) + " << 1\n");
      result.append(convertVar(i) + " <- " + convertVar(i) + " + 1\n");
      return result;
    }
  };

 


  // return
  struct Instruction_return : Instruction {

    std::string toIR() {
      return "return";
    }
  };

  // return t
  struct Instruction_return_t : Instruction {
    Item t;
    std::string toIR() {
      return "return " + convertVar(t);
    }
    Instruction_return_t(Item i) : t(i) {}
  };

  // label
  struct Instruction_label : Instruction {
    Item label;
    std::string toIR() {
      return label.value;
    }

    Instruction_label(Item l) : label(l) {}
  };

  // br t label label
  struct Instruction_br_t : Instruction {
    Item t, label1, label2;

    std::string toIR() {
      std::string result = "";
      if (isVar(t)){
        std::pair<std::string, Item> decoded = toDecode(t);
        Item t_decoded = decoded.second;
        result.append(decoded.first);
        result.append("br " + convertVar(t_decoded) + " " + label1.value + " " + label2.value);
        return result;
      }
      return "br " + t.value + " " + label1.value + " " + label2.value;
    }

    Instruction_br_t(Item temp, Item l1, Item l2) : t(encodeConst(temp)), label1(l1), label2(l2) {}
  };

  // br label
  struct Instruction_br : Instruction {
    Item label;
    std::string toIR() {
      return "br " + label.value;
    }

    Instruction_br(Item l) : label(l) {}
  };

  // name <- length name t
  struct Instruction_get_length : Instruction {
    Item var1, var2, t;

    std::string toIR() {
      return convertVar(var1) + " <- length " + convertVar(var2) + " " + convertVar(t);
    }

    Instruction_get_length(Item v1, Item v2, Item t1) : var1(v1), var2(v2), t(t1) {}
  };

  // var <- call callee (args)
  // name <- name(args)
  struct Instruction_save_call : Instruction {
    Item var, callee;
    std::vector<Item> args;

    std::string toIR() {
      std::string result = "";
      result.append(convertVar(var) + " <- call " + convertVar(callee, true) + "(");
      for (int ii = 0; ii<args.size(); ii++){
        if (ii == 0)
          result.append(convertVar(args[ii]));
        else
          result.append(", " + convertVar(args[ii]));
      }
      result.append(")");
      return result;
    }

    Instruction_save_call (std::vector<Item> a, std::vector<std::string>* names) {
      this->var = a[0];
      this->callee = a[1];
      this->f_names = names;
      for (int ii = 2; ii < a.size(); ii++){
        this->args.push_back(encodeConst(a[ii]));
      }
    }
  };

  // call callee (args)
  // name(args)
  struct Instruction_call : Instruction {
    Item callee;
    std::vector<Item> args;
    std::string toIR() {
      std::string result = "";
      result.append("call " + convertVar(callee, true) + "(");
      for (int ii = 0; ii<args.size(); ii++){
        if (ii == 0)
          result.append(convertVar(args[ii]));
        else
          result.append(", " + convertVar(args[ii]));
      }
      result.append(")");
      return result;
    }

    Instruction_call (std::vector<Item> a, std::vector<std::string>* names) {
      this->callee = a[0];
      this->f_names = names;
      for (int ii = 1; ii < a.size(); ii++){
        this->args.push_back(encodeConst(a[ii]));
      }
    }
  };

  // var <- var[t]*
  // name <- name[t]*
  struct Instruction_load_array : Instruction {
    Item name, array;
    std::vector<Item> indices;
    int count;

    std::string toIR() {
      std::string array_check = "";
      std::string error = ":arrayerr_" + std::to_string(count);
      std::string cont = ":continue_" + std::to_string(count);
      std::string len_var = "%sizeofdimension_yeet";
      std::string cmp_var = "%comparingsize_yeet";

      array_check.append(cmp_var + " <- " + convertVar(array) + " = 0\n");
      array_check.append("br " + cmp_var + " " + error + " " + cont + "\n");
      array_check.append(error + "\n");
      array_check.append("call array-error(0,0)\n");
      array_check.append(cont + "\n");


      // array access check
      error = error + "_" + std::to_string(count);
      cont = cont + "_" + std::to_string(count);

      for (int ii = 0; ii<indices.size(); ii++) {
        array_check.append(len_var + " <- length " + convertVar(array) + std::to_string(ii) + "\n");
        array_check.append(cmp_var + " <- " + convertVar(indices[ii]) + " < " + len_var + "\n");
        array_check.append("br " + cmp_var + " " + cont + " " + error + "\n");
        array_check.append(error + "\n");
        array_check.append("call array-error(" + convertVar(array) + ", " + convertVar(indices[ii]) + ")\n");
        array_check.append(cont + "\n");

        error = error + "_" + std::to_string(count);
        cont = cont + "_" + std::to_string(count);
      }

      std::string result = array_check + convertVar(name) + " <- " + convertVar(array);
      std::string decoded = "";
      std::vector<Item> real_indices;
      int count = 0;
      for (int ii = 0; ii<indices.size(); ii++) {
        Item i = indices[ii];
        if (isVar(i)){
          std::pair<std::string, Item> decode_pair = toDecode(i, count);
          decoded.append(decode_pair.first);
          real_indices.push_back(decode_pair.second);
        }

        result.append("[" + convertVar(i) + "]");
      }
      return result;

    }

    Instruction_load_array(std::vector<Item> args, int c) {
      this->count = c;
      this->name = args[0];
      this->array = args[1];
      for (int ii = 2; ii<args.size(); ii++){
        this->indices.push_back(args[ii]);
      }
    }
  };

  // var[t]* <- var
  // name[t]* <- name

  /*
  array[i] <- var

  cmp_var <- array = 1
  br cmp_var :error :continue
  :error
  call array-error(0,0)
  :continue

  len <- length array k  // where k is the dimension
  cmp_var <- i < len
  br cmp_var :continue2 :error2
  :error2
  call array-error(array, i)
  :continue2

  var[t] <- var

  */
  struct Instruction_store_array : Instruction {
    Item name, array;
    std::vector<Item> indices;
    int count;
    std::string toIR() {

      // initial declaration check
      std::string array_check = "";
      std::string error = ":arrayerr_" + std::to_string(count);
      std::string cont = ":continue_" + std::to_string(count);
      std::string len_var = "%sizeofdimension_yeet";
      std::string cmp_var = "%comparingsize_yeet";


      array_check.append(cmp_var + " <- " + convertVar(array) + " = 0\n");
      array_check.append("br " + cmp_var + " " + error + " " + cont + "\n");
      array_check.append(error + "\n");
      array_check.append("call array-error(0,0)\n");
      array_check.append(cont + "\n");

      // array access check
      
      error = error + "_" + std::to_string(count);
      cont = cont + "_" + std::to_string(count);

      for (int ii = 0; ii<indices.size(); ii++) {
        array_check.append(len_var + " <- length " + convertVar(array) + std::to_string(ii) + "\n");
        array_check.append(cmp_var + " <- " + convertVar(indices[ii]) + " < " + len_var + "\n");
        array_check.append("br " + cmp_var + " " + cont + " " + error + "\n");
        array_check.append(error + "\n");
        array_check.append("call array-error(" + convertVar(array) + ", " + convertVar(indices[ii]) + ")\n");
        array_check.append(cont + "\n");

        error = error + "_" + std::to_string(count);
        cont = cont + "_" + std::to_string(count);
      }

      std::string result = array_check;
      result.append(convertVar(array));
      for (Item i : indices){
        result.append("[" + convertVar(i) + "]");
      }
      result.append(" <- " + convertVar(name));
      return result;
    }

    Instruction_store_array(std::vector<Item> args, int c) {
      this->count = c;
      this->name = args[args.size()-1];
      this->array = args[0];
      for (int ii = 1; ii<args.size()-1; ii++){
        this->indices.push_back(args[ii]);
      }
    }
  };

  // var <- t op t
  // name <- t op t
  struct Instruction_op : Instruction {
    Item var, t1, t2, op;
    std::string toIR() {
      std::string result = convertVar(var) + " <- " + convertVar(t1) + " " + op.value + " " + convertVar(t2);
      result.append(toEncode(var));
      return result;
    }  

    Instruction_op(Item v, Item t, Item o, Item ot) : var(v), t1(encodeConst(t)), t2(encodeConst(ot)), op(o) {}
  };

  // var <- new Array(args)
  struct Instruction_make_array : Instruction {
    Item var;
    std::vector<Item> args;
    std::string toIR() {
      std::string result = "";
      result.append(convertVar(var) + " <- new Array(");
      for (int ii = 0; ii<args.size(); ii++){
        if (ii == 0)
          result.append(convertVar(args[ii]));
        else
          result.append(", " + convertVar(args[ii]));
      }
      result.append(")");
      return result;
    }
    Instruction_make_array(std::vector<Item> a) {
      this->var = a[0];
      for (int ii = 1; ii<a.size(); ii++){
        this->args.push_back(encodeConst(a[ii]));
      }
    }
  };

  // var <- new Tuple(t);
  struct Instruction_make_tuple : Instruction {
    Item var, t;
    std::string toIR() {
      return convertVar(var) + " <- new Tuple(" + convertVar(t) + ")";
    } 
    Instruction_make_tuple(std::vector<Item> a) {
      this->var = a[0];
      this->t = encodeConst(a[1]);
    }
  };

  // var <- s
  struct Instruction_move : Instruction {
    Item var, s;
    std::string toIR() {
      return convertVar(var) + " <- " + convertVar(s);
    } 

    Instruction_move(Item v, Item s1) : var(v), s(encodeConst(s1)) {}
  };

  // type var
  struct Instruction_declare_var : Instruction {
    Item type, var;
    std::string toIR() {
      std::string result = "";
      result.append(convertVar(var) + " <- 0\n");
      result.append(type.value + " " + convertVar(var));
      return result;
    } 

    Instruction_declare_var(Item t, Item v) : type(t), var(v) {}
  };

  struct Instruction_print : Instruction {
    Item t;
    std::string toIR() {
      return "call print(" + convertVar(t) + ")";
    }

    Instruction_print(Item t1) : t(t1) {}
  };

}