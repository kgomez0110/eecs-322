#pragma once

#include <vector>
#include <set>
#include <unordered_map>
#include <string>

namespace L3 {
  struct Instruction;
  struct Function;
  struct Program;
  struct Variable;
  struct Context;
  struct Tree;
  struct Node;
  struct Tile;
  struct Item;

  enum Instruction_Type {
    RETURN,
    RETURN_T,
    VAR_CALL,
    CALL,
    BR_VAR,
    BR,
    LOAD,
    STORE,
    THREE_OP,
    CMP,
    MOVE,
    LABEL
  };

  enum Node_Type {
    VAR,
    CONST,
    OP,
    CALLEE 
  };

  struct Item {
    bool isVar;
    std::string value;
    Variable* var;
  };

  struct Node {
    Node_Type type;
    Item value;
    std::vector<Node*> children;
  };

  struct Tree {
    Node* head;
    int size;
  };  


  struct Variable {
    std::string name;
    std::vector<Instruction*> instructions;
    std::vector<std::pair<int, int>> live_ranges;
  };

  struct Instruction {
    Instruction_Type type;
    Tree* tree;
    std::set<Variable*> gen;
    std::set<Variable*> kill;
    int line_num;
    virtual void makeTree() = 0;
  };

  struct Context {
    std::vector<Instruction*> instructions;
    std::vector<Tree*> merged_trees;
    int num_instructions;
  };


  struct Function {
    std::string name;
    std::vector<Instruction*> label_instructions;
    std::vector<Context*> contexts;
    std::unordered_map<std::string, Variable*> variables;
    std::vector<Instruction*> instructions;
    std::vector<Item> arguments;
  };

  struct Program {
    std::vector<Function*> functions;
  };

  // return
  struct Instruction_Return : Instruction {
    Instruction_Type type;
    int line_num;
    Tree* tree;
    std::set<Variable*> gen;
    std::set<Variable*> kill;
    void makeTree() { 
      this->tree = new Tree();
      auto n = new Node();
      n->type = OP;
      Item ret;
      ret.value = "return";
      ret.isVar = false;
      n->value = ret;
      this->tree->head = n;
    }
    Instruction_Return(int num) : type(RETURN), line_num(num) { 
      makeTree();
    }
  };

  // return t
  struct Instruction_Return_T : Instruction {
    Instruction_Type type;
    Item t;
    int line_num;
    std::set<Variable*> gen;
    std::set<Variable*> kill;
    Tree* tree;
    void makeTree() { 
      this->tree = new Tree();
      auto n = new Node();
      auto t_node = new Node();
      n->type = OP;
      Item ret;
      ret.value = "return";
      ret.isVar = false;
      n->value = ret;
      if (t.isVar)
        t_node->type = VAR;
      else 
        t_node->type = CONST;
      t_node->value = this->t;
      n->children.push_back(t_node);
      this->tree->head = n;
    }
    Instruction_Return_T(Item i, int num) : type(RETURN_T), t(i), line_num(num) { 
      makeTree();
    }
  };

  // var <- call callee (args)
  struct Instruction_var_call : Instruction {
    Instruction_Type type;
    Item var;
    Item callee;
    std::vector<Item> args;
    int line_num;
    std::set<Variable*> gen;
    std::set<Variable*> kill;
    Tree* tree;
    void makeTree(){
      this->tree = new Tree();
      auto n = new Node();
      auto op = new Node();
      n->type = VAR;
      n->value = this->var;
      op->type = CALLEE;
      op->value = this->callee;
      n->children.push_back(op);
      for (Item ii : this->args){
        auto i = new Node();
        if (ii.isVar)
          i->type = VAR;
        else 
          i->type = CONST;
        i->value = ii;
        op->children.push_back(i);
      }
      this->tree->head = n;
    }
    Instruction_var_call(Item v, Item c, std::vector<Item> a, int num) : type(VAR_CALL), var(v), callee(c), args(a), line_num(num) { 
      makeTree();
    }
  };

