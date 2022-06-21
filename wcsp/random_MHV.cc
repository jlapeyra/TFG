#ifndef RANDOM_MHV
#define RANDOM_MHV

#include "utils.cc"
#include "wcsp.hh"
#include "MHV_cpx.hh"
#include <stdlib.h>
#include <time.h>
#include <stdexcept>
using namespace std;

void pop(vector<int>& v, int idx) {
    for (int i = idx; i < v.size()-1; ++i) {
        v[i] = v[i+1];
    }
    v.pop_back();
}

vector<vector<int>> randomMHV(int vecs, int comps, int size_domain, float rate_unhittable, int seed)
/**
 * If successful, it generates a sequence of `vecs` vectors with `comps` components 
 * indexing the domain (indexs range: 0 .. size_domain-1)
 *  - Each vector has `comps*rate_unhittable` unhittable values (rounded to lower integer).
 *  - There is no pair of vectors `v1`, `v2` such that `v1 <= v2` i.e. `v2` dominates `v1`.
 * An unhittable value is the larges value of its domain.
 * 
 * Pre:
 *  - 0 < vecs, comps, size_domain
 *  - 0 <= rate_unhittable < 1
 * 
 * If not successful, throw exception RANDOM_NOT_POSSIBLE
 */
{    
    srand(seed);
    //srand(time(NULL)); //"random" seed
    
    vector<vector<int>> K;

    int unhittable = (size_domain-1);
    int num_unhittable = int(rate_unhittable*comps);
    vector<int> idx_comps(comps);
    for (int j = 0; j < comps; ++j) {
        idx_comps[j] = j;
    }

    bool acceptable ; // (¬∃ v1,v2 s.t. v1>=v2)
    int attempts = 0;
    do {
        K = vector<vector<int>>(vecs, vector<int>(comps, 0));
        acceptable = true;
        for (int i = 0; i < vecs and acceptable; ++i) {
            int attempts_row = 0;
            do {
                auto idx_comps_undef = idx_comps;
                for (int r = 0; r < num_unhittable; ++r) {
                    int random = rand()%idx_comps_undef.size();
                    int j = idx_comps_undef[random];
                    pop(idx_comps_undef, random);
                    K[i][j] = unhittable; //assignació d'unhittables
                }
                for (int j : idx_comps_undef) {
                    K[i][j] = rand()%(size_domain-1); //assignació d'unhittables
                }
                for (int ii = 0; ii < i and acceptable; ++ii) {
                    if (K[i] >= K[ii] or K[i] <= K[ii]) { // comprovació que no es dominen
                        acceptable = false;
                        K[i] = vector<int>(comps, 0);
                    }
                }
                attempts_row++;
            } while (not acceptable and attempts_row < 50);
        }
        attempts++;
    } while (not acceptable and attempts < 50);

    if (not acceptable) {
        //cerr << "ERROR: Couldn't find any set of vectors without dominations among them.\n";
        throw invalid_argument("ERROR: Couldn't find any set of vectors without dominations among them.\n");
    }

    return K;
}

vector<vector<Cost>> simpleDomains(int comps, int domsize, bool quadratic) {
    vector<vector<Cost>> domains(comps, vector<Cost>(domsize));
    for(int i = 0; i < comps; i++) {
        for (int di = 0; di < domsize; di++) {
            if (not quadratic)
                domains[i][di] = di;
            else
                domains[i][di] = di*di;
        }
    }
    return domains;
}


int main(int argc, char const *argv[])
{
    if (argc < 6) {
        cout << "USAGE: ./random_MHV comps vecs domsize domtype rate_unhittable seed" << endl;
        cout << "   domtype = lin|quad" << endl;
        exit(1);
    }
    int comps = stoi(argv[1]);
    int vecs = stoi(argv[2]);
    int domsize = stoi(argv[3]);
    bool dom_quadratic = argv[4][0] == 'q';  //domtype: linear (0,1,2...) or quadratic (0²,1²,2²...)
    float rate_unhittable = stof(argv[5]);
    int seed;
    if (argc == 6) seed = time(NULL);
    else           seed = stoi(argv[6]);
    cout << "seed = " << seed << endl;

    vector<vector<Cost>> costs = simpleDomains(comps, domsize, dom_quadratic);
    vector<vector<int>> K = randomMHV(vecs, comps, domsize, rate_unhittable, seed);

    cout << K << endl;
    cout << "costs: " << costs[0] << endl << endl;

    MHV_cplex solver(K, costs, "greedy");
    long time;

    bool factible = solver.solve_HV(10, time);
    if (factible) {
        auto hv = solver.getHV_idom();
        cout << "SOLVED" << endl;
        assert(solver.isHV(hv));
        cout << "OK" << endl;
        cout << "hv = " << hv << endl;
        cout << "cost hv = " << solver.getCost_HV() << endl;
        cout << endl;
    }
    else {
        cout << "no factible" << endl;
    }


}

    




#endif