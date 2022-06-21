#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "function.hh"
#include "wcsp.hh"

using namespace std;
using std::cout;
using std::endl;
using std::map;
using std::pair;
using std::set;
using std::string;
using std::vector;

Wcsp::Wcsp() : nvars(0), nfuncs(0), lb(0), ub(0) {}

void Wcsp::showCore(const vector<int> &k) const {
  assert(k.size() == functions.size());
  for (int i = 0; i < k.size(); i++)
    if (k[i] < costs[i].size()) {
      cout << "(";
      functions[i].show(1);
      cout << "cost " << k[i] << ") ";
    }
  cout << endl;
}

Cost Wcsp::vector_cost(const vector<int> &h) const {
  assert(h.size() == functions.size());
  Cost c = 0;
  for (int i = 0; i < h.size(); i++) {
    c = c + costs[i][h[i]];
  }
  return c;
}

int Wcsp::cost2index(int func, Cost c) const {
  for (int i = 0; i < costs[func].size(); i++)
    if (c == costs[func][i])
      return i;
  return costs[func].size();
}

Cost Wcsp::index2cost(int func, int idx) const {
  if (idx >= costs[func].size()) return ub;
  return costs[func][idx];
}

void Wcsp::sortVariables(int option) {
  vector<pair<int, int>> v(nvars);
  for (int i = 0; i < nvars; i++)
    v[i] = make_pair(var2functions[i].size(), i);
  sort(v.begin(), v.end());
  for (int i = 0; i < nvars; i++)
    varOrd[i] = v[i].second;
}

void Wcsp::read(string fileName) { //(char const* fileName) {
  fstream file(fileName);
  if (not file.is_open()) {
    cerr << "Error: File " << fileName << "cannot be opened" << endl;
    exit(EXIT_FAILURE);
  }
  string name;
  int maxdomsize;
  file >> name >> nvars >> maxdomsize >> nfuncs >> ub;
  lb = 0;
  Cost lb2 = 0;
  domsize = vector<int>(nvars);
  for (int i = 0; i < nvars; ++i)
    file >> domsize[i];
  bool consistent = true; // there must be a zero cost in every cost function
  int unsorted = 0;
  var2functions = vector<vector<int>>(nvars);
  for (int i = 0; i < nfuncs; ++i) {
    int arity;
    file >> arity;
    if (arity == 0) {
      Cost c;
      int ntuples;
      file >> c >> ntuples;
      lb += c;
      assert(ntuples == 0);
    } else {
      vector<int> scope;
      vector<int> domscope;
      int v;
      for (int k = 0; k < arity; k++) {
        file >> v;
        scope.push_back(v);
        domscope.push_back(domsize[v]);
        var2functions[v].push_back(i);
      }
      int ntuples;
      Cost defcost;
      file >> defcost >> ntuples;
      Function f(scope, domscope, defcost, ub);
      for (int j = 0; j < ntuples; ++j) {
        vector<int> t(arity);
        for (int k = 0; k < arity; k++)
          file >> t[k];
        Cost c;
        file >> c;
        f.addCost(t, c);
      }
      Cost mincost = f.getMinCost();
      if (mincost > 0) { // it is not NC*
        f.substractCost(mincost);
        consistent = false;
        lb2 = lb2 + mincost;
      }
      if (f.sortedScope())
        functions.push_back(f);
      else {
        unsorted++;
        Function f2 = f.sortScope();
        functions.push_back(f2);
      }
    }
  }

  nfuncs = functions.size();

  file.close();

  // final adjustments
  cout << "lb " << lb << " ub " << ub << " before adjustment" << endl;
  if (not consistent)
    cout << "Warning, the problem was not even NC " << lb2 << endl;
  if (unsorted > 0)
    cout << unsorted << " unsorted cost functions" << endl;
  lb = lb + lb2;
  assert(lb < ub);
  ub = ub - lb;
  cout << "lb " << 0 << " ub " << ub << " after adjustment" << endl;
  if (lb > 0)
    for (int i = 0; i < functions.size(); i++)
      functions[i].updateTop(ub);

  // initialize variable ordering, by default lexicographic
  varOrd = vector<int>(nvars);
  for (int i = 0; i < nvars; i++)
    varOrd[i] = i;

  // create cost vectors
  costs = vector<vector<Cost>>(functions.size());
  for (int i = 0; i < functions.size(); i++)
    costs[i] = functions[i].allCosts();
  cout << "costs ready"<<endl;

  Cost new_ub = 0;
  for (int i = 0; i < functions.size(); i++)
    new_ub += costs[i][costs[i].size()-1];
  new_ub += 1;
  if (new_ub < ub) {
    ub = new_ub;
    for (int i = 0; i < functions.size(); i++)
      functions[i].updateTop(ub);
    cout << "ub " << ub << " after 2nd adjustment" << endl;
  }

  for (int i = 0; i < nfuncs; i++) {
    assert(functions[i].getTop() == ub);
    assert(functions[i].check());
  }
}

void Wcsp::show(int level) const {

 
  cout << nvars << " variables ";
  cout << nfuncs << " functions " << lb << " lb, " << ub << " ub" << endl;
  if (level > 0) {
    /*
  cout<<"Var ordering:";
  for(int i=0;i<varOrd.size(); i++) cout<<varOrd[i]<<" ";
  cout<<endl;
    */
     cout << "Domsize:";
    for (int i = 0; i < domsize.size(); i++)
      cout << domsize[i] << " ";
    cout << endl;

    int maxarity=0;
    for(int i=0;i<nfuncs; i++) if(functions[i].arity()>maxarity) maxarity=functions[i].arity();
    vector<int> frecArities(maxarity+1,0);
    for(int i=0;i<nfuncs; i++) frecArities[functions[i].arity()]++;
    cout << "arities (%) ";
    for (int i = 1; i < frecArities.size(); i++)
      cout << "(" << i << ", " << (frecArities[i]*100)/nfuncs << "), ";
    cout << endl;

    int maxcosts=0;
    for(int i=0;i<nfuncs; i++) if(costs[i].size()>maxcosts) maxcosts=costs[i].size();
    vector<int> frecCosts(maxcosts+1,0);
    for(int i=0;i<nfuncs; i++) frecCosts[costs[i].size()]++;
    cout << "costs (%) ";
    for (int i = 1; i < frecCosts.size(); i++)
      cout << "(" << i << ", " << (frecCosts[i]*100)/nfuncs << "), ";
    cout << endl;

    
  }
  if (level > 1) {
    cout << "Arities/granularity";
    for (int i = 0; i < functions.size(); i++)
      cout << "(" << functions[i].arity() << ", " << costs[i].size() << "), ";
    cout << endl;

    cout << "Neigbors: ";
    for (int i = 0; i < nvars; i++) {
      cout << i << ":";
      for (int j = 0; j < var2functions[i].size(); j++)
        cout << functions[var2functions[i][j]].arity() << " ";
      cout << endl;
    }
    /*
    cout<<"Costs: ";
    for(int i=0;i<functions.size();i++){
      cout<<i<<":";
      for(int j=0;j<costs[i].size();j++) cout<<costs[i][j]<<" ";
      cout<<endl;
    }
    */
  }
}

void Wcsp::BE() const {}
