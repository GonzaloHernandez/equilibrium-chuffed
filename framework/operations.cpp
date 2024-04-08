#include "equilibrium.h"
#include "iostream"

//------------------------------------------------------------

MainProblem::MainProblem() {
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

    equilibrium(vars,util);

    branch(vars, VAR_INORDER, VAL_MIN);
    output_vars(vars);
}

//------------------------------------------------------------

SubProblem::SubProblem() 
:   vars(*this,3,1,3), 
    util(*this,3,-10,30) {
    Gecode::IntVar x = vars[0];
    Gecode::IntVar y = vars[1];
    Gecode::IntVar z = vars[2];

    rel(*this, util[0] == x + y + z);
    rel(*this, util[1] == x * y * z);
    rel(*this, util[2] == x - y - z);

    branch(*this, vars, Gecode::INT_VAR_NONE(), 
                        Gecode::INT_VAL_MIN() );
}

//------------------------------------------------------------

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
