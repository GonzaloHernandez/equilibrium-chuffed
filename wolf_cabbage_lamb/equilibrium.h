#ifndef MYPROPAGATOR_H
#define MYPROPAGATOR_H

#include "chuffed/core/propagator.h"
#include "mainmodel.h"

class Equilibrium : public Propagator {
private:
    MainProblem&    problem;
public :
    Equilibrium(MainProblem&);
    bool propagate();
    void wakeup(int i, int);
    void clearPropState();
    bool checkNash();
};

#include "gecode/int.hh"
#include "gecode/minimodel.hh"

class SubModel : public Gecode::Space {
protected:
    Gecode::IntVarArray vars;
    Gecode::IntVarArray util;
    int optvar;
public:
    SubModel() : vars(*this,3,0,1), util(*this,3,0,1) {
        optvar = 0;

        Gecode::BoolVar w  = Gecode::expr(*this, vars[0] == 1);  // Wolf
        Gecode::BoolVar c  = Gecode::expr(*this, vars[1] == 1);  // Cabbage
        Gecode::BoolVar l  = Gecode::expr(*this, vars[2] == 1);  // Lamb

        Gecode::BoolVar uw = Gecode::expr(*this, util[0] == 1);  // Wolf utility
        Gecode::BoolVar uc = Gecode::expr(*this, util[1] == 1);  // Cabbage tility
        Gecode::BoolVar ul = Gecode::expr(*this, util[2] == 1);  // Lamb utility
        
        rel(*this, uw == (w && l));
        rel(*this, uc == 0);
        rel(*this, ul == ((!w && c && l) || (w && !l)));

        branch(*this, vars, Gecode::INT_VAR_NONE(), 
                            Gecode::INT_VAL_MIN() );
    }
    //---------------------------------------------------------
    SubModel(SubModel& source) 
    : Gecode::Space(source) {
        vars.update(*this, source.vars);
        util.update(*this, source.util);
        optvar = source.optvar;
    }
    //---------------------------------------------------------
    virtual Gecode::Space* copy() {
        return new SubModel(*this);
    }
    //---------------------------------------------------------
    void setOptVar(int i) {
        optvar = i;
    }
    //---------------------------------------------------------
    virtual void constrain(const Gecode::Space& current) {
        const SubModel& candidate = 
            static_cast<const SubModel&>(current);
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
    //------------------------------------------------------------
    void print() {
        std::cout << vars << " " << util << std::endl;
    }
};

Equilibrium::Equilibrium(MainProblem& p) : problem(p){
    for (int i=0; i<3; i++) {
        problem.vars[i]->attach(this, 0, EVENT_F );
    }
}
//-------------------------------------------------------------
bool Equilibrium::propagate()  {
    for (int i=0; i<3; i++) {
        if (!problem.vars[i]->isFixed()) return true;
    }

    if ( checkNash() == false ) return false;

    return true;
}
//-------------------------------------------------------------
void Equilibrium::wakeup(int i, int) {
    pushInQueue();
}
//-------------------------------------------------------------
void Equilibrium::clearPropState() { 
    in_queue = false; 
}
//-------------------------------------------------------------
bool Equilibrium::checkNash() { 
    for (int i=0; i<3; i++) {
        int currentutility = problem.util[i]->getVal();
        SubModel* submodel = new SubModel();
        for (int j=0; j<3; j++) {
            if (j==i) continue;
            submodel->fixValue(j, problem.vars[j]->getVal());
        }

        submodel->setOptVar(i);
        Gecode::BAB<SubModel> subengine(submodel);
        delete submodel;
        SubModel* best = nullptr;
        while (SubModel* better = subengine.next()) {
            if (best) delete best;
            best = better;
        }
        if (best) {
            int bestutility = best->getUtility(i);
            delete best;

            if (bestutility>currentutility) {
                if (so.lazy) {
                    Clause* r = Reason_new(4);

                    for (int j=0; j<3; j++) {
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

#endif  // MYPROPAGATOR_H