#include "chuffed/vars/modelling.h"
#include "equilibrium.h"
#include "iostream"

class MainProblem : public Problem {
public:
    vec<IntVar*>    vars;
    vec<IntVar*>    util;
    friend class Equilimbrium;
public:
    //-----------------------------------------------------
    MainProblem() {
        createVars(vars,3,1,3);
        createVars(util,3,-10,30);

        int_linear(vars,IRT_EQ,util[0]);

        IntVar* temp;
        createVar(temp,-10,30);
        int_times(vars[0],vars[1],temp);
        int_times(temp,vars[2],util[1]);

        vec<int> args(3);
        args[0] = 1;
        args[1] = -1;
        args[2] = -1;
        int_linear(args,vars,IRT_EQ,util[2]);

        new Equilibrium(vars,util);

        branch(vars, VAR_INORDER, VAL_MIN);
        output_vars(vars);
    }
    //-----------------------------------------------------
    void print(std::ostream& out) override  {
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



uint64_t gettime() {
    using   namespace std::chrono;
    return  duration_cast<milliseconds>(
                system_clock::now().time_since_epoch()
            ).count();
}

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

// int main(int argc, char const *argv[])
// {
//     SubProblem* model = new SubProblem();
//     Gecode::DFS<SubProblem> engine(model);
//     delete model;
//     while (SubProblem* solution = engine.next()) {
//         solution->print();
//         delete solution;
//     }
//     return 0;
// }
