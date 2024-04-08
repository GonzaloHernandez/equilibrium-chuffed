
#include "chuffed/core/propagator.h"

class Order : public Propagator {
private:
    IntView<0> x,y;
public:
    Order(IntView<0>_x,IntView<0>_y) : x(_x), y(_y) {
        x.attach(this,0, EVENT_F);
        y.attach(this,1, EVENT_F);
    }
    //--------------------------------------------------
    void wakeup(int, int) override {
        pushInQueue();
    }
    //--------------------------------------------------
    void clearPropState() override {
        in_queue = false;
    }
    //--------------------------------------------------
    bool propagate() override {

        if (so.lazy) {
            if (x.setMaxNotR(y.getMax())) {
                if (! x.setMax(y.getMax(), Reason(y.getMaxLit())) ) return false;
                // Clause* c = Reason_new(2);
                // (*c)[1] = ~y.getMaxLit();
                // if (! x.setMax(y.getMax(), Reason(c)) ) return false;
            }
            if (y.setMinNotR(x.getMin())) {
                if (! y.setMin(x.getMin(), Reason(x.getMinLit())) ) return false;
            }
        }
        else {
            if (x.getMax()>y.getMax()) return false;
        }

        return true;
    }
};
