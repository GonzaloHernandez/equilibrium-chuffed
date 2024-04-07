#include "chuffed/core/propagator.h"
#include "subproblem.h"

class Equilibrium : public Propagator {
private:
    vec<IntVar*> vars;
    vec<IntVar*> util;
public :
    //-----------------------------------------------------
    Equilibrium(vec<IntVar*>& v, vec<IntVar*>& u) 
    : vars(v), util(u)
    {
        for (int i=0; i<3; i++) {
            vars[i]->attach(this, i, EVENT_F );
            util[i]->attach(this, i, EVENT_F );
        }
    };
    //-----------------------------------------------------
    bool propagate() override {
        for (int i=0; i<3; i++) {
            if (!vars[i]->isFixed()) return true;
            if (!util[i]->isFixed()) return true;
        }

        // std::cout   << "+++ ";

        // for (int i=0; i<3; i++) {
        //     std::cout   << "{"
        //                 << vars[i]->getMin() << ".."
        //                 << vars[i]->getMax() << "} ";
        // }
        // for (int i=0; i<3; i++) {
        //     std::cout   << "{"
        //                 << util[i]->getMin() << ".."
        //                 << util[i]->getMax() << "} ";
        // }

        // std::cout << std::endl;

        if ( checkNash() == false ) return false;

        return true;        
    };
    //-----------------------------------------------------
    bool checkNash() {
        for (int i=0; i<3; i++) {
            int currentutility = util[i]->getVal();
            SubProblem* submodel = new SubProblem();
            for (int j=0; j<3; j++) {
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
                        Clause* r = Reason_new(4);

                        for (int j=0; j<3; j++) {
                            if (j==i) {
                                (*r)[j+1] = util[j]->getMinLit();
                            }
                            else {
                                (*r)[j+1] = vars[j]->getMinLit();
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
    //-----------------------------------------------------
    void wakeup(int i, int) override {
        pushInQueue();
    }
    //-----------------------------------------------------
    void clearPropState() override {
        in_queue = false; 
    }
};
