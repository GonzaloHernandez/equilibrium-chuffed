#ifndef MYPROBLEM_H
#define MYPROBLEM_H

#include "chuffed/vars/modelling.h"

int n = 3;

//---------------------------------------------------------

class MyProblem : public Problem {
private:
    vec<IntVar*>    vars;
    vec<IntVar*>    util;
    friend class MyPropagator;
public:
    MyProblem();
    void print(std::ostream&) override;
    void addConstraint();
    std::string toStr();
};

//---------------------------------------------------------

#include "myproblem.cpp"

#endif // MYPROBLEM_H