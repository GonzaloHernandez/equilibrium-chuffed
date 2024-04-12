#include "chuffed/vars/modelling.h"
#include "chuffed/core/propagator.h"
#include "gecode/int.hh"
#include "gecode/minimodel.hh"
#include "table.h"

//------------------------------------------------------------

Table*  newinformation;

bool testing = false;

//------------------------------------------------------------

class MainProblem : public Problem {
public:
    vec<IntVar*>    vars;
    vec<IntVar*>    util;
public:
    MainProblem();  // Should be implemented by user
    void print(std::ostream& out)   override ;
    void addConstraints()           override;
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
    Gecode::Space* copy()                           override;
    void constrain(const Gecode::Space& current)    override;
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
void MainProblem::addConstraints() {

    if (!testing) return;

    /*  This method requires a modification in "chuffed". It's 
        necessary to create this new method in the class Problem
        and adding an invocation en Engine::serach after the 
        "blockCurrentSol" instruction.
    */

    int n = vars.size();
    int** vals = newinformation->getRow(0);
    for (int c = 0; c < newinformation->len(0); c++) {
        Clause& clause = *Reason_new(n);
        int i = vals[c][0];
        for (int j=0; j<n; j++) {
            if (j==i) {
                clause[j] = util[j]->getLit( vals[c][j+1], LR_GE);
            }
            else {
                clause[j] = vars[j]->getLit( vals[c][j+1], LR_NE);
            }
        }
        sat.addClause(clause);
        // sat.confl = &clause;
    }

    newinformation->empty(0);
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
    newinformation = new Table(1,vars.size());
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

        if (!best) continue;    // this should never happen

        int bestutility = best->getUtility(i);

        delete best;

        if ( bestutility > currentutility) {
            if (so.lazy) {
                Clause* r = Reason_new(n);

                int k=1;
                for (int j=0; j<n; j++) {
                    if (j!=i) {
                        (*r)[k++] = vars[j]->getValLit();
                    }
                }

                util[i]->setMin(bestutility,r); 
            }

            return false;
        }

        if (!testing) continue;

        /*This bellow block is part of addConstraint method*/
        int vals[n+1];
        vals[0] = i;
        for (int j=0; j<n; j++) {
            if (j==i) {
                vals[j+1] = bestutility;
            }
            else {
                vals[j+1] = vars[j]->getVal();
            }
        }
        newinformation->add(0,vals);
        /*This above block is part of addConstraint method*/

    }
    return true;
}

//------------------------------------------------------------
// Free functions
//------------------------------------------------------------

void equilibrium(vec<IntVar*>& v, vec<IntVar*>& u) {
    new Equilibrium(v,u);
}