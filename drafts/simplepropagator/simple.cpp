#include "iostream"
#include "chuffed/vars/modelling.h"
#include "order.cpp"

//=====================================================

class Simple : public Problem {
private:
    vec<IntVar*> v;
public:
    Simple() {
        createVars(v,2,1,4);

        new Order( v[0] , v[1] );

        branch(v, VAR_INORDER, VAL_MIN);
        output_vars(v);
    }
    //--------------------------------------------------
    void print(std::ostream& os) override {
        os << v[0]->getVal() << " " << v[1]->getVal();
    };
    //--------------------------------------------------
};


//=====================================================


int main(int argc, char *argv[])
{
    parseOptions(argc,argv);
    so.nof_solutions = 0;
    so.lazy = false;
    Simple* problem = new Simple();
    engine.solve(problem);    
    delete problem;

    return 0;
}
