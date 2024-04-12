#include "equilibrium.h"
#include "iostream"

//------------------------------------------------------------

MainProblem::MainProblem() {
    createVars(vars,3,1,3);
    createVars(util,3,1,3);

    //Model in chuffed way

    equilibrium(vars,util);

    branch(vars, VAR_INORDER, VAL_MIN);
    output_vars(vars);
}

//------------------------------------------------------------

SubProblem::SubProblem() 
:   vars(*this,3,1,3), 
    util(*this,3,1,3) {

    //Model in gecode way

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

// /*------------- for testing purspose ---------------------*/
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
