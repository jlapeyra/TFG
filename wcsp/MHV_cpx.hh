#ifndef MHV_CPX_HH
#define MHV_CPX_HH

#include <vector>
#include "wcsp.hh"
#include "utils.cc"
#include "callback.hh"

using std::vector;

#include <ilcplex/ilocplex.h>
ILOSTLBEGIN;

class MHV_cplex {
private:
    vector<vector<int>> K;   // set of cores where K[j][i] is the index of the cost for the i-th component of core K[j]
    vector<vector<Cost> > costs;  // costs[i][m] is the m-th cost of i-th function  (i.e. i-th component of each core)
                                  // (costs[i].size() - 1 is the index of the last finite cost)
                                  //       costs[i][0] = 0;  /* no --> max{0, min{costes de la función i-ésima}} */
    int e;    // número de componentes de los cores ( = número de funciones en el problema)
    
    IloEnv env;
    IloCplex cplex;
    IloModel model;
    vector<IloNumVarArray> h;
    IloExpr obj_expr; //objective expression to minimize
    IloObjective obj; //objective

    UBCallback* callback;
    
    vector<Cost> best_h;      //MHV
    vector<int>  best_h_idom; //MHV
    Cost         best_cost=0; //cost MHV
    vector<Cost> good_h;      //HV
    vector<int>  good_h_idom; //HV
    Cost         good_cost=0; //cost HV
    bool feasible;
    string hv_method; // "na" | "greedy" | "model" | "callback"
    // hv_method == "na"       : HV is not computed
    // hv_method == "greedy"   : HV is greedily extended from the last MHV found
    // hv_method == "model"    : HV is computed with the model used for MHV 
    //                           without objective function, but restricted to ub
    // hv_method == "callback" : ...
    
    static int nb_calls;

    bool solve(vector<Cost>& found_h, vector<int>& found_h_idom, Cost & found_cost);
    void updateHV_greedy(const vector<int>& k);

public:
    // domain[i][K[j][i]] is the i-th element of j-th vector
    // domain[i] is sorted ascendent
    // ¿domain?

    MHV_cplex() {} // only for compatibility
    MHV_cplex(const vector<vector<int>>& K, const vector<vector<Cost>>& costs, string hv_method);
    void addCore(const vector<int>& core);
    
    bool solve_MHV(long& time);
    Cost getCost_MHV() { return best_cost; }
    vector<int> getMHV_idom() { return best_h_idom; }
    vector<Cost> getMHV() { return best_h; }

    bool solve_HV(Cost ub, long& time);
    Cost getCost_HV() { return good_cost; }
    vector<int> getHV_idom() { return good_h_idom; }
    vector<Cost> getHV() { return good_h; }
    
    bool isHV(const vector<int>& h_idom); // is h_idom hitting vector

    template <typename T>
    static bool hit(const vector<T>& a, const vector<T>& b) {return not (a <= b);} 
    // a hits b  <=>  ∃i: a[i] > b[i]  <=>  not (a <= b)

};
#endif
