#pragma once

#include <vector>
#include <set>
#include <string>

#include <L3.h>

namespace L3 {

  const std::vector<std::string> arguments = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
  struct Tile {
    int cost;
    Instruction* inst;
    virtual bool match(Tree* tree) = 0;
    virtual std::string toL2() = 0;
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
    int numChildren (Node* node) {
      return node->children.size();
    }
    std::string getNodeVal (Node* node) {
      return node->value.value;
    }
  };

/*
  ReturnTileT
  ReturnTile
  MoveTile
  CalleeTile
  SaveCalleeTile
  GotoTile
  CjumpTile
  LoadTile
  StoreTile
  MultiplyTile
  AddTile
  SubtractTile
  AndTile
  LessThanTile
  LessThanEqualTile
  EqualTile
  GreaterThanTile
  GreaterThanEqualTile
*/
  struct ReturnTileT : Tile {
    int cost = 2;
    std::string t;
    bool match(Tree* tree) {
      if (tree->head->value.value == "return" && tree->head->children.size() == 1){
        this->t = tree->head->children[0]->value.value;
        return true;
      }
      return false;
    }
    std::string toL2() {
      return "rax <- " + this->t + "\nreturn " + this->t;
    }
  };

  struct ReturnTile : Tile {
    int cost = 1;
    bool match(Tree* tree){
      return (tree->head->value.value == "return" && tree->head->children.size() == 0);
    }
    std::string toL2() {
      return "return";
    }
  };

  struct MoveTile : Tile {
    int cost = 1;
    std::string var, s;
    bool match(Tree* tree){
      if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && !NodeOpP(tree->head)){
        this->var = getNodeVal(tree->head);
        this->s = getNodeVal(tree->head->children[0]);
        return true;
      }
      return false;
    }
    std::string toL2() {
      return this->var + " <- " + this->s;
    }
  };

  struct CalleeTile : Tile {
    int cost = 3;
    std::string callee;
    std::vector<std::string> args;
    bool match(Tree* tree) {
      if (NodeCalleeP(tree->head)){
        this->callee = getNodeVal(tree->head);
        for (Node* child : tree->head->children){
          this->args.push_back(getNodeVal(child));
          this->cost++;
        }
        return true;
      }
      return false;
    }
    std::string toL2() {
      std::string result = "";
      std::string ret_label = this->callee + "_ret";
      result.append("mem rsp -8 <- " + ret_label + "\n");
      for (int ii = 0; ii < this->args.size(); ii++) {
        if (ii < 6) {
          result.append(arguments[ii] + " <- " + this->args[ii] + "\n");
        }
        else {
          result.append("stack-arg " + std::to_string((ii - 6) * 8) + "<- " + this->args[ii] + "\n");
        }
      }
      result.append("call " + this->callee + std::to_string(this->args.size()) + "\n" + ret_label);
      return result;
    }
  };

  struct SaveCalleeTile : Tile {
    int cost = 4;
    std::string var;
    std::string callee;
    std::vector<std::string> args;
    bool match(Tree* tree) {
      if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && NodeCalleeP(tree->head->children[0])){
        this->var = getNodeVal(tree->head);
        this->callee = getNodeVal(tree->head->children[0]);
        for (Node* child : tree->head->children[0]->children){
          this->args.push_back(getNodeVal(child));
          this->cost++;
        }
        return true;
      }
      return false;
    }

    std::string toL2() {
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
      result.append("call " + this->callee + std::to_string(this->args.size()) + "\n");
      result.append(this->var + " <- rax\n" + ret_label);
      return result;
    }
  };
  
  struct GotoTile : Tile {
    int cost = 1;
    std::string label;
    
    bool match(Tree* tree) {
      if (NodeOpP(tree->head) && numChildren(tree->head) == 1){
        this->label = getNodeVal(tree->head->children[0]);
        return true;
      }
      return false;
    }

    std::string toL2() {
      return "goto " + this->label;
    }
  };

  struct CjumpTile : Tile { 
    int cost = 1;
    std::string label, var;

    bool match(Tree* tree) {
      if (NodeOpP(tree->head) && numChildren(tree->head) == 2) {
        this->label = getNodeVal(tree->head->children[1]);
        this->var = getNodeVal(tree->head->children[0]);
        return true;
      }
      return false;
    }

    std::string toL2() {
      return "cjump " + this->var + " = 1 " + this->label;
    }
  };

  struct LoadTile : Tile { 
    int cost = 1;
    std::string var1, var2;

    bool match(Tree* tree) {
      if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == "load") {
        this->var1 = getNodeVal(tree->head);
        this->var2 = getNodeVal(tree->head->children[0]->children[0]);
        return true;
      }
      return false;
    }

    std::string toL2() {
      return this->var1 + " <- mem rsp " + this->var2 + "0";
    }
  };

  struct StoreTile : Tile {
    int cost = 1;
    std::string var, s;

    bool match(Tree* tree) {
      if (getNodeVal(tree->head) == "store"){
        this->var = getNodeVal(tree->head->children[0]);
        this->s = getNodeVal(tree->head->children[1]);
        return true;
      }
      return false;
    }

    std::string toL2() {
      return "mem " + this->var + " 0 <- " + this->s;
    }
  };

  struct MultiplyTile : Tile {
    int cost = 2;
    std::string var, t1, t2;

    bool match(Tree* tree) {
      if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == "*"){
        this->var = getNodeVal(tree->head);
        this->t1 = getNodeVal(tree->head->children[0]->children[0]);
        this->t2 = getNodeVal(tree->head->children[0]->children[1]);
        return true;
      }
      return false;
    }

    std::string toL2() {
      return this->var + " <- " + this->t1 + "\n " + this->var + " *= " + this->t2;
    }
  };

  struct AddTile : Tile {
    int cost = 2;
    std::string var, t1, t2;

    bool match(Tree* tree) {
      if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == "+"){
        this->var = getNodeVal(tree->head);
        this->t1 = getNodeVal(tree->head->children[0]->children[0]);
        this->t2 = getNodeVal(tree->head->children[0]->children[1]);
        return true;
      }
      return false;
    }

    std::string toL2() {
      return this->var + " <- " + this->t1 + "\n " + this->var + " += " + this->t2;
    }
  };

  struct SubtractTile : Tile {
    int cost = 2;
    std::string var, t1, t2;

    bool match(Tree* tree) {
      if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == "-"){
        this->var = getNodeVal(tree->head);
        this->t1 = getNodeVal(tree->head->children[0]->children[0]);
        this->t2 = getNodeVal(tree->head->children[0]->children[1]);
        return true;
      }
      return false;
    }

    std::string toL2() {
      return this->var + " <- " + this->t1 + "\n " + this->var + " -= " + this->t2;
    }
  };

  struct AndTile : Tile {
    int cost = 2;
    std::string var, t1, t2;

    bool match(Tree* tree) {
      if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == "&"){
        this->var = getNodeVal(tree->head);
        this->t1 = getNodeVal(tree->head->children[0]->children[0]);
        this->t2 = getNodeVal(tree->head->children[0]->children[1]);
        return true;
      }
      return false;
    }

    std::string toL2() {
      return this->var + " <- " + this->t1 + "\n " + this->var + " &= " + this->t2;
    }
  };

  struct LessThanTile : Tile {
    int cost = 2;
    std::string var, t1, t2;

    bool match(Tree* tree) {
      if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == "<"){
        this->var = getNodeVal(tree->head);
        this->t1 = getNodeVal(tree->head->children[0]->children[0]);
        this->t2 = getNodeVal(tree->head->children[0]->children[1]);
        return true;
      }
      return false;
    }

    std::string toL2() {
      return this->var + " <- " + this->t1 + " < " + this->t2;
    }
  };

  struct LessThanEqualTile : Tile {
    int cost = 2;
    std::string var, t1, t2;

    bool match(Tree* tree) {
      if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == "<="){
        this->var = getNodeVal(tree->head);
        this->t1 = getNodeVal(tree->head->children[0]->children[0]);
        this->t2 = getNodeVal(tree->head->children[0]->children[1]);
        return true;
      }
      return false;
    }

    std::string toL2() {
      return this->var + " <- " + this->t1 + " <= " + this->t2;
    }
  };

  struct EqualTile : Tile {
    int cost = 2;
    std::string var, t1, t2;

    bool match(Tree* tree) {
      if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == "="){
        this->var = getNodeVal(tree->head);
        this->t1 = getNodeVal(tree->head->children[0]->children[0]);
        this->t2 = getNodeVal(tree->head->children[0]->children[1]);
        return true;
      }
      return false;
    }

    std::string toL2() {
      return this->var + " <- " + this->t1 + " = " + this->t2;
    }
  };

  struct GreaterThanTile : Tile {
    int cost = 2;
    std::string var, t1, t2;

    bool match(Tree* tree) {
      if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == ">"){
        this->var = getNodeVal(tree->head);
        this->t1 = getNodeVal(tree->head->children[0]->children[0]);
        this->t2 = getNodeVal(tree->head->children[0]->children[1]);
        return true;
      }
      return false;
    }

    std::string toL2() {
      return this->var + " <- " + this->t2 + " < " + this->t1;
    }
  };

  struct GreaterThanEqualTile : Tile {
    int cost = 2;
    std::string var, t1, t2;

    bool match(Tree* tree) {
      if (NodeVarP(tree->head) && numChildren(tree->head) == 1 && getNodeVal(tree->head->children[0]) == ">="){
        this->var = getNodeVal(tree->head);
        this->t1 = getNodeVal(tree->head->children[0]->children[0]);
        this->t2 = getNodeVal(tree->head->children[0]->children[1]);
        return true;
      }
      return false;
    }

    std::string toL2() {
      return this->var + " <- " + this->t2 + " <= " + this->t1;
    }
  };

}