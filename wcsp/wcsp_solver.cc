#include "wcsp_solver.hh"

// tells if k1 is dominated by k2 or, what is the same, if k1 does not hit k2
bool WcspSolver::dominated(const vector<int> &k1, const vector<int> &k2) { // k1<= k2 ?
  return k1 <= k2;
  /*for (int i = 0; i < k1.size(); i++)
    if (k1[i] > k2[i])
      return false;
  return true;*/
}

// adds k to K2 and removes from K2 all dominated vectors
void WcspSolver::add_core(vector<vector<int>> &K2, const vector<int> &k) {
  int i = 0;
  while (i < K2.size()) {
    if (dominated(K2[i], k))
      K2.erase(K2.begin() + i);
    else
      ++i;
  }
  K2.push_back(k); // push_back copia k
}

// tells if h hits all the cores in K2
bool WcspSolver::hits(const vector<int> &h, const vector<vector<int>> &K2) {
  for (int i = 0; i < K2.size(); ++i)
    if (dominated(h, K2[i]))
      return false;
  return true;
}

// K += C;  K2 += C;  C = []
void WcspSolver::extend_K(vector<vector<int>>& C) {
  while (C.size() != 0) {
    K.addCore(C.back());
    add_core(K2, C.back());
    C.pop_back();
  }
}

// h = MHV(K)
void WcspSolver::solve_MHV(vector<int>& h, long& time) {
  //if (hits(..., K2)) { cout << "lucky you! "; return; // The previous h is still valid }
  if (K.solve_MHV(time)) {
    h = K.getMHV_idom();
    assert(wcsp.vector_cost(h) == K.getCost_MHV());
  }
  else {
    cout << "MHV fail: Unfeasible set of cores" << endl;
    //cout << K2 << endl;
    exit(EXIT_FAILURE);
  }
}

// h = HV(K)
void WcspSolver::solve_HV(vector<int>& h, long& time, Cost ub) {
  if (K.solve_HV(ub, time)) {
    h = K.getHV_idom();
    assert(wcsp.vector_cost(h) == K.getCost_HV());
  }
  else {
    cout << "HV fail: Unfeasible set of cores" << endl;
    //cout << K2 << endl;
    exit(EXIT_FAILURE);
  }
}

/*
PRE: csp_solver  = "sat"|"fc"
PRE: alg_version = "lb"|"lub"
PRE: if (alg_version = "lub") then hv_method = "greedy"|"model"
*/
WcspSolver::WcspSolver(const Wcsp &wcsp, string csp_solver="sat", string alg_version="lb", string hv_method = "na") 
        : wcsp(wcsp), alg_version(alg_version), hv_method(hv_method)
{
  if (csp_solver == "fc") csp = new ForwardChecking(wcsp);
  else /* "sat" */        csp = new CSP_sat(wcsp);
}

Cost WcspSolver::solve() {
  int iteration = 0;
  long time_solver = 0;
  long time_mhv = 0;
  int ncores = 0;

  vector<int> h(wcsp.nfuncs, 0); // hitting vector

  K2 = vector<vector<int>>();

  if (alg_version == "lb") { // algorithm version with lb
    cout << "Iteration <i>   lb <lb> ub <ub>   cores <found> <independent> <new>   time <csp> <mhv>" << endl;

    K = MHV_cplex(vector<vector<int>>(0), wcsp.costs, "na");

    while (not csp->solve(h, time_solver)) {
      vector<vector<int>>& C = csp->getCores();
      int n_new_cores = C.size();
      ncores += C.size();

      extend_K(C); // K += C
      solve_MHV(h, time_mhv); // h = MHV()
      //lb == cost(h) == K.getCost_MHV()
      
      cout << "Iteration " << iteration++ 
          << "  lb " << wcsp.lb + K.getCost_MHV() << " ub " << wcsp.lb + wcsp.ub
          << "  cores " << ncores << " " << K2.size() << " " << n_new_cores 
          << "  time " << time_solver / 1000000.0 << " " << time_mhv / 1000000.0
          << endl;
    }
    return wcsp.lb + K.getCost_MHV();
  }

  else { // algorithm version with lb & ub
    cout << "Iteration <i>  (MHV  lb <lb> ub <ub>) | (HV  cores <found> <independent> <new>)  time <csp> <mhv> <hv>" << endl;
    
    K = MHV_cplex(vector<vector<int>>(0), wcsp.costs, hv_method);
    Cost ub = wcsp.ub;
    Cost lb = 0;
    long time_hv = 0;
    while (lb < ub) {
      if (csp->solve(h, time_solver)) {

        ub = min(ub, wcsp.vector_cost(h));
        solve_MHV(h, time_mhv); // h = MHV()
        lb = max(lb, wcsp.vector_cost(h));

        cout << "Iteration " << iteration++ << " MHV "
          << " lb " << wcsp.lb + lb << " ub " << wcsp.lb + ub
          << "  time " << time_solver / 1000000.0 << " " << time_mhv / 1000000.0 << " " << time_hv / 1000000.0
          << endl;

        //cout << "\nMHV: lb " << lb << " ub " << ub << endl;
      }
      else {
        vector<vector<int>>& C = csp->getCores();
        ncores += C.size();
        int n_new_cores = C.size();

        extend_K(C);
        solve_HV(h, time_hv, ub);

        cout << "Iteration " << iteration++ << "  HV "
          << " cores " << ncores << " " << K2.size() << " " << n_new_cores 
          << "  time " << time_solver / 1000000.0 << " " << time_mhv / 1000000.0 << " " << time_hv / 1000000.0
          << endl;
        
        //cout << endl << "HV" << endl;
      }

      //cout << K2  << " " << h << endl;
    }
    return wcsp.lb + lb;
  }
}
