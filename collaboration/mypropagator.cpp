#include "mypropagator.h"
#include "gecode/int.hh"
#include "gecode/minimodel.hh"

//=============================================================

class MySubProblem : public Gecode::Space {
protected:
    Gecode::IntVarArray vars;
    Gecode::IntVarArray util;
    int optvar;
public:
    MySubProblem() : vars(*this,n,1,n), util(*this,n,1,n) {
        optvar = 0;
        for (int i=0; i<n; i++) {
            Gecode::count(  *this, 
                            vars, vars[i], 
                            Gecode::IRT_EQ, 
                            util[i]);
        }
        branch(*this, vars, Gecode::INT_VAR_NONE(), 
                            Gecode::INT_VAL_MIN() );
    }
    //---------------------------------------------------------
    MySubProblem(MySubProblem& source) 
    : Gecode::Space(source) {
        vars.update(*this, source.vars);
        util.update(*this, source.util);
    }
    //---------------------------------------------------------
    void print() const {
        std::cout << vars << std::endl;
    }
    //---------------------------------------------------------
    virtual Gecode::Space* copy() {
        return new MySubProblem(*this);
    }
    //---------------------------------------------------------
    void setOptVar(int i) {
        optvar = i;
    }
    //---------------------------------------------------------
    virtual void constrain(const Gecode::Space& current) {
        const MySubProblem& candidate = 
            static_cast<const MySubProblem&>(current);
        Gecode::rel(*this, 
                    util[optvar], 
                    Gecode::IRT_GR, 
                    candidate.util[optvar]);
    }
    //---------------------------------------------------------
    void fixValue(int i,int val) {
        Gecode::rel(*this, vars[i], Gecode::IRT_EQ, val);
    }
    //------------------------------------------------------------
    void setPreference(int i, int val) {
        rel(*this, util[i], Gecode::IRT_GR, val);
    }
    //------------------------------------------------------------
    int getUtility(int i) {
        return util[i].val();
    }
};

//=============================================================

MyPropagator::MyPropagator(MyProblem& problem) : problem(problem) {
    bestus.growTo(n);
    for (int i=0; i<n; i++) {
        problem.vars[i]->attach(this, 0, EVENT_F );
    }
}

//-------------------------------------------------------------

bool MyPropagator::propagate()  {
    for (int i=0; i<n; i++) {
        if (!problem.vars[i]->isFixed()) return true;
    }

    if ( checkNash() == false ) {
        return false;
    }

    // vec<BoolView> clause(2);
    // clause[0] = newBoolVar();
    // int_rel_half_reif(problem.vars[0],IRT_NE,1,clause[0]);

    // clause[1] = newBoolVar();
    // int_rel_half_reif(problem.vars[1],IRT_NE,2,clause[1]);

    // bool_clause(clause);

    // if (problem.vars[0]->getMax() <= 1) {
        // int_rel(problem.vars[1], IRT_GT, 1);
        // return false;
    // }
    
    // Clause* r = Reason_new(2);
    // if (!problem.util[0]->setMin(3,r)) return false;

    // problem.addConstraint();

    // if (problem.util[2]->getMin()<2) {
    //     Clause* r = Reason_new(1);
    //     if (!problem.util[2]->setMin(2,r)) return false;
    // }

    return true;
}

//-------------------------------------------------------------

void MyPropagator::wakeup(int i, int) {
    pushInQueue();
}

//-------------------------------------------------------------

void MyPropagator:: clearPropState() { 
    in_queue = false; 
}

//-------------------------------------------------------------

bool MyPropagator::checkNash() { 
    for (int i=0; i<n; i++) {
        int currentutility = problem.util[i]->getVal();
        MySubProblem* submodel = new MySubProblem();
        for (int j=0; j<n; j++) {
            if (j==i) continue;
            submodel->fixValue(j, problem.vars[j]->getVal());
        }
        submodel->setOptVar(i);
        Gecode::BAB<MySubProblem> subengine(submodel);
        delete submodel;
        MySubProblem* best = nullptr;
        while (MySubProblem* better = subengine.next()) {
            if (best) delete best;
            best = better;
        }
        if (best) {
            int bestutility = best->getUtility(i);

            vec<BoolView> clause(n);
            for(int j=0; j<n; j++) {
                clause[j] = newBoolVar();

                if (j==i) {
                    int_rel_half_reif(  newIntVar(bestutility),
                                        IRT_LE,
                                        problem.util[i],
                                        clause[j]);

                }
                else {
                    int_rel_half_reif(  problem.vars[j],
                                        IRT_NE,
                                        problem.vars[j]->getVal(),
                                        clause[j]);
                }
            }
            bool_clause(clause);

            delete best;
            if (bestutility>currentutility) return false;
        }
    }
    return true; 
}