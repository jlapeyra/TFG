#ifndef WCSP_HH
#define WCSP_HH

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cassert>
using namespace std;
using std::cout;
using std::endl;

using std::vector;
using std::map;
using std::pair;
using std::string;

#include "function.hh"

class Wcsp{
public:
  int nvars;
  int nfuncs;
  Cost lb;
  Cost ub;
  vector<int> domsize;
  vector<Function> functions;
  vector<vector<int> > var2functions; //for each variable, functions where it appears
  vector<vector<Cost> > costs;	// functions costs
  vector<int> varOrd; // variable ordering

  //vector<int> functions_label;
  //vector<vector<int> > Bucket; //for each variable; its bucket

public:
  Wcsp();
  int getVar(int i) const {return varOrd[i];}
  void showCore(const vector<int>& k) const;// shows the hiteable components of the core
  Cost vector_cost(const vector<int>& h) const;// computes the cost of a hitting vector
  int cost2index(int func, Cost c) const;// computes the position of c in costs[func]
  Cost index2cost(int func, int idx) const;// computes funcs's idx-th cost (last cost is ub)
  void sortVariables(int option);
  void read(string fileName);//(char const* fileName); // reads a .wcsp file
  void show(int level) const;
  void BE() const;
};
#endif
