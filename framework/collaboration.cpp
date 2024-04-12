#include "equilibrium.h"
#include "iostream"

int n = 4;

//------------------------------------------------------------

MainProblem::MainProblem() {
    createVars(vars,n,1,n);
    createVars(util,n,1,n);

    for (int i=0; i<n; i++) {
        vec<BoolView> checks(n);
        for (int j=0; j<n; j++) {
            checks[j] = newBoolVar();
            int_rel_reif(vars[j],IRT_EQ,vars[i],checks[j]);
        }
        bool_linear(checks,IRT_EQ,util[i]);
    }


    equilibrium(vars,util);

    branch(vars, VAR_INORDER, VAL_MIN);
    output_vars(vars);
}

//------------------------------------------------------------

SubProblem::SubProblem()
:   vars(*this,n,1,n), 
    util(*this,n,1,n) {
    
    for (int i=0; i<n; i++) {
        Gecode::count(  *this, 
                        vars, vars[i], 
                        Gecode::IRT_EQ, 
                        util[i]);
    }

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

//------------------------------------------------------------

int main(int argc, char *argv[])
{
    // n = atoi(argv[1]);

    parseOptions(argc,argv);
    so.nof_solutions = 0;
    // so.lazy = false;
    MainProblem* problem = new MainProblem();
    
    uint64_t t1 = gettime();
    engine.solve(problem);    
    uint64_t t2 = gettime();
    std::cout << "n : " << t2-t1 << std::endl;

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
