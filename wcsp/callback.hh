#ifndef CALLBACK_HH
#define CALLBACK_HH

#include "utils.cc"

#include <ilcplex/ilocplex.h>
ILOSTLBEGIN;

class UBCallback : public IloCplex::MIPInfoCallbackI {

public:
    UBCallback(IloEnv env) : IloCplex::MIPInfoCallbackI(env) {}

    CallbackI* duplicateCallback () const {
        return (new (getEnv()) UBCallback(*this));
    }

    bool active;
    Cost ub;

    void main() {
        if (active) {
            Cost obj = getIncumbentObjValue();
            if (obj < ub) abort();
        }
    }
};

#endif