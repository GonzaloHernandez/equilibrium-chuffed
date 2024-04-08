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

//-------------------------------------------------------------

#include "mypropagator.h"
#include "gecode/int.hh"

class Equilibrium : public Gecode::Space {
protected:
    Gecode::IntVarArray vars;
    Gecode::IntVarArray util;
    int optvar;
public:
    Equilibrium() : vars(*this,n,1,n), util(*this,n,1,n) {
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
    Equilibrium(Equilibrium& source) 
    : Gecode::Space(source) {
        vars.update(*this, source.vars);
        util.update(*this, source.util);
        optvar = source.optvar;
    }
    //---------------------------------------------------------
    void print() const {
        std::cout << vars << std::endl;
    }
    //---------------------------------------------------------
    virtual Gecode::Space* copy() {
        return new Equilibrium(*this);
    }
    //---------------------------------------------------------
    void setOptVar(int i) {
        optvar = i;
    }
    //---------------------------------------------------------
    virtual void constrain(const Gecode::Space& current) {
        const Equilibrium& candidate = 
            static_cast<const Equilibrium&>(current);
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

    std::cout << "++ " << problem.toStr() << std::endl;
    for (int i=0; i<n; i++) {
        if (!problem.vars[i]->isFixed()) return true;
    }

    if ( checkNash() == false ) {
        return false;
    }

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
        Equilibrium* submodel = new Equilibrium();
        for (int j=0; j<n; j++) {
            if (j==i) continue;
            submodel->fixValue(j, problem.vars[j]->getVal());
        }

        submodel->setOptVar(i);
        Gecode::BAB<Equilibrium> subengine(submodel);
        delete submodel;
        Equilibrium* best = nullptr;
        while (Equilibrium* better = subengine.next()) {
            if (best) delete best;
            best = better;
        }
        if (best) {
            int bestutility = best->getUtility(i);
            delete best;

            if (bestutility>currentutility) {
                if (so.lazy) {
                    Clause* r = Reason_new(n+1);

                    for (int j=0; j<n; j++) {
                        if (j==i) {
                            (*r)[j+1] = problem.util[j]->getMinLit();
                        }
                        else {
                            (*r)[j+1] = problem.vars[j]->getMinLit();
                        }
                    }

                    problem.util[i]->setMin(bestutility,r);
                }

                return false;
            }
        }
    }
    return true; 
}

#endif // MYPROPAGATOR_H