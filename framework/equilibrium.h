#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"
#include "gecode/int.hh"
#include "gecode/minimodel.hh"

//------------------------------------------------------------

class MainProblem : public Problem {
public:
    vec<IntVar*>    vars;
    vec<IntVar*>    util;
public:
    MainProblem();  // Should be implemented by user
    void print(std::ostream& out) override ;
    std::string toStr();
};

//------------------------------------------------------------

class SubProblem : public Gecode::Space {
protected:
    Gecode::IntVarArray vars;
    Gecode::IntVarArray util;
    int optvar = 0;
public:
    SubProblem();   // Should be implemented by user
    SubProblem(SubProblem& source);
    virtual Gecode::Space* copy();
    virtual void constrain(const Gecode::Space& current);
    void setOptVar(int i);
    void fixValue(int i,int val);
    void setPreference(int i, int val);
    int  getUtility(int i);
    void print();
};

//------------------------------------------------------------

class Equilibrium : public Propagator {
private:
    vec<IntVar*> vars;
    vec<IntVar*> util;
public :
    Equilibrium(vec<IntVar*>& v, vec<IntVar*>& u);
    void wakeup(int i, int) override;
    void clearPropState()   override;
    bool propagate()        override;
    bool checkNash();
};

//------------------------------------------------------------
// Main Problem (Chuffed)
//------------------------------------------------------------

void MainProblem::print(std::ostream& out) {
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
//------------------------------------------------------------
std::string MainProblem::toStr() {
    std::stringstream os;
    print(os);
    return os.str();
}  

//------------------------------------------------------------
// Sub Problem (Gecode)
//------------------------------------------------------------

SubProblem::SubProblem(SubProblem& source) : Gecode::Space(source) {
    vars.update(*this, source.vars);
    util.update(*this, source.util);
    optvar = source.optvar;
}

Gecode::Space* SubProblem::copy() {
    return new SubProblem(*this);
}
//------------------------------------------------------------
void SubProblem::setOptVar(int i) {
    optvar = i;
}
//------------------------------------------------------------
void SubProblem::constrain(const Gecode::Space& current) {
    const SubProblem& candidate = 
    static_cast<const SubProblem&>(current);
    Gecode::rel(*this, 
                util[optvar], 
                Gecode::IRT_GR, 
                candidate.util[optvar]);
}
//------------------------------------------------------------
void SubProblem::fixValue(int i,int val) {
    Gecode::rel(*this, vars[i], Gecode::IRT_EQ, val);
}
//------------------------------------------------------------
void SubProblem::setPreference(int i, int val) {
    rel(*this, util[i], Gecode::IRT_GR, val);
}
//------------------------------------------------------------
int SubProblem::getUtility(int i) {
    return util[i].val();
}
//------------------------------------------------------------
void SubProblem::print() {
    std::cout << vars << " " << util << std::endl;
}

//------------------------------------------------------------
// Equilibrium (Chuffed)
//------------------------------------------------------------

Equilibrium::Equilibrium(vec<IntVar*>& v, vec<IntVar*>& u) 
: vars(v), util(u)
{
    for (int i=0; i<vars.size(); i++) {
        vars[i]->attach(this, i, EVENT_F );
        util[i]->attach(this, i, EVENT_F );
    }
};
//------------------------------------------------------------
void Equilibrium::wakeup(int i, int) {
    pushInQueue();
}
//------------------------------------------------------------
void Equilibrium::clearPropState() {
    in_queue = false; 
}//------------------------------------------------------------
bool Equilibrium::propagate() {
    for (int i=0; i<vars.size(); i++) {
        if (!vars[i]->isFixed()) return true;
        if (!util[i]->isFixed()) return true;
    }

    if ( checkNash() == false ) return false;

    return true;        
};
//------------------------------------------------------------
bool Equilibrium::checkNash() {
    int n = vars.size();

    for (int i=0; i<n; i++) {
        int currentutility = util[i]->getVal();
        SubProblem* submodel = new SubProblem();
        for (int j=0; j<n; j++) {
            if (j==i) continue;
            submodel->fixValue(j, vars[j]->getVal());
        }

        submodel->setOptVar(i);
        Gecode::BAB<SubProblem> subengine(submodel);
        delete submodel;
        SubProblem* best = nullptr;
        while (SubProblem* better = subengine.next()) {
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
                            (*r)[j+1] = util[j]->getValLit();
                            // (*r)[j+1] = util[j]->getLit(bestutility, LR_EQ);
                        }
                        else {
                            // (*r)[j+1] = vars[j]->getMinLit();
                            (*r)[j+1] = vars[j]->getMaxLit();
                        }
                    }

                    util[i]->setMin(bestutility,r); 
                }

                return false;
            }
        }
    }
    return true; 
}

//------------------------------------------------------------
// Free functions
//------------------------------------------------------------

void equilibrium(vec<IntVar*>& v, vec<IntVar*>& u) {
    new Equilibrium(v,u);
}