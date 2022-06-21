//#include "types.hh"
#include "wcsp.hh"
#include "MHV_cpx.hh"
#include <vector>
#include <cassert>
#include <string>
#include <sstream>
//#include <iostream>
#include <chrono>
using namespace std::chrono;

using std::vector;

#include <ilcplex/ilocplex.h>
ILOSTLBEGIN


int MHV_cplex::nb_calls = 0;

// domain[i][K[j][i]] is the i-th element of j-th vector
// domain[i] is sorted ascendent
MHV_cplex::MHV_cplex(const vector<vector<int>>& K, const vector<vector<Cost>>& costs, string hv_method = "na")
: K(K), costs(costs), e(costs.size()), hv_method(hv_method) {

    model = IloModel(env);

    // variables (h[][])
    // h[i][k]  <=>  h_mhv[i] > domain[k]   
    h = vector<IloNumVarArray>(e); //(comps);
    for (int i = 0; i < e; i++) {
        //domsize[i] = domain[i].size();
        //h[i] = IloNumVarArray(env, domsize[i] - 1, 0, 1, ILOBOOL);  // Emma: -1 porque exite INF? (y si una función no tiene INF?)
                                                                    //      tiene que ser domsize[i]: los cores con domsize[i] - 1 no se pueden hitear, 
                                                                    //      pero domsize[i] - 1 es una variable porque ella sí que puede hitear a los cores
        h[i] = IloNumVarArray(env, costs[i].size(), 0, 1, ILOBOOL); 
    }

    if (hv_method == "greedy") {
        good_h_idom = vector<int>(e, 0);
        good_h = vector<Cost>(e);
        for (int i = 0; i < e; ++i) good_h[i] = costs[i][good_h_idom[i]];
        good_cost = sum(good_h);
    }

    // restriccions per tal que h sigui consistent
    // ∀i ∀k : h[i][k+1] >= h[i][k]    // Emma: h[i][k + 1] <= h[i][k]  : es al revés la inecuación (?)
    for (int i = 0; i < e; ++i) {
        for (int m = 1; m < costs[i].size(); ++m) model.add(h[i][m - 1] - h[i][m] >= 0);
    }

    // restriccions del problema
    // ∀i : h_mhv hits K[j]  
    //          i.e. ∀i ∃j : h_mhv[i] > domain[C[j][i]]
    for (int j = 0; j < K.size(); j++) {
        addCore(K[j]);
    }
    obj_expr = IloExpr(env);
    for (int i = 0; i < e; ++i) {
        obj_expr += h[i][0]*(costs[i][0]); 
        assert(costs[i][0]==0);         // como costs[i][0] == 0 se podría quitar porque no aporta valor al objetivo
        for (int m = 1; m < costs[i].size(); ++m) {
            assert(costs[i][m] - costs[i][m - 1]>0);
            obj_expr += h[i][m]*(costs[i][m] - costs[i][m - 1]);  
        }
    }
    obj = IloMinimize(env, obj_expr);
    model.add(obj);
    //obj_expr.end();

    cplex = IloCplex(model);
    
    // configuration of output information given by cplex
    cplex.setOut(env.getNullStream());
    cplex.setWarning(env.getNullStream());
    cplex.setError(env.getNullStream());

    if (hv_method == "callback") {
        callback = new UBCallback(env);
        callback->active = false;
        cplex.use(IloCplex::Callback(callback));
    }
}
    
void MHV_cplex::addCore(const vector<int>& core) {
    // restricció:
    //      h_mhv hits core
    //          i.e. ∀i ∃j : h_mhv[i] > domain[K[j][i]]     ¿¿i.e. ∀i ∃j : h[i][K[j][i]]???
    int len = 0;
    IloEnv env = model.getEnv();
    IloExpr expr(env);
    assert(core.size() == e);
    for (int i = 0; i < e; i++) {
        if (core[i] < costs[i].size() - 1) {
            //std::cout<<i<<" ";
            expr += h[i][core[i] + 1];
            ++len;
        }
    }
    model.add(expr >= 1);
    expr.end(); // Emma


    if (hv_method == "greedy") {
        updateHV_greedy(core);
    }      
    //std::cout << std::endl << " --- Added new core of length " << len << std::endl;      
}

// POST: h hits k
void MHV_cplex::updateHV_greedy(const vector<int>& k) {
    // h hits k  <=>  ∃i: h[i] > k[i]  <=>  not (h <= k)
    if (not hit(good_h_idom, k)) {
        assert(k.size() == e and good_h_idom.size() == e);
        Cost best_inc = numeric_limits<Cost>::max();
        int best_i = -1;
        for (int i = 0; i < e; i++) {
            if (good_h_idom[i] <= k[i] and k[i] < costs[i].size() - 1) {
                Cost inc = costs[i][k[i]+1] - costs[i][good_h_idom[i]];
                if (inc < best_inc) {
                    best_inc = inc;
                    best_i = i;
                }
            }
        }
        if (best_i == -1) {
            cout << "Unhittable core" << endl;
            cout << k << endl;
            exit(EXIT_FAILURE);
        }
        good_h_idom[best_i] = k[best_i]+1;
        good_h[best_i] = costs[best_i][good_h_idom[best_i]];
        good_cost += best_inc;
        assert(good_cost == sum(good_h));
        assert(hit(good_h_idom, k));
    }
}

bool MHV_cplex::solve(vector<Cost>& found_h, vector<int>& found_h_idom, Cost & found_cost) {

    feasible = cplex.solve() == IloTrue;

    if (not feasible) return false;
    
    found_h = vector<Cost>(e);
    found_h_idom = vector<int>(e);
    for (int i = 0; i < e; ++i) {
        int m = costs[i].size() - 1;
        while (m > 0 and cplex.getIntValue(h[i][m]) == 0) --m;
        found_h_idom[i] = m;
        found_h[i] = costs[i][m];
    }
    assert(isHV(found_h_idom));

    found_cost = sum(found_h);

    return true; //feasible
}

bool MHV_cplex::solve_MHV(long& time) { //solve MHV
    //std::cout << "empezado solve ..." << std::endl;

    ++nb_calls;

    auto start = high_resolution_clock::now();
    
    feasible = solve(best_h, best_h_idom, best_cost);
    good_h = best_h;
    good_h_idom = best_h_idom;
    good_cost = best_cost;

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    time += duration.count();

    return true; //feasible
}


bool MHV_cplex::solve_HV(Cost ub, long& time) {
    auto start = high_resolution_clock::now();

    if (hv_method == "model") {
        model.remove(obj);
        IloConstraint constraint_lt_ub = (obj_expr < ub); //cplex accepta enters de <= 53 bits
        model.add(constraint_lt_ub); 

        feasible = solve(good_h, good_h_idom, good_cost);
        
        model.remove(constraint_lt_ub);
        if (not feasible) //cas particular en què lb == ub
            feasible = solve(good_h, good_h_idom, good_cost);

        model.add(obj);
    }
    else if (hv_method == "callback") {
        callback->active = true;
        callback->ub = ub;
        feasible = solve(good_h, good_h_idom, good_cost);
        callback->active = false;
    }
    else feasible = true;

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    time += duration.count();

    return feasible;
}



// h_idom is HS <=> ∀i h_idom hits K[i]
bool MHV_cplex::isHV(const vector<int>& h_idom) {
    for (const vector<int>& vec : K) {
        if (not hit(h_idom, vec)) return false;
    }
    return true;
}