  // call callee (args)
  struct Instruction_call : Instruction {
    Instruction_Type type;
    std::vector<Item> args;
    Item callee;
    int line_num;
    std::set<Variable*> gen;
    std::set<Variable*> kill;
    Tree* tree;
    void makeTree(){
      this->tree = new Tree();
      auto n = new Node();
      n->type = CALLEE;
      n->value = this->callee;
      for (Item ii : this->args){
        auto i = new Node();
        if (ii.isVar)
          i->type = VAR;
        else 
          i->type = CONST;
        i->value = ii;
        n->children.push_back(i);
      }
      this->tree->head = n;
    }
    Instruction_call(Item c, std::vector<Item> a, int num) : type(CALL), callee(c), args(a), line_num(num) { 
      makeTree();
    }
  };

  // br var label
  struct Instruction_br_var : Instruction {
    Instruction_Type type;
    Item var, label;
    int line_num;
    std::set<Variable*> gen;
    std::set<Variable*> kill;
    Tree* tree;
    void makeTree() { 
      this->tree = new Tree();
      auto n = new Node();
      n->type = OP;
      Item br;
      br.value = "br";
      br.isVar = false;
      n->value = br;
      auto var_node = new Node();
      var_node->value = this->var;
      var_node->type = VAR;
      auto label_node = new Node();
      label_node->value = this->label;
      label_node->type = CONST;
      n->children.push_back(var_node);
      n->children.push_back(label_node);
      this->tree->head = n;
    }
    Instruction_br_var(Item v, Item l, int num) : type(BR_VAR), var(v), label(l), line_num(num) { 
      makeTree();
    }
  };

  // br label
  struct Instruction_br : Instruction {
    Instruction_Type type;
    Item label;
    int line_num;
    std::set<Variable*> gen;
    std::set<Variable*> kill;
    Tree* tree;
    void makeTree() { 
      this->tree = new Tree();
      auto n = new Node();
      n->type = OP;
      Item br;
      br.value = "br";
      br.isVar = false;
      n->value = br;
      auto label_node = new Node();
      label_node->value = this->label;
      label_node->type = CONST;
      n->children.push_back(label_node);
      this->tree->head = n;
    }
    Instruction_br(Item l, int num) : type(BR), label(l), line_num(num) { 
      makeTree();
    }
  };

  // var <- load var
  struct Instruction_load : Instruction {
    Instruction_Type type;
    Item var1, var2;
    int line_num;
    std::set<Variable*> gen;
    std::set<Variable*> kill;
    Tree* tree;
    void makeTree() { 
      this->tree = new Tree();
      auto var1_node = new Node();
      var1_node->type = VAR;
      var1_node->value = this->var1;
      auto var2_node = new Node();
      var2_node->type = VAR;
      var1_node->value = this->var2;
      auto load_node = new Node();
      load_node->type = OP;
      Item load;
      load.isVar = false;
      load.value = "load";
      load_node->value = load;
      var1_node->children.push_back(load_node);
      load_node->children.push_back(var2_node);
      this->tree->head = var1_node;
    }
    Instruction_load(Item v1, Item v2, int num) : type(LOAD), var1(v1), var2(v2), line_num(num) { 
      makeTree();
    }
  };

  // store var <- s
  struct Instruction_store : Instruction {
    Instruction_Type type;
    Item var, s;
    int line_num;
    std::set<Variable*> gen;
    std::set<Variable*> kill;
    Tree* tree;
    void makeTree(){
      this->tree = new Tree();
      auto store_node = new Node();
      Item store;
      store.isVar = false;
      store.value = "store";
      store_node->value = store;
      auto var_node = new Node();
      var_node->value = this->var;
      var_node->type = VAR;
      auto s_node = new Node();
      s_node->value = this->s;
      if (this->s.isVar)
        s_node->type = VAR;
      else
        s_node->type = CONST;
      store_node->children.push_back(var_node);
      store_node->children.push_back(s_node);
      this->tree->head = store_node;
    }
    Instruction_store(Item v, Item n, int num) : type(STORE), var(v), s(n), line_num(num) {
      makeTree();
    }
  };

