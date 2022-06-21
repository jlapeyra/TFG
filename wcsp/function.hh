#ifndef FUNCTION_HH
#define FUNCTION_HH

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cassert>
#include "utils.cc"
using namespace std;
using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::pair;
using std::string;

class Function {// cost functions implemented as a flattened vector of costs
private:
  vector<int> scope; // list of variables (no order assumed)
  vector<int> domsize; //list of domain sizes
  vector<int> offset; //product of previous domain sizes
  map<int, int> var2pos; //where in the scope is the variable
  vector<Cost> costs; //cost function
  Cost top; // all values in costs must be <= top

  int  tuple2index(const vector<int>& t) const;
  vector<int>  index2tuple(int p) const;
  bool  indexContains(int p, int var, int val) const;
  int  index2var(int p, int var) const;
  int posVar(int var) const; //returns where is var in the scope, -1 if not in the scope
  Function  removeVar(int var, Cost fill) const;//returns a function with the same top and domain as "this" except for var with all costs =fill
  
public:
  Function(const vector<int>& s,const vector<int>& d, Cost def, Cost top, int sem=0);
  // if type==0 --> all costs are the default "def"
  // if type==1 --> cost= sum of values
  // if type==2 --> cost= |s|- number of different values
  bool sortedScope() const;
  Function sortScope() const;//because the wcsp format may have cost functions whose scope is not ordered, we use this method to order it;
  void  show(int level) const;
  bool coversScope(const vector<int>& s) const; //this.scope \subseteq s
  bool coversAssg(const vector<int>& assg) const;
  bool coversAssgButOne(const vector<int>& assg, int& var) const;
  void addCost(const vector<int>& t, Cost c);
  void setTop(Cost newtop) {top = newtop;}
  Cost getTop() const {return top;}
  bool check() const;// checks costs are between zero and top
  void updateTop(Cost newtop); //decreases all costs higher than newTop
  Cost getMinCost() const;
  void substractCost(Cost c); //substracts c from every tuple   
  Cost getCost(const vector<int>& t) const;// scope(t)==scope(this.scope)
  Cost getCostAssg(const vector<int>& assg) const;
  Cost getCostExtended(const vector<int>& t, const vector<int>& s)const;// scope(t)\superseteq scope(this.scope)
  vector<Cost> allCosts() const;
  vector<int> getScope() const;
  bool inScope(int var) const;
  int arity() const;

  //
  Function condition(int var, int val) const;
  Function project(int var) const;  
  Function join(const Function& f) const;

  //
  int numTuples() const;
  Cost getCost(int idx) const; // get cost from tuple index
  vector<int> getTuple(int idx) const; // get tuple from tuple index

};
#endif
