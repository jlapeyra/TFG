#ifndef CSPSAT_HH
#define CSPSAT_HH

#include "sat-cadical/src/cadical.hpp"
#include "wcsp.hh"
#include "function.hh"
#include "csp.hh"
#include <cassert>
#include <map>

#ifdef NDEBUG
#undef NDEBUG
#endif


class CSP_sat : public CoreCSP {

public:
//private:
    CaDiCaL::Solver solver;
    
    int lit_num = 1; //next avaliable literal
    vector<int> var2lit;
    vector<int> func2lit;

    int varVal2lit(int var, int val) const;
    int funcICost2lit(int func, int idx_cost) const;
    int funcCost2lit(int func, Cost cost) const;

    int smallestFail(const vector<int>& h, int func);

    bool solve(const vector<int> &h, vector<int>& k);

public:
    CSP_sat(const Wcsp& wcsp); // : CoreCSP(wcsp) {}
    bool solve(vector<int> h);

};


#endif