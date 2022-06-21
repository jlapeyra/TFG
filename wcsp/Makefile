# Option nodebug=1 ignores all asserts
# If a debug version is made (e.g. "make"), run "make clean" 
#   before making a nodebug version (e.g. "make nodebug=1").
# The seme applies in the opposite case.


main: main.o wcsp.o function.o MHV_cpx.o forward_checking.o csp_sat.o wcsp_solver.o
	$(CCC) $(CCFLAGS) -o main main.o wcsp.o function.o MHV_cpx.o forward_checking.o csp_sat.o wcsp_solver.o $(LIBCADICAL) $(CCLNFLAGS)

main.o: main.cc wcsp.hh function.hh MHV_cpx.hh forward_checking.hh wcsp_solver.hh utils.cc 
	$(CCC) $(CCFLAGS) -c main.cc 

wcsp_solver.o: wcsp_solver.hh wcsp_solver.cc
	$(CCC) $(CCFLAGS) -c wcsp_solver.cc

wcsp.o: wcsp.hh wcsp.cc
	$(CCC) $(CCFLAGS) -c wcsp.cc

function.o: function.hh function.cc
	$(CCC) $(CCFLAGS) -c function.cc

MHV_cpx.o: MHV_cpx.cc MHV_cpx.hh callback.hh
	$(CCC) $(CCFLAGS) -c MHV_cpx.cc

random_MHV: random_MHV.cc utils.cc MHV_cpx.o
	$(CCC) $(CCFLAGS) -o random_MHV  random_MHV.cc MHV_cpx.o $(CCLNFLAGS)

forward_checking.o: forward_checking.cc forward_checking.hh csp.hh
	$(CCC) $(CCFLAGS) -c forward_checking.cc

CADICAL = sat-cadical
LIBCADICAL = $(CADICAL)/build/libcadical.a

csp_main: csp_main.cc  csp_sat.o forward_checking.o  wcsp.o function.o  $(LIBCADICAL) 
	$(CCC) $(CCFLAGS) -o csp_main  csp_main.cc csp_sat.o forward_checking.o wcsp.o function.o  $(LIBCADICAL) $(CCLNFLAGS)

csp_sat.o: csp_sat.hh csp_sat.cc $(LIBCADICAL) csp.hh
	$(CCC) $(CCFLAGS) -c csp_sat.cc 

$(LIBCADICAL): $(CADICAL)/src/*.hpp $(CADICAL)/src/*.cpp $(CADICAL)/src/
	cd $(CADICAL); pwd; ./configure && make

clean:
	rm -f  *.o main csp_main random_MHV

clean_cadical:
	cd $(CADICAL); make clean

################################

SYSTEM     = x86-64_linux
LIBFORMAT  = static_pic
CPLEXDIR      = /opt/ibm/ILOG/CPLEX_Studio201/cplex
CONCERTDIR    = /opt/ibm/ILOG/CPLEX_Studio201/concert
BOOSTDIR   = /usr/local

################

CCC = g++
LDFLAGS = #-g

ifdef nodebug
	DEBUGFLAG = -DNDEBUG
else
	DEBUGFLAG =
endif

#####

CCOPT = -std=c++11 -m64 -O3 -fPIC -fno-strict-aliasing -fexceptions  -DIL_STD  -D_GLIBCXX_DEBUG #-DNDEBUG

CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
BOOSTLIBDIR   = $(BOOSTDIR)/lib

CCLNFLAGS = -L$(CPLEXLIBDIR) -lilocplex -lcplex -L$(CONCERTLIBDIR) -lconcert -lm -pthread -ldl 

CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include
BOOSTINCDIR   = $(BOOSTDIR)/include

CCFLAGS = $(DEBUGFLAG) $(CCOPT) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR) -I$(BOOSTINCDIR)

