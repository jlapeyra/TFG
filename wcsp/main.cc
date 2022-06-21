#include "wcsp_solver.hh"
#include <ctime>

void usage(string arg0) {
    cout << "USAGE:" << endl;
    cout << "" << arg0 << " file.wcsp [-s] [-c sat|fc] [-a lb|lub] [-v greedy|model|callback] [-h]" << endl;
    cout << "    file.wcsp : input WCSP file" << endl;
    cout << "    -s|--solution : print optimal solution" << endl;
    cout << "    -c|--csp-solver   sat|fc                (default: sat)" << endl;
    cout << "    -a|--alg-version  lb|lub                (default: lub)" << endl;
    cout << "    -v|--hv-method    greedy|model|callback (default: greedy)" << endl;
    cout << "        --hv_method only makes sense when --alg-version lub" << endl;
    cout << "    -h|--help : this message is printed" << endl;
   // cout << ""
   // cout << "\t\t\t if hv_version = lub, hv_method must be given" << endl;

    exit(1);
}

#define a0 argv[0]

int main(int argc, char const *argv[]) {
    // cout << "File: " << argv[1] << endl;

    if (argc <= 1) usage(a0);

    string filename = "";
    bool print_sol = false;
    string csp_solver = "sat";
    string alg_version = "lub";
    string hv_method = "greedy";

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-s" or arg == "--solution") {
            print_sol = true;
        }
        else if (arg == "-c" or arg == "--csp-solver") {
            i++;
            arg = argv[i];
            if (arg != "sat" and arg != "fc") {
                cout << "ERROR: csp-solver must be 'sat' or 'fc', but '"<<arg<<"' given" << endl;
                usage(a0);
            }
            csp_solver = arg;
        }
        else if (arg == "-a" or arg == "--alg-version") {
            i++;
            arg = argv[i];
            if (arg != "lb" and arg != "lub") {
                cout << "ERROR: alg-version must be 'lb' or 'lub', but '"<<arg<<"' given" << endl;
                usage(a0);
            }
            alg_version = arg;
        }
        else if (arg == "-v" or arg == "--hv-method") {
            i++;
            arg = argv[i];
            if (arg != "greedy" and arg != "model" and arg != "callback") {
                cout << "ERROR: hv-method must be 'greedy' or 'model' or 'callback', but '"<<arg<<"' given" << endl;
                usage(a0);
            }
            hv_method = arg;
        }
        else if (arg == "-h" or arg == "--help") {
            usage(a0);
        }
        else if (arg[0] == '-') {
            cout << "ERROR: invalid option "<<arg<<endl;
            usage(a0);
        }
        else if (filename == "") {
            filename = arg;
            if (filename.substr(filename.size()-5, filename.size()) != ".wcsp") 
                cout << "WARNING: input file "<<arg<<" does not have extension .wcsp" << endl;
        }
        else {
            cout << "ERROR: 1 positional argument expected (file.wcsp), but more than 1 given" << endl;
            usage(a0);
        }
    }
    if (alg_version == "lb") hv_method = "na";
    if (filename == "") {
        cout << "file.wcsp must be given" << endl;
        usage(a0);
    }

    cout << filename << endl;
    cout << "Options: " << csp_solver << ", " << alg_version << ", " << hv_method << endl;


    Wcsp wcsp;
    wcsp.read(filename);
    wcsp.show(0);

    auto start = high_resolution_clock::now();
    auto cpu_start = clock();

    WcspSolver solver(wcsp, csp_solver, alg_version, hv_method);
    Cost optimal = solver.solve();
    
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    double real_time = duration.count() / 1000000.0;

    auto cpu_end = clock();
    double cpu_time = double(cpu_end-cpu_start) / CLOCKS_PER_SEC;

    if (print_sol) {
        cout << endl;
        cout << "optimal solution: (variable values)" << endl;
        for (int x : solver.getSolution()) {
            cout << x << " ";
        }
        cout << endl << endl;
    }

    cout << "Optimal cost: " << optimal << endl;
    cout << "Real time: " << real_time << endl;
    cout << "CPU time: " << cpu_time << endl;
}


//#include <boost/program_options.hpp>
//using namespace boost::program_options;
//namespace po = boost::program_options;

/*if (argc <= 3 or string(argv[1]) == "--help"
        or not (string(argv[2])=="sat" or string(argv[2])=="fc")
        or not (string(argv[3])=="lb"  or string(argv[3])=="lub")
        or (string(argv[3])=="lub" and (argc <= 4 
            or not (string(argv[4])=="greedy" or string(argv[4])=="model" or string(argv[4])=="callback")))
        )
    {
        cout << "USAGE:" << endl;
        cout << "\t" << argv[0] << " file.wcsp csp_solver alg_version [hv_method]" << endl;
        cout << "\t\t csp_solver = sat|fc" << endl;
        cout << "\t\t alg_version = lb|lub" << endl;
        cout << "\t\t hv_method = greedy|model|callback" << endl;
        cout << "\t\t\t if hv_version = lub, hv_method must be given" << endl;
        exit(1);
    }
    string csp_solver = argv[2];
    string alg_version = argv[3];
    string hv_method;
    if (alg_version == "lub") hv_method = argv[4];
    else                      hv_method = "na";
    */
