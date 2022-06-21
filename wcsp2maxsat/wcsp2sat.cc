// -*- Mode: c++; c-basic-offset: 4 -*-

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <tuple>
#include <map>
#include <unordered_map>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <numeric>

using namespace std;
using boost::format;

const bool debug = false;

typedef long long Cost;

struct wcsptuple {
    vector<size_t> tup;
    Cost cost;

    wcsptuple(vector<size_t> const &t, Cost c) : tup(t), cost(c) {}
};

struct wcspfunc {
    Cost defcost;
    vector<size_t> scope;
    vector<wcsptuple> specs;

    size_t arity() const { return scope.size(); }
};

struct wcsp {
    Cost ub;
    vector<size_t> domains;    
    vector<size_t> offset; // desplazamiento para calcular id var bool
    vector<wcspfunc> functions;

    size_t nvars() const { return domains.size(); }
};

struct wclause {
    vector<int> c;
    Cost w;
};


template<typename T>
vector<T> read_vec(istream& is)
{
    vector<T> r;
    T s;
    is >> s;
    while(is) {
        r.push_back(s);
        is >> s;
    }
    return r;
}

template<typename T>
vector<T> read_vec(string const& line)
{
    istringstream iss(line);
    return read_vec<T>(iss);
}

tuple<string, size_t, size_t, size_t, Cost> read_header(string const& line)
{
    istringstream iss(line);

    string name;
    size_t nvars;
    size_t domsize;
    size_t nfun;
    Cost ub;

    iss >> name >> nvars >> domsize >> nfun >> ub;
    return make_tuple(name, nvars, domsize, nfun, ub);
}


wcspfunc read_fun(istream& is)
{
    string line;
    getline(is, line);
    vector<Cost> hd = read_vec<Cost>(line);
    size_t arity = hd[0];
    Cost defcost = hd[hd.size()-2];
    size_t nspec = hd[hd.size()-1];

    vector<wcsptuple> specs;
    for(size_t i = 0; i != nspec; ++i) {
        getline(is, line);
        vector<Cost> v = read_vec<Cost>(line);
        specs.push_back( {vector<size_t>(v.begin(), v.begin()+arity), v[v.size()-1]} );
    }

    return { defcost, vector<size_t>(hd.begin()+1, hd.begin()+1+arity),
            specs };
}


void write_clause(ostream& os, wclause const& cl)  // <--- cambiado
{
    //++c.numclauses;
    os << cl.w << ' ';
    for (int l : cl.c) {
        //if (debug) {
        //    if (l < 0) os << '-';
        //    string const& n = c.rv.find(abs(l))->second;
        //    os << n << ' ';
        //}
        //else 
        os << l << ' ';
    }
    os << "0\n";
}


namespace std {
    template<typename T>
    ostream& operator<<(ostream& os, vector<T> const& v)
    {
        os << '[';
        bool comma = false;
        for(auto& e: v) {
            if(comma)
                os << ',';
            comma = true;
            os << e;
        }
        os << ']';
        return os;
    }
}

int dvar(const wcsp& w, int v, int d) // <---- cambiado
{
    //return var(w, (format("x%s_%s") % v % d).str(), w.v);
    if (w.domains[v] <= 2) return w.offset[v] + 1;
    else return w.offset[v] + 1 + d;
}


void write_domains(ostream& os, wcsp const& w) //, wcnf& c)  // <--- cambiado
{
    for (size_t i = 0; i != w.nvars(); ++i) {
        if (w.domains[i] == 1) {
            // we treat unary domain as a special case of binary
            //int bvar = dvar(c, i, 1);
            //c.v[(format("x%s_%s") % i % 0).str()] = -bvar;
            //if( w.domains[i] == 1 )
            write_clause(os, {{ -dvar(w, i, 0) }, w.ub });
        } else if (w.domains[i] > 2) {
            wclause pos{ {}, w.ub };
            for (size_t j1 = 0; j1 < w.domains[i]; ++j1) {
                pos.c.push_back(dvar(w, i, j1));
                for(size_t j2 = j1+1; j2 < w.domains[i]; ++j2) {  // at-most-one constraints
                    write_clause(os, {{-dvar(w, i, j1), -dvar(w, i, j2)}, w.ub}); 
                }
            }
            write_clause(os, pos);  // at-least-one constraint
        }
    }
}


// write direct encoding of a single cost function. Also used by the
// tuple encoding for unary cost functions.
template<typename P>
void foreach_tuple(wcsp const &w, wcspfunc const &f,
                   vector<size_t>& current, P proc)
{
    if (current.size() == f.arity()) {
        proc(current);
        return;
    }
    int idx = current.size();
    current.push_back(0);
    int var = f.scope[idx];
    for (size_t j = 0; j != w.domains[var]; ++j) {
        current[idx] = j;
        foreach_tuple(w, f, current, proc);
    }
    current.pop_back();
}


int write_cf_clauses(ostream& os,
                      wcsp const &w, wcspfunc const &f,
                      map< vector<size_t>, Cost > & fexplicit,
                      vector<size_t>& current)
{
    int nc = 0;
    foreach_tuple(w, f, current, [&](vector<size_t>& current) {
            auto i = fexplicit.find(current);
            Cost tcost;
            if (i != fexplicit.end()) tcost = min(i->second, w.ub);
            else tcost = min(f.defcost, w.ub);
            if (tcost == 0) return;
            wclause cl{ {}, tcost };
            for (size_t j = 0; j != current.size(); ++j) {
            	if (w.domains[f.scope[j]] <= 2 and current[j] == 0)
	                cl.c.push_back(dvar(w, f.scope[j], current[j]));
                else cl.c.push_back(-dvar(w, f.scope[j], current[j]));
            }
            write_clause(os, cl);
            ++nc;
            return;
        });
    return nc;
}

int write_function(wcspfunc const& f, wcsp /*const*/& w, ostream& os) {
    // convert functions
    //for(auto& f : w.functions) {
    if (f.arity() == 0) {
	cerr << "c WARNING: 0-arity functions (cost: " << f.defcost << " )" << endl;
	w.ub = w.ub - f.defcost;
            //write_clause(os, c, { {}, f.defcost });
            //continue;
        return 0;
    }
    if (f.specs.size() == 0) {
    	cerr << "c WARNING: function with default value for all its tuples (cost: " << f.defcost << " )" << endl;
	w.ub = w.ub - f.defcost;
	return 0;
    }
    int nc = 0; /// yo
    vector<size_t> current;
    map< vector<size_t>, Cost > tups;
    for (auto& tuple : f.specs) tups[tuple.tup] = tuple.cost;
        //current.clear();
    nc += write_cf_clauses(os, w, f, tups, current);
    assert(current.empty());
    //}
    return nc;
}

int main(int argc, char* argv[])
{
    namespace po = boost::program_options;

    string encoding;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("encoding", po::value<string>(&encoding)->default_value("tuple"),
                "use direct/tuple encoding")
        ("input-file,i", po::value<string>(), "wcsp input file")
        ("output-file,o", po::value<string>(), "wcsp output file")
        ;

    po::positional_options_description p;
    p.add("input-file", 1).add("output-file", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
              options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }

    bool tenc = false;   // de momento sólo direct
    /*if (encoding == "direct" )
        tenc = false;
    */

    if (!vm.count("input-file")) {
        cout << "must specify input file\n";
        return 1;
    }

    ifstream ifs(vm["input-file"].as<string>());
    if (!ifs) {
        cout << "could not open " << argv[1] << "\n";
        return 1;
    }

    ofstream rofs;
    ostream *ofs;
    if (vm.count("output-file")) {
        rofs.open(vm["output-file"].as<string>());
        if( !rofs ) {
            cout << "could not open " << argv[2] << "\n";
            return 1;
        }
        ofs = &rofs;
    } else
        ofs = &cout;

    // traducir:
    
    wcsp w;

    string name;
    size_t nvars;
    size_t domsize;
    size_t nfun;

    string line;

    getline(ifs, line);
    tie(name, nvars, domsize, nfun, w.ub) = read_header(line);

    getline(ifs, line);
    w.domains = read_vec<size_t>(line);
    w.offset = vector<size_t>(w.domains.size(), 0);
    for (size_t i = 1; i < w.offset.size(); ++i) {
        if (w.domains[i - 1] > 2) w.offset[i] = w.offset[i - 1] + w.domains[i - 1];
        else w.offset[i] = w.offset[i - 1] + 1;
    }

    int nv = accumulate(w.domains.begin(), w.domains.end(), 0,
                        [&](int x, int d) {
                            if (d > 2)
                                return x + d;
                            else
                                return x + 1;
                        });

    int nc = accumulate(w.domains.begin(), w.domains.end(), 0,
                        [](int x, int d) -> int {
                            if (d > 2)
                                return x + d*(d-1)/2 + 1;
                            else if (d == 1)
                                return x + 1;
                            else
                                return x;
                        });
                        
    // convert all domains
    write_domains(*ofs, w);

    for (size_t i = 0; i != nfun; ++i) {
        wcspfunc f = read_fun(ifs); // w.functions.push_back(read_fun(is));
        nc += write_function(f, w, *ofs);
    }

    *ofs << "p wcnf " << nv << " " << nc << " " << w.ub << endl;
}
