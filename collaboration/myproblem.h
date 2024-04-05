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

#include "mypropagator.h"


MyProblem::MyProblem() {
    createVars(vars,n,1,n);
    createVars(util,n,1,n);

    for (int i=0; i<n; i++) {
        vec<BoolView> checks(n);
        for (int j=0; j<n; j++) {
            checks[j] = newBoolVar();
            int_rel_reif(vars[j],IRT_EQ,vars[i],checks[j]);
        }
        bool_linear(checks,IRT_EQ,util[i]);
    }

    new MyPropagator(*this);

    branch(vars, VAR_INORDER, VAL_MIN);
    branch(util, VAR_INORDER, VAL_MIN);
    output_vars(vars);
    output_vars(util);
}

//-------------------------------------------------------------

void MyProblem::addConstraint() {
    vec<BoolView> clause(2);
    clause[0] = newBoolVar();
    int_rel_half_reif(vars[0],IRT_NE,1,clause[0]);

    clause[1] = newBoolVar();
    int_rel_half_reif(vars[1],IRT_NE,2,clause[1]);

    bool_clause(clause);

    // int i=0;
    // MyProblem& problem = *this;

    // vec<BoolView> clause(n);
    // for(int j=0; j<n; j++) {
    //     clause[j] = newBoolVar();

    //     if (j==i) {
    //         int_rel_half_reif(  newIntVar(3),
    //                             IRT_GE,
    //                             problem.util[i],
    //                             clause[j]);

    //     }
    //     else {
    //         int_rel_half_reif(  problem.vars[j],
    //                             IRT_NE,
    //                             2,
    //                             clause[j]);
    //     }
    // }
    // bool_clause(clause);
}

//-------------------------------------------------------------

std::string MyProblem::toStr() {
    std::stringstream os;
    print(os);
    return os.str();
}  

//-------------------------------------------------------------

void MyProblem::print(std::ostream& out) {
    out << "[ ";
    for (int i=0; i<vars.size(); i++) {
    if (vars[i]->getMin() == vars[i]->getMax())
            out << vars[i]->getMin() << " "; 
        else
            out << "{" << vars[i]->getMin() 
                << ".." << vars[i]->getMax() 
                << "} "; 
    }
    out << "] [ ";
    for (int i=0; i<util.size(); i++) {
        if (util[i]->getMin() == util[i]->getMax())
            out << util[i]->getMin() << " "; 
        else
            out << "{" << util[i]->getMin() 
                << ".." << util[i]->getMax() 
                << "} "; 
    }
    out << "]";
}

//-------------------------------------------------------------

#endif // MYPROBLEM_H