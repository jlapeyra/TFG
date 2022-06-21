#ifndef FORWARD_CHECKING_HH
#define FORWARD_CHECKING_HH

#include "function.hh"
#include "wcsp.hh"
#include "csp.hh"

#include <algorithm>
#include <iostream>
#include <random>
#include <assert.h>

using std::cout;
using std::endl;
using std::make_pair;
using std::pair;
using std::string;

class ForwardChecking : public CoreCSP {

public:
  ForwardChecking(const Wcsp& wcsp) : CoreCSP(wcsp) {}

private:
  /* variable selection heuristic for forward checking */
  int min_dom(const vector<int> &csp_assign, const vector<int> &csp_domsize);

  bool look_ahead(vector<int> &csp_assign, int var, int a, const vector<int> &h,
                  vector<vector<int>> &csp_doms,
                  vector<int> &csp_domsize, vector<pair<int, int>> &csp_changes,
                  vector<Cost> &k, const vector<int> &mask);

  bool forward_checking(vector<int> &csp_assign, int unassigned,
                        const vector<int> &h,
                        vector<vector<int>> &csp_doms, vector<int> &csp_domsize,
                        vector<Cost> &kcp, const vector<int> &mask);


  // gets ready for a new call to forward forward_checking
  // It prepares the doms and domsize variables
  //  mask tells which constraints are desactivated (value 0)
  //  assign tells which variables are desactivated (value -2)
  bool ini_domains(const vector<int> &h,
                  const vector<int> &assign, vector<vector<int>> &doms,
                  vector<int> &domsize, const vector<int> &mask);

public:
  // computes in C a set of cores of de csp defined as wcsp^h
  //  it does so by calling Forward checking untill the problem is satisfiable
  //  After each unsatisfiable call (which returns a core k) it removes all the
  //  constraints in the core and all the variables that become isolated
  bool solve(vector<int> h);

};


#endif