#ifndef CORE_CSP_HH
#define CORE_CSP_HH

#include "wcsp.hh"
#include <vector>
#include <chrono>
using namespace std::chrono;

class CoreCSP {
public:
    const Wcsp& wcsp;
    vector<vector<int>> C; //cores, C ⊆ {k | h ≤ k, wcsp^k unsat}
    vector<int> sol; //solution, wcsp^h(sol) = true

    CoreCSP(const Wcsp& wcsp) : wcsp(wcsp) {}

    // if wcsp^h sat   -> return true
    // if wcsp^h unsat -> return false & store a set of cores at C (see getCores())
    virtual bool solve(vector<int> h) = 0;
    
    // if wcsp^h sat   -> return true
    // if wcsp^h unsat -> return false & store a set of cores at C (see getCores())
    bool solve(const vector<int> &h, long &time) {
        auto start = high_resolution_clock::now();
        bool ret = solve(h);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        time += duration.count();
        //cout << "ret"<<ret<< endl;
        return ret;
    }

    // PRE: solve(h) called & returned FALSE
    //      i.e. wcsp^h unsat
    // return a set of cores for wcsp^h
    //      i.e. return C s.t. C ⊆ {k | h ≤ k, wcsp^k unsat}
    vector<vector<int>>& getCores() {return C;}

    // PRE: solve(h) called & returned TRUE
    //      i.e. wcsp^h sat
    // return a solution for wcsp^h
    //      i.e. return sol s.t. wcsp^h(sol) = true
    vector<int> getSolution() {return sol;}

protected:
    int idx_min_Cost(const vector<int>& k) {
        assert(wcsp.nfuncs == k.size() and k.size() > 0);
        int min_f = 0;
        Cost min_cost = wcsp.index2cost(0, k[0]);
        for (int f = 1; f < wcsp.nfuncs; f++) {
            Cost cost = wcsp.index2cost(f, k[f]);
            if (cost < min_cost) {
                min_f = f;
                min_cost = cost;
            }
        }
        return min_f;
    }


};

#endif