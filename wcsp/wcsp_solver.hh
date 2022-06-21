#ifndef WCSP_SOLVER_HH
#define WCSP_SOLVER_HH


#include "MHV_cpx.hh"
#include "function.hh"
#include "wcsp.hh"
#include "csp.hh"
#include "forward_checking.hh"
#include "csp_sat.hh"
#include "utils.cc"

#include <iostream>
#include <algorithm>
//#include <boost/program_options.hpp>
#include <assert.h>

#include <chrono>
using namespace std::chrono;

using namespace std;


class WcspSolver {
  
  bool dominated(const vector<int> &k1, const vector<int> &k2); // k1<= k2 ?
  bool hits(const vector<int> &h, const vector<vector<int>> &K2); // tells if h hits all the cores in K2
  void add_core(vector<vector<int>> &K2, const vector<int> &k); // adds k to K2 and removes from K2 all dominated vectors
  void extend_K(vector<vector<int>>& C); // K += C;  K2 += C;  C = []

  
  void solve_MHV(vector<int>& h, long& time); // h = MHV(K)
  void solve_HV(vector<int>& h, long& time, Cost ub); // h = HV(K)


  const Wcsp& wcsp;       // WCSP data 
  MHV_cplex K;            // MHV solver for the set of cores
  vector<vector<int>> K2; // set of cores (copy of K) needed for the greedy
  CoreCSP* csp;           // CSP solver

  const string alg_version; // "lb"|"lub"
  const string hv_method;   // "na"|"greedy"|"model"

public:

  /*
  PRE: csp_solver  = "sat"|"fc"
  PRE: alg_version = "lb"|"lub"
  PRE: if (alg_version = "lub") then hv_method = "greedy"|"model"
  */
  WcspSolver(const Wcsp &wcsp, string csp_solver, string alg_version, string hv_method);

  Cost solve();

  // PRE: solve() finished successfully
  vector<int> getSolution() {return csp->getSolution();}
};


#endif
