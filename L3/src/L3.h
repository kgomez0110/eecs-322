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
    VAR_CALL_RUNTIME,
    CALL,
    CALL_RUNTIME,
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
    CALLEE,
    CALLEE_RUNTIME 
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
    Item t, callee, var, label, var1, var2, s, t1, t2, op, cmp;
    std::vector<Item> args;
    int line_num;
    void makeTree() {
      this->tree = new Tree();
      switch (this->type) {
        case RETURN : {
          auto n = new Node();
          n->type = OP;
          Item ret;
          ret.value = "return";
          ret.isVar = false;
          n->value = ret;
          this->tree->head = n;
          break;
        }

        case RETURN_T : {
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
          break;
        }

        case VAR_CALL : {
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
          break;
        }

        case VAR_CALL_RUNTIME : {
          auto n = new Node();
          auto op = new Node();
          n->type = VAR;
          n->value = this->var;
          op->type = CALLEE_RUNTIME;
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
          break;
        }

        case CALL : {
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
          break;
        }

        case CALL_RUNTIME : {
          auto n = new Node();
          n->type = CALLEE_RUNTIME;
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
          break;
        }

        case BR_VAR : {
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
          break;
        }

        case BR : {
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
          break;
        }

        case LOAD : {
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
          break;
        }

        case STORE : {
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
          break;
        }

        case THREE_OP : {
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
          break;
        }

        case CMP : {
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
          break;
        }

        case MOVE : {
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
          break;
        }

        case LABEL : {
          auto label_node = new Node();
          label_node->value = label;
          label_node->type = CONST;
          this->tree->head = label_node;
          break;
        }
      }
    }

    Instruction(Instruction_Type t, std::vector<Item> i ,int l) : line_num(l), type(t) {
      switch (t) {
        case RETURN :
          // return
          break;

        case RETURN_T : {
          // return t
          this->t = i[0];
          break;
        }
        
        case VAR_CALL : {
          // var <- call callee (args)
          this->var = i[0];
          this->callee = i[1];
          std::vector<Item>::iterator it = i.begin() + 2;
          std::vector<Item> arguments(it, i.end());
          this->args = arguments;
          break;
        }

        case VAR_CALL_RUNTIME : {
          // var <- call callee (args)
          this->var = i[0];
          this->callee = i[1];
          std::vector<Item>::iterator it = i.begin() + 2;
          std::vector<Item> arguments(it, i.end());
          this->args = arguments;
          break;
        }
        
        case CALL : {
          // call callee (args)
          this->callee = i[0];
          std::vector<Item>::iterator it = i.begin() + 1;
          std::vector<Item> arguments(it, i.end());
          this->args = arguments;
          break;
        }

        case CALL_RUNTIME : {
          // call callee (args)
          this->callee = i[0];
          std::vector<Item>::iterator it = i.begin() + 1;
          std::vector<Item> arguments(it, i.end());
          this->args = arguments;
          break;
        }

        case BR_VAR : {
          // br var label
          this->var = i[0];
          this->label = i[1];
          break;
        }

        case BR : {
          // br label
          this->label = i[0];
          break;
        }
        
        case LOAD : {
          // var <- load var
          this->var1 = i[0];
          this->var2 = i[1];
          break;
        }
        
        case STORE : {
          // store var <- s
          this->var = i[0];
          this->s = i[1];
          break;
        }

        case THREE_OP : {
          // var <- t op t
          this->var = i[0];
          this->t1 = i[1];
          this->op = i[2];
          this->t2 = i[3];
          break; 
        }
        
        case CMP : {
          // var <- t cmp t
          this->var = i[0];
          this->t1 = i[1];
          this->cmp = i[2];
          this->t2 = i[3];
          break;
        }
        
        case MOVE: {
          // var <- s
          this->var = i[0];
          this->s = i[1];
          break;
        }
        
        case LABEL : {
          // label
          this->label = i[0];
          break;
        }
      }
      this->makeTree();
    }
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

  





  
}