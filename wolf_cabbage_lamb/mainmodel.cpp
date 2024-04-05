#ifndef MYPROBLEM_H
#define MYPROBLEM_H

#include "chuffed/vars/modelling.h"
#include "equilibrium.cpp"
#include "iostream"

const int w = 0;
const int c = 1;
const int l = 2;

class MainProblem : public Problem {
private:
    vec<IntVar*>    vars;
    vec<IntVar*>    util;
    friend class Equilimbrium;
public:
    //-----------------------------------------------------
    MainProblem() {
        createVars(vars,3,0,1);
        createVars(util,3,0,1);

        int_rel(util[w], IRT_EQ, 1);
        int_rel(util[c], IRT_EQ, 0);
        int_rel(util[l], IRT_EQ, vars[l]);

        // new MyPropagator(*this);

        branch(vars, VAR_INORDER, VAL_MIN);
        output_vars(vars);
    }
    //-----------------------------------------------------
    void print(std::ostream& out) override {
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
    //-----------------------------------------------------
    std::string toStr() {
        std::stringstream os;
        print(os);
        return os.str();
    }
};

//-------------------------------------------------------------

uint64_t gettime() {
    using   namespace std::chrono;
    return  duration_cast<milliseconds>(
                system_clock::now().time_since_epoch()
            ).count();
}

//-------------------------------------------------------------

int main(int argc, char *argv[])
{
    parseOptions(argc,argv);
    so.nof_solutions = 0;
    // so.lazy = false;
    MainProblem* problem = new MainProblem();
    
    uint64_t t1 = gettime();
    engine.solve(problem);    
    uint64_t t2 = gettime();
    std::cout << ": " << t2-t1 << std::endl;

    delete problem;
    return 0;
}


#endif // MYPROBLEM_H