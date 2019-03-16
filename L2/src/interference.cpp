#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <utility>
#include <iostream>
#include <algorithm>
#include <stack>

#include <L2.h>
#include <interference.h>


using namespace std;


namespace L2 {

  std::vector<std::string> gp_reg = {"rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10", "r11", "rax", "r12", "r13", "r14", "r15", "rbp", "rbx"};
  std::vector<std::string> no_rcx = {"r10", "r11", "r8", "r9", "rax", "rdi", "rdx", "rsi", "r12", "r13", "r14", "r15", "rbp", "rbx"};
  
  Variable* getVarFromSet(std::set<Variable*> s, int64_t index){
    std::set<Variable*>::iterator it = s.begin();
    for (int ii = 0; ii < index; ii++){
      it++;
    }
    return *it;
  }
  void addEdge(Variable* v1, Variable* v2, Graph &g){
    if (v1 == v2){
      return;
    }
    if (!g.nodes.count(v1)){
      auto n = new Node(v1);
      g.nodes[v1] = n;
    }
    if (!g.nodes.count(v2)){
      auto n = new Node(v2);
      g.nodes[v2] = n;
    }
    g.nodes[v1]->neighbors.insert(g.nodes[v2]);
    g.nodes[v2]->neighbors.insert(g.nodes[v1]);
  }

  void addEdgesSet(std::set<Variable*> vars, Graph &g){
    int64_t num_vars = vars.size();
    for (int ii = 0; ii < num_vars-1; ii++){
      for (int jj = ii+1; jj < num_vars; jj++){
        addEdge(getVarFromSet(vars, ii), getVarFromSet(vars, jj), g);
      }
    }
  }

  void addEdgesBetweenSets(std::set<Variable*> s1, std::set<Variable*> s2, Graph &g){
    int64_t num_vars = s1.size();
    for (int ii = 0; ii < num_vars; ii++){
      for (Variable* var : s2){
        addEdge(getVarFromSet(s1, ii), var, g);
      }
    }
  }

  void addGPs(std::vector<std::string> gp, Graph &g, Function &f){
    int64_t num_reg = gp.size();
    Variable* second_reg;
    Variable* reg;
    for (int ii = 0; ii < num_reg-1; ii++){
      if (!f.variables.count(gp[ii])){
        reg = new Variable(gp[ii]);
        f.variables[gp[ii]] = reg;
      }
      for (int jj = ii+1; jj<num_reg; jj++){
        if (!f.variables.count(gp[jj])){
          second_reg = new Variable(gp[jj]);
          f.variables[gp[jj]] = second_reg;
        }

        addEdge(f.variables[gp[ii]], f.variables[gp[jj]], g);
      }
    }
  }

  void addShift(std::vector<std::string> gp, Variable* var, Graph &g, Function &f){
    int64_t num_reg = gp.size();
    Variable* reg;
    for (int ii = 0; ii < num_reg-1; ii++){
      if (!f.variables.count(gp[ii])){
        reg = new Variable(gp[ii]);
      }
      else{
        reg = f.variables[gp[ii]];
      } 
      addEdge(var, reg, g);
    }
  }

  Graph generateGraph(std::vector<std::set<Variable*>> in, std::vector<std::set<Variable*>> out, Function &f){
    Graph g;
    addGPs(gp_reg, g, f);
    int64_t num_instructions = in.size();
    for (int ii = 0; ii < num_instructions; ii++){
      addEdgesSet(in[ii], g);
      addEdgesSet(out[ii], g);
      addEdgesBetweenSets(f.instructions[ii]->variables_killed, out[ii], g);
      if (f.instructions[ii]->inst == L2::SOPSX){
        Variable* v = f.variables[f.instructions[ii]->operands[2]];
        addShift(no_rcx, v, g, f);
      }
    }
    std::unordered_map<Variable*, Node*>::iterator it = g.nodes.begin();
    while (it != g.nodes.end()){
      (*it).second->degree = (*it).second->neighbors.size();
      it++;
    }
    return g;
  }

  

  bool sortDegreeAscending(Node* a, Node* b){
    return a->degree < b->degree;
  }

  bool sortDegreeDescending(Node* a, Node* b){
    return a->degree > b->degree;
  }

  std::stack<Node*> orderedNodes(Graph &g){
    std::vector<Node*> nodes;
    std::stack<Node*> nodeStack;
    std::unordered_map<Variable*, Node*>::iterator it = g.nodes.begin();
    while (it != g.nodes.end()){
      nodes.push_back((*it).second);
      it++;
    }
    std::sort(nodes.begin(), nodes.end(), sortDegreeAscending);
    int index = 0;
    while (index < nodes.size() && nodes[index]->degree < 15){
      index++;
    }
    std::sort(nodes.begin()+index, nodes.end(), sortDegreeDescending);
    for (int ii = nodes.size()-1; ii>=0; ii--){
      nodeStack.push(nodes[ii]);
    }
    return nodeStack;
  }

  void colorRegisters(Graph &g, Function &f){
    for (int ii = 0; ii<15; ii++){
      g.nodes[f.variables[gp_reg[ii]]]->color = ii;
    }
  }

  std::vector<Variable*> coloring (Graph &g, Function &f){
    std::vector<Variable*> spilling = {};
    colorRegisters(g, f);
    std::stack<Node*> nodeStack = orderedNodes(g);
    Node* temp;
    while (!nodeStack.empty()){
      temp = nodeStack.top();
      if (temp->color == 15){
        for (int ii = 0; ii < 14; ii++){
          std::set<Node*>::iterator it = temp->neighbors.begin();
          while (it != temp->neighbors.end()){
            if ((*it)->color == ii)
              break;
            it++;
          }
          if (it == temp->neighbors.end()){
            temp->color = ii;
            break;
          }
        }
      }
      if (temp->color == 15){
        spilling.push_back(temp->var);
      }
      nodeStack.pop();
    }
    return spilling;
  }



}