  // var <- t op t
  struct Instruction_op : Instruction {
    Instruction_Type type;
    Item var, t1, t2, op;
    int line_num;
    std::set<Variable*> gen;
    std::set<Variable*> kill;
    Tree* tree;
    void makeTree() { 
      this->tree = new Tree();
      auto var_node = new Node();
      var_node->type = VAR;
      var_node->value = this->var;
      auto t1_node = new Node();
      t1_node->value = this->t1;
      if (this->t1.isVar)
        t1_node->type = VAR;
      else
        t1_node->type = CONST;
      auto t2_node = new Node();
      t2_node->value = this->t2;
      if (this->t2.isVar)
        t2_node->type = VAR;
      else
        t2_node->type = CONST;
      
      auto op_node = new Node();
      op_node->value = this->op;
      op_node->type = OP;

      var_node->children.push_back(op_node);
      op_node->children.push_back(t1_node);
      op_node->children.push_back(t2_node);

      this->tree->head = var_node;
    }
    Instruction_op(Item v, Item n1, Item o, Item n2, int num) : type(THREE_OP), var(v), t1(n1), t2(n2), op(o), line_num(num) {
      makeTree();
    }
  };

  // var <- t cmp t
  struct Instruction_cmp : Instruction {
    Instruction_Type type;
    Item var, t1, t2, cmp;
    int line_num;
    std::set<Variable*> gen;
    std::set<Variable*> kill;
    Tree* tree;
    void makeTree() { 
      this->tree = new Tree();
      auto var_node = new Node();
      var_node->type = VAR;
      var_node->value = this->var;
      auto t1_node = new Node();
      t1_node->value = this->t1;
      if (this->t1.isVar)
        t1_node->type = VAR;
      else
        t1_node->type = CONST;
      auto t2_node = new Node();
      t2_node->value = this->t2;
      if (this->t2.isVar)
        t2_node->type = VAR;
      else
        t2_node->type = CONST;
      
      auto op_node = new Node();
      op_node->value = this->cmp;
      op_node->type = OP;

      var_node->children.push_back(op_node);
      op_node->children.push_back(t1_node);
      op_node->children.push_back(t2_node);

      this->tree->head = var_node;
    }
    Instruction_cmp(Item v, Item n1, Item c, Item n2, int num) : type(CMP), var(v), t1(n1), t2(n2), cmp(c), line_num(num) { 
      makeTree();
    } 
  };

  // var <- s
  struct Instruction_move : Instruction { 
    Instruction_Type type;
    Item var, s;
    int line_num;
    std::set<Variable*> gen;
    std::set<Variable*> kill;
    Tree* tree;
    void makeTree() { 
      this->tree = new Tree();
      auto var_node = new Node();
      var_node->type = VAR;
      var_node->value = this->var;
      auto s_node = new Node();
      s_node->value = this->s;
      if (this->s.isVar)
        s_node->type = VAR;
      else 
        s_node->type = CONST;
      
      var_node->children.push_back(s_node);
      this->tree->head = var_node;
    }
    Instruction_move(Item v, Item val, int num) : type(MOVE), var(v), s(val), line_num(num) { 
      makeTree();
    }
  };

  // label
  struct Instruction_label : Instruction {
    Instruction_Type type;
    Item label;
    int line_num;
    std::set<Variable*> gen;
    std::set<Variable*> kill;
    Tree* tree;
    void makeTree() { 
      this->tree = new Tree();
      auto label_node = new Node();
      label_node->value = label;
      label_node->type = CONST;
      this->tree->head = label_node;
    }
    Instruction_label(Item l, int num) : type(LABEL), label(l), line_num(num) {
      makeTree();
    }
  };





  
}