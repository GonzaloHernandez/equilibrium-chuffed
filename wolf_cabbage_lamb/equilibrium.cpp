#include "chuffed/core/propagator.h"
#include "submodel.cpp"

class Equilibrium : public Propagator {
private:
    vec<IntView<0>> vars;
    vec<IntView<0>> util;
public :

    Equilibrium(vec<IntView<0>> v,vec<IntView<0>> u) : vars(v), util(u) {
        for (int i=0; i<3; i++) {
            vars[i].attach(this, 0, EVENT_F );
        }
    }
    //-------------------------------------------------------------
    bool propagate()  {
        for (int i=0; i<3; i++) {
            if (!vars[i].isFixed()) return true;
        }

        if ( checkNash() == false ) return false;

        return true;
    }
    //-------------------------------------------------------------
    void wakeup(int i, int) {
        pushInQueue();
    }
    //-------------------------------------------------------------
    void clearPropState() { 
        in_queue = false; 
    }
    //-------------------------------------------------------------
    bool checkNash() { 
        for (int i=0; i<3; i++) {
            int currentutility = util[i].getVal();
            SubModel* submodel = new SubModel();
            for (int j=0; j<3; j++) {
                if (j==i) continue;
                submodel->fixValue(j, vars[j].getVal());
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
                    Clause* r = Reason_new(2);

                    (*r)[1] = ~(util[i].getMinLit());
                    util[i].setMin(bestutility,r);

                    return false;
                }
            }
        }
        return true; 
    }
};