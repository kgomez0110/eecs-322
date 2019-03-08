#pragma once

#include <vector>
#include <set>
#include <string>

#include <L3.h>

namespace L3 {

const std::vector<std::string> arguments = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
enum Tree_Type {
  RETURN_TILE_T,
  RETURN_TILE,
  MOVE_TILE,
  CALLEE_TILE,
  CALLEE_RUNTIME_TILE,
  SAVE_CALLEE_TILE,
  SAVE_CALLEE_RUNTIME_TILE,
  GOTO_TILE,
  CJUMP_TILE,
  LOAD_TILE,
  STORE_TILE,
  MULTIPLY_TILE,
  ADD_TILE,
  SUBTRACT_TILE,
  AND_TILE,
  LESS_THAN_TILE,
  LESS_THAN_EQUAL_TILE,
  EQUAL_TILE,
  GREATER_THAN_TILE,
  GREATER_THAN_EQUAL_TILE,
  LABEL_TILE,
  RIGHT_SHIFT_TILE,
  LEFT_SHIFT_TILE
};

struct Tile {
  int cost;
  Tree_Type type;
  std::string t, var, s, callee, label, var1, var2, t1, t2;
  std::vector<std::string> args; 
  bool NodeVarP (Node* node) {
    return node->type == VAR;
  }
  bool NodeConstP (Node* node) {
    return node->type == CONST;
  }
  bool NodeOpP (Node* node) {
    return node->type == OP;
  }
  bool NodeCalleeP (Node* node) {
    return node->type == CALLEE;
  }
  bool NodeCalleeRuntimeP (Node* node) {
    return node->type == CALLEE_RUNTIME;
  }
  int numChildren (Node* node) {
    return node->children.size();
  }
  std::string getNodeVal (Node* node) {
    return node->value.value;
  }
 
  std::string toL2(){
    switch (this->type) {
      case RETURN_TILE_T : {
        return "rax <- " + this->t + "\nreturn";
      }

      case RETURN_TILE : {
        return "return";
      }

      case MOVE_TILE : {
        return this->var + " <- " + this->s;
      }

      case CALLEE_TILE : {
        std::string result = "";
        std::string ret_label = this->callee + "_ret";
        result.append("mem rsp -8 <- " + ret_label + "\n");
        for (int ii = 0; ii < this->args.size(); ii++) {
          if (ii < 6) {
            result.append(arguments[ii] + " <- " + this->args[ii] + "\n");
          }
          else {
            result.append("stack-arg " + std::to_string((ii - 6) * 8) + " <- " + this->args[ii] + "\n");
          }
        }
        result.append("call " + this->callee + " " + std::to_string(this->args.size()) + "\n" + ret_label);
        return result;
      }

      case CALLEE_RUNTIME : {
        std::string result = "";
        for (int ii = 0; ii < this->args.size(); ii++) {
          if (ii < 6) {
            result.append(arguments[ii] + " <- " + this->args[ii] + "\n");
          }
          else {
            result.append("stack-arg " + std::to_string((ii - 6) * 8) + " <- " + this->args[ii] + "\n");
          }
        }
        result.append("call " + this->callee + " " + std::to_string(this->args.size()));
        return result;  
      }

      case SAVE_CALLEE_TILE : {
        std::string result = "";
        std::string ret_label = this->callee + "_ret";
        result.append("mem rsp -8 <- " + ret_label + "\n");
        for (int ii = 0; ii < this->args.size(); ii++) {
          if (ii < 6) {
            result.append(arguments[ii] + " <- " + this->args[ii] + "\n");
          }
          else {
            result.append("stack-arg " + std::to_string((ii - 6) * 8) + " <- " + this->args[ii] + "\n");
          }
        }
        result.append("call " + this->callee + " " + std::to_string(this->args.size()) + "\n");
        result.append(this->var + " <- rax\n" + ret_label);
        return result;
      }

      case SAVE_CALLEE_RUNTIME_TILE : {
        std::string result = "";
        for (int ii = 0; ii < this->args.size(); ii++) {
          if (ii < 6) {
            result.append(arguments[ii] + " <- " + this->args[ii] + "\n");
          }
          else {
            result.append("stack-arg " + std::to_string((ii - 6) * 8) + " <- " + this->args[ii] + "\n");
          }
        }
        result.append("call " + this->callee + " " + std::to_string(this->args.size()) + "\n");
        result.append(this->var + " <- rax");
        return result;
      }

      case GOTO_TILE : {
        return "goto " + this->label;
      }

      case CJUMP_TILE : {
        return "cjump " + this->var + " = 1 " + this->label;
      }

      case LOAD_TILE : {
        return this->var1 + " <- mem rsp " + this->var2 + "0";
      }

      case STORE_TILE : {
        return "mem " + this->var + " 0 <- " + this->s;
      }

      case MULTIPLY_TILE : {
        return this->var + " <- " + this->t1 + "\n" + this->var + " *= " + this->t2;
      }

      case ADD_TILE : {
        return this->var + " <- " + this->t1 + "\n" + this->var + " += " + this->t2;
      }

      case SUBTRACT_TILE : {
        return this->var + " <- " + this->t1 + "\n" + this->var + " -= " + this->t2;
      }

      case AND_TILE : {
        return this->var + " <- " + this->t1 + "\n" + this->var + " &= " + this->t2;
      }

      case LESS_THAN_TILE : {
        return this->var + " <- " + this->t1 + " < " + this->t2;
      }

      case LESS_THAN_EQUAL_TILE : {
        return this->var + " <- " + this->t1 + " <= " + this->t2;
      }

      case EQUAL_TILE : {
        return this->var + " <- " + this->t1 + " = " + this->t2;
      }

      case GREATER_THAN_EQUAL_TILE : {
        return this->var + " <- " + this->t2 + " <= " + this->t1;
      }

      case GREATER_THAN_TILE : {
        return this->var + " <- " + this->t2 + " < " + this->t1;
      }

      case LABEL_TILE : {
        return this->label;
      }

      case LEFT_SHIFT_TILE : {
        return this->var + " <- " + this->t1 + "\n" + this->var + " <<= " + this->t2;
      }

      case RIGHT_SHIFT_TILE : {
        return this->var + " <- " + this->t1 + "\n" + this->var + " >>= " + this->t2;
      }
    }
  }
  Tile(Tree* tree)  {
    if (tree->head->value.value == "return" && tree->head->children.size() == 1){
      this->t = tree->head->children[0]->value.value;
      this->type = RETURN_TILE_T;
    }
    else if (tree->head->value.value == "return" && tree->head->children.size() == 0){
      this->type = RETURN_TILE;
    }
    else if (NodeCalleeP(tree->head)){
      this->callee = getNodeVal(tree->head);
      for (Node* child : tree->head->children){
        this->args.push_back(getNodeVal(child));
        this->cost++;
      }
      this->type = CALLEE_TILE;
    }
    else if (NodeCalleeRuntimeP(tree->head)){
      this->callee = getNodeVal(tree->head);
      for (Node* child : tree->head->children){
        this->args.push_back(getNodeVal(child));
        this->cost++;
      }
      this->type = CALLEE_RUNTIME_TILE;
    }
    else if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && NodeCalleeP(tree->head->children[0])){
      this->var = getNodeVal(tree->head);
      this->callee = getNodeVal(tree->head->children[0]);
      for (Node* child : tree->head->children[0]->children){
        this->args.push_back(getNodeVal(child));
        this->cost++;
      }
      this->type = SAVE_CALLEE_TILE;
    }
    else if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && NodeCalleeRuntimeP(tree->head->children[0])){
      this->var = getNodeVal(tree->head);
      this->callee = getNodeVal(tree->head->children[0]);
      for (Node* child : tree->head->children[0]->children){
        this->args.push_back(getNodeVal(child));
        this->cost++;
      }
      this->type = SAVE_CALLEE_RUNTIME_TILE;
    }
    else if (NodeOpP(tree->head) && numChildren(tree->head) == 1){
      this->label = getNodeVal(tree->head->children[0]);
      this->type = GOTO_TILE;
    }
    else if (NodeOpP(tree->head) && numChildren(tree->head) == 2) {
      this->label = getNodeVal(tree->head->children[1]);
      this->var = getNodeVal(tree->head->children[0]);
      this->type = CJUMP_TILE;
    }
    else if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == "load") {
      this->var1 = getNodeVal(tree->head);
      this->var2 = getNodeVal(tree->head->children[0]->children[0]);
      this->type = LOAD_TILE;
    }
    else if (getNodeVal(tree->head) == "store"){
      this->var = getNodeVal(tree->head->children[0]);
      this->s = getNodeVal(tree->head->children[1]);
      this->type = STORE_TILE;
    }
    else if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == "*"){
      this->var = getNodeVal(tree->head);
      this->t1 = getNodeVal(tree->head->children[0]->children[0]);
      this->t2 = getNodeVal(tree->head->children[0]->children[1]);
      this->type = MULTIPLY_TILE;
    }
    else if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == "+"){
      this->var = getNodeVal(tree->head);
      this->t1 = getNodeVal(tree->head->children[0]->children[0]);
      this->t2 = getNodeVal(tree->head->children[0]->children[1]);
      this->type = ADD_TILE;
    }
    else if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == "-"){
      this->var = getNodeVal(tree->head);
      this->t1 = getNodeVal(tree->head->children[0]->children[0]);
      this->t2 = getNodeVal(tree->head->children[0]->children[1]);
      this->type = SUBTRACT_TILE;
    }
    else if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == "&"){
      this->var = getNodeVal(tree->head);
      this->t1 = getNodeVal(tree->head->children[0]->children[0]);
      this->t2 = getNodeVal(tree->head->children[0]->children[1]);
      this->type = AND_TILE;
    }
    else if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == "<"){
      this->var = getNodeVal(tree->head);
      this->t1 = getNodeVal(tree->head->children[0]->children[0]);
      this->t2 = getNodeVal(tree->head->children[0]->children[1]);
      this->type = LESS_THAN_TILE;
    }
    else if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == "<="){
      this->var = getNodeVal(tree->head);
      this->t1 = getNodeVal(tree->head->children[0]->children[0]);
      this->t2 = getNodeVal(tree->head->children[0]->children[1]);
      this->type = LESS_THAN_EQUAL_TILE;
    }
    else if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == "="){
      this->var = getNodeVal(tree->head);
      this->t1 = getNodeVal(tree->head->children[0]->children[0]);
      this->t2 = getNodeVal(tree->head->children[0]->children[1]);
      this->type = EQUAL_TILE;
    }
    else if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == ">"){
      this->var = getNodeVal(tree->head);
      this->t1 = getNodeVal(tree->head->children[0]->children[0]);
      this->t2 = getNodeVal(tree->head->children[0]->children[1]);
      this->type = GREATER_THAN_TILE;
    }
    else if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == ">="){
      this->var = getNodeVal(tree->head);
      this->t1 = getNodeVal(tree->head->children[0]->children[0]);
      this->t2 = getNodeVal(tree->head->children[0]->children[1]);
      this->type = GREATER_THAN_EQUAL_TILE;
    }
    else if (NodeConstP(tree->head) && numChildren(tree->head) == 0){
      this->label = getNodeVal(tree->head);
      this->type = LABEL_TILE;
    }
    else if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == "<<"){
      this->var = getNodeVal(tree->head);
      this->t1 = getNodeVal(tree->head->children[0]->children[0]);
      this->t2 = getNodeVal(tree->head->children[0]->children[1]);
      this->type = LEFT_SHIFT_TILE;
    }
    else if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == ">>"){
      this->var = getNodeVal(tree->head);
      this->t1 = getNodeVal(tree->head->children[0]->children[0]);
      this->t2 = getNodeVal(tree->head->children[0]->children[1]);
      this->type = RIGHT_SHIFT_TILE;
    }
    else if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && !NodeOpP(tree->head) ){
      this->var = getNodeVal(tree->head);
      this->s = getNodeVal(tree->head->children[0]);
      this->type = MOVE_TILE;
    }
  }
};




}