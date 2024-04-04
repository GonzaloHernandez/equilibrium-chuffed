#ifndef MYPROPAGATOR_H
#define MYPROPAGATOR_H

#include "myproblem.h"
#include "chuffed/core/propagator.h"

//---------------------------------------------------------

class MyPropagator : public Propagator {
private:
    MyProblem&      problem;
    vec<vec<int>>   bestus;
public :
    MyPropagator(MyProblem&);
    bool propagate()            override;
    void wakeup(int i, int)     override;
    void clearPropState()       override;
    bool checkNash();
};

//---------------------------------------------------------

#include "mypropagator.cpp"

#endif // MYPROPAGATOR_H