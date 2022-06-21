#include "forward_checking.hh"

/* variable selection heuristic for forward checking */
int ForwardChecking::min_dom(const vector<int> &csp_assign, const vector<int> &csp_domsize) {

  int min = 1000000;
  int var_min = -1;
  for (int i = 0; i < wcsp.nvars; i++) {
    if (csp_assign[i] == -1 and csp_domsize[i] < min) {
      min = csp_domsize[i];
      var_min = i;
    }
  }
  // return var_min;

  vector<int> candidates;
  for (int i = 0; i < wcsp.nvars; i++) {
    if (csp_assign[i] == -1 and csp_domsize[i] == min)
      candidates.push_back(i);
  }
  random_shuffle(candidates.begin(), candidates.end());
  return candidates[0];
}

bool ForwardChecking::look_ahead(vector<int> &csp_assign, int var, int a, const vector<int> &h,
                vector<vector<int>> &csp_doms,
                vector<int> &csp_domsize, vector<pair<int, int>> &csp_changes,
                vector<Cost> &k, const vector<int> &mask) {

  int var2;
  for (int i = 0; i < wcsp.nfuncs; ++i) {
    if (wcsp.functions[i].inScope(var) and
        wcsp.functions[i].coversAssgButOne(csp_assign, var2) and
        csp_assign[var2] == -1) {
      int size_var2 = csp_doms[var2].size();
      vector<pair<int, Cost>> Q;
      for (int b = 0; b < csp_doms[var2].size(); ++b) {
        csp_assign[var2] = b;
        if (csp_doms[var2][b] == -1) {
          Cost c = wcsp.functions[i].getCostAssg(csp_assign);
          if (c == wcsp.ub or (mask[i] == 1 and c > wcsp.costs[i][h[i]])) {
            csp_doms[var2][b] = i;
            csp_domsize[var2]--;
            csp_changes.push_back(make_pair(var2, b));
            Q.push_back(make_pair(i, c));
            --size_var2;
          }
        } else {
          int idx2 = csp_doms[var2][b];
          Cost c = wcsp.functions[idx2].getCostAssg(csp_assign);
          Q.push_back(make_pair(idx2, c));
          --size_var2;
        }
        csp_assign[var2] = -1;
      }
      if (size_var2 == 0) {
        while (Q.size() > 0) {
          k[Q.back().first] = min(k[Q.back().first], Q.back().second);
          Q.pop_back();
        }
        return false;
      }
    }
  }
  return true;
}

bool ForwardChecking::forward_checking(vector<int> &csp_assign, int unassigned,
                      const vector<int> &h,
                      vector<vector<int>> &csp_doms, vector<int> &csp_domsize,
                      vector<Cost> &kcp, const vector<int> &mask) {

  if (unassigned == 0)
    return true;
  // cout<<unassigned<<endl;
  int var =
      min_dom(csp_assign, csp_domsize); // select next variable to assign
  for (int a = 0; a < wcsp.domsize[var]; ++a) {
    int idx_h = csp_doms[var][a];
    assert(idx_h >= -1 and idx_h < wcsp.nfuncs);
    csp_assign[var] = a;
    if (idx_h != -1) { // it has been pruned by constraint idx_h
      Cost c = wcsp.functions[idx_h].getCostAssg(csp_assign);
      kcp[idx_h] = min(kcp[idx_h], c); // updates core
    } else {
      vector<pair<int, int>> csp_changes;
      bool satisfiable = look_ahead(csp_assign, var, a, h, csp_doms,
                                    csp_domsize, csp_changes, kcp, mask) and
                         forward_checking(csp_assign, unassigned - 1, h,
                                          csp_doms, csp_domsize, kcp, mask);
      for (int i = 0; i < csp_changes.size(); ++i) {
        csp_doms[csp_changes[i].first][csp_changes[i].second] = -1;
        csp_domsize[csp_changes[i].first]++;
      }
      if (satisfiable) {
        csp_assign[var] = -1;
        return true;
      }
    }
    csp_assign[var] = -1;
  }
  return false;
}


// gets ready for a new call to forward forward_checking
// It prepares the doms and domsize variables
//  mask tells which constraints are desactivated (value 0)
//  assign tells which variables are desactivated (value -2)
bool ForwardChecking::ini_domains(const vector<int> &h,
                 const vector<int> &assign, vector<vector<int>> &doms,
                 vector<int> &domsize, const vector<int> &mask) {

  for (int i = 0; i < wcsp.nvars; i++)
    if (assign[i] == -1)
      domsize[i] = wcsp.domsize[i];
  for (int i = 0; i < wcsp.nvars; ++i)
    if (assign[i] == -1)
      for (int j = 0; j < doms[i].size(); ++j) {
        doms[i][j] = -1;
      }

  // prunes domains according with relaxation of unary cost functions in h
  for (int i = 0; i < wcsp.nfuncs; ++i) {
    if (wcsp.functions[i].arity() == 1) {
      int var = wcsp.functions[i].getScope()[0];
      if (assign[var] == -1) {
        for (int a = 0; a < wcsp.domsize[var]; ++a) {
          Cost c = wcsp.functions[i].getCost(vector<int>{a});
          if (c == wcsp.ub or (mask[i] == 1 and h[i] < wcsp.costs[i].size()
                               and c > wcsp.costs[i][h[i]])) {
            doms[var][a] = i;
            domsize[var]--;
            if (domsize[var] == 0) {
              return false;
            }
          }
        }
      }
    }
  }
  return true;
}

// computes in C a set of cores of de csp defined as wcsp^h
//  it does so by calling Forward checking untill the problem is satisfiable
//  After each unsatisfiable call (which returns a core k) it removes all the
//  constraints in the core and all the variables that become isolated
bool ForwardChecking::solve(vector<int> h) {

  //cout << wcsp.costs;
  //for (const auto& c : wcsp.costs) cout << c.size() << " ";
  //cout << endl << endl;

  // creates relevant variables
  vector<int> csp_assign(wcsp.nvars, -1);
  vector<int> csp_domsize(wcsp.nvars);
  vector<vector<int>> csp_doms(wcsp.nvars);
  for (int i = 0; i < wcsp.nvars; ++i) {
    csp_doms[i] = vector<int>(wcsp.domsize[i]);
  }
  vector<int> mask(wcsp.nfuncs); // 1 means active
  vector<Cost> k2(wcsp.nfuncs); // core codificado como el coste mínimo de poda
  for (int i = 0; i < k2.size(); ++i)
    k2[i] = wcsp.ub; // gets ready for first iteration

  // initializes variables for the first call
  int unassigned = wcsp.nvars; // number of unassigned variables,
  for (int i = 0; i < wcsp.nvars; i++)
    csp_assign[i] = -1; // -1 means unassigned, -2 means eliminated
  for (int i = 0; i < k2.size(); i++)
    mask[i] = 1;
  bool domainok = ini_domains(h, csp_assign, csp_doms, csp_domsize, mask);
  assert(domainok); // puesto que el problema es NC, no puede ser que la
                    // relajacion pode todos los valores de una variable
  //cout << "-";

  C = vector<vector<int>>();

  int unsats = 1;
  while (unsats > 0) {
    vector<int> k(wcsp.nfuncs);
    vector<int> h_ = h;
    unsats = 0;
    while (not forward_checking(csp_assign, unassigned, h_, csp_doms,
                                csp_domsize, k2, mask)) {
      unsats++;

      // reformulates and stores the core (k2) as vector of indexes (k)
      for (int i = 0; i < k.size(); i++) {
        k[i] = wcsp.cost2index(i, k2[i]);  // later: C.push_back(k-1)
      }
      assert(k >= h_);

      int i = idx_min_Cost(k);
      h_ = k;
      if (h_[i] == wcsp.costs[i].size()) break;
      h_[i]++;
          
      // prepare variables for next call
      for (int i = 0; i < h_.size(); i++)
        if (h_[i] >= wcsp.costs[i].size()) mask[i] = 0;
      for (int i = 0; i < k2.size(); ++i)
        k2[i] = wcsp.ub; // gets ready for next iteration
      domainok = ini_domains(h_, csp_assign, csp_doms, csp_domsize, mask);
      assert(domainok); // puesto que el problema es NC, no puede ser que la
                        // relajacion pode todos los valores de una variable

    }

    if (unsats > 0) {

      C.push_back(k-1);
      //cout << unsats << ".";

      // prepare variables for next call
      for (int i = 0; i < k2.size(); i++)
        if (k2[i] < wcsp.ub) {
          mask[i] = 0; // remove functions from previous core
          h[i] = wcsp.costs[i].size();
        }
      for (int i = 0; i < wcsp.nvars; i++) { // remove variables that have become isolated
        unassigned = 0;
        csp_assign[i] = -2;
        for (int j = 0; j < wcsp.var2functions[i].size(); j++) {
          int idx = wcsp.var2functions[i][j];
          if (wcsp.functions[idx].arity() > 1 and mask[idx] == 1) {
            csp_assign[i] = -1;
            unassigned++;
          }
        }
      }
      for (int i = 0; i < k2.size(); ++i)
        k2[i] = wcsp.ub; // gets ready for next iteration
      domainok = ini_domains(h, csp_assign, csp_doms, csp_domsize, mask);
      assert(domainok); // puesto que el problema es NC, no puede ser que la
                        // relajacion pode todos los valores de una variable
    }

    //cout << unassigned << ".";
  }
  //cout << "+";
  return C.size() == 0;
}
