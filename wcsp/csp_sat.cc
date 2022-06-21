#include "csp_sat.hh"
#include <set>
#include <iostream>
using namespace std;
using std::cout;
using std::cerr;

//CaDiCaL::Solver * solver = new CaDiCaL::Solver;

CSP_sat::CSP_sat(const Wcsp& wcsp) : CoreCSP(wcsp) {
    lit_num = 1;

    var2lit = vector<int>(wcsp.nvars);
    for (int x = 0; x < wcsp.nvars; x++) {
        // ∀x,a: make atom xa
        //  where x variable, a in domain(x)
        var2lit[x] = lit_num;
        lit_num += wcsp.domsize[x];

        // ∀x: make clause (x1 v x2 v ... v xn),   
        //  where x variable, domain(x)=1..n
        //cerr << endl << "var = " << x << ", dom = " << wcsp.domsize[x] << endl;
        for (int a = 0; a < wcsp.domsize[x]; a++) {
            solver.add(varVal2lit(x,a));
        }
        solver.add(0); //cerr << endl;
        
        // ∀x,a,b: make clause -(xa ^ xb) i.e. (-xa v -xb)
        //  where x variable; a,b in domain(x); a!=b
        for (int a = 0; a < wcsp.domsize[x]; a++) {
            for (int b = a+1; b < wcsp.domsize[x]; b++) {
                solver.add(-varVal2lit(x,a));
                solver.add(-varVal2lit(x,b));
                solver.add(0); //cerr << endl;
            }
        }
    }
    //cerr << endl << "--------------" << endl << endl;

    func2lit = vector<int>(wcsp.nfuncs);
    for (int f = 0; f < wcsp.nfuncs; f++) {
        // ∀f,c: make atom f_c
        //  where f function, c is cost of f, c > 0 
        func2lit[f] = lit_num;
        lit_num += wcsp.costs[f].size();

        // ∀f,t: make clause (-x1_a1 v -x2_a2 v ... v -xn_an v <block>),   
        //  where t is tuple <a1, a2, ..., an>(cost c) of function f(x1, x2, ..., xn)
        //      - if c == 0   : <block> = true   i.e. clause not included
        //      - if c == top : <block> = false  i.e. hard clause (<block> not included)
        //      - else        : <block> = f_c    i.e. soft clause (<block> assumption)
        const Function& func = wcsp.functions[f];
        for (int t = 0; t < func.numTuples(); t++) {
            Cost cost = func.getCost(t);
            if (cost > 0) {
                if (cost < wcsp.ub) solver.add(funcCost2lit(f, cost));
                vector<int> tuple = func.getTuple(t);
                for (int xf = 0; xf < func.arity(); xf++) {
                    int x = func.getScope()[xf];
                    int a = tuple[xf];
                    solver.add(-varVal2lit(x,a));
                }
                solver.add(0); //cerr << endl;
            }
        }
        //cerr << endl;
    }
}

// solve wcsp^h
// if sat   : return true  ,  sol is a solution,   i.e. wcsp^h(sol)=true
// if unsat : return false ,  C is a set of cores, i.e. C ⊆ {k | h ≤ k, wcsp^k unsat}
bool CSP_sat::solve(vector<int> h) {
    assert(h.size() == wcsp.nfuncs);

    C = vector<vector<int>>(0);
    //cout << "-";

    //vector<int> k;
    //while (not solve(h, k)) {
    int unsats = 1;
    while (unsats != 0) {
        vector<int> k;
        vector<int> h_ = h;
        unsats = 0;
        while (not solve(h_, k)) {  //solve wcsp^h_
            unsats++;
            int i = idx_min_Cost(k);
            h_ = k;
            if (h_[i] == wcsp.costs[i].size()) break;
            h_[i]++;
            // wcsp^k unsat, k >= old h_
            // we soften h_ so that h_ = (k1, k2, ..., ki+1, ... kn) 
            //      where i is the component mith minimal cost
        }
        if (unsats > 0) {
            C.push_back(k);
            for (int f = 0; f < wcsp.nfuncs; f++) {
                if (k[f] < wcsp.costs[f].size()) h[f] = wcsp.costs[f].size();
                // if  k[f] == inf : h[f] = h[f]
                // else            : h[f] = inf 
            }
            //cout << unsats << ".";
        }
    }
    //cout << "+";

    if (C.size() == 0) { //sat
        sol = vector<int>(wcsp.nvars);
        for (int x = 0; x < wcsp.nvars; x++) {
            int count = 0;
            for(int a = 0; a < wcsp.domsize[x]; a++) {
                if (solver.val(varVal2lit(x,a)) > 0) {
                    sol[x] = a; 
                    count++;
                }
            }
            assert(count == 1);
        }
        return true;
    }
    else { //unsat
        sol = vector<int>(0);
        return false;
    }
}

// solve wcsp^h
// if sat   : return true
// if unsat : return false; k is a core (i.e. wcsp^k unsat, h ≤ k)
bool CSP_sat::solve(const vector<int> &h, vector<int>& k) {
    for (int f = 0; f < wcsp.nfuncs; f++) {
        int num_costs = wcsp.costs[f].size();
        for (int c = 1; c < num_costs; c++) {
            int sign = (c <= h[f]) ? 1 : -1;
            solver.assume(sign*funcICost2lit(f, c));
        }
    }
    int r = solver.solve();
    assert(r == 10 or r == 20);
    bool sat = (r == 10);
    if (not sat) {
        k = vector<int>(wcsp.nfuncs);
        for (int f = 0; f < wcsp.nfuncs; f++) {
            k[f] = smallestFail(h, f);
        }
        assert(h <= k);
    }
    return sat;
}

int CSP_sat::varVal2lit(int var, int val) const {
    assert(0 <= val and val < wcsp.domsize[var]);
        //cerr << sign<<'('<<var<<','<<val<<')' << ' ';//debug
    return var2lit[var] + val;
}

int CSP_sat::funcICost2lit(int func, int idx_cost) const {
    assert(0 <= idx_cost and idx_cost < wcsp.costs[func].size());
    return func2lit[func] + idx_cost;
}

int CSP_sat::funcCost2lit(int func, Cost cost) const {
    //assert(cost > 0);
    int idx_cost = wcsp.cost2index(func, cost);
        //cerr << sign<<"[f"<<func<<",c"<<idx_cost<<":"<<cost<<"]" << ' ';//debug
    return funcICost2lit(func, idx_cost);
}


// PRE: solve(h) called & returned FALSE
// POST: given function index f, return cost index c s.t.:
//      c>=h[f], ∃k: wcsp^k unsat, k[f]=c
int CSP_sat::smallestFail(const vector<int>& h, int f) {
    int num_costs = wcsp.costs[f].size();
    for (int c = h[f]+1; c < num_costs; c++) {
        if (solver.failed(-funcICost2lit(f,c))) {
            return c-1;
        }
    }
    return num_costs;
}


/*int CSP_sat::idx_min_Cost(const vector<int>& k) {
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
*/



/*bool sat = false;
    while (not sat) {
        
        vector<int> k;
        sat = solve(h, k);
        if (not sat) {C.push_back(k); 
        for (int f = 0; f < wcsp.nfuncs; f++) {
            if (k[f] < wcsp.costs[f].size()) {h[f] = wcsp.costs[f].size();}
        }}*/
        /*vector<int> k;
        sat = solve(h, k);
        if (not sat) {
            for (int f = 0; f < wcsp.nfuncs; f++) {
                if (k[f] < wcsp.costs[f].size()) {
                    k[f]++;
                    vector<int> k2;
                    if (solve(k, k2)) k[f]--;
                    else k = k2;
                    h[f] = wcsp.costs[f].size();
                }
            }
            //cout << ".";
            C.push_back(k);
        }*/