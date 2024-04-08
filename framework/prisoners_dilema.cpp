#include "equilibrium.h"
#include "iostream"

//------------------------------------------------------------

MainProblem::MainProblem() {
    createVars(vars,2,0,1);
    createVars(util,2,0,3);
    IntVar*  x = vars[0];
    IntVar*  y = vars[1];
    IntVar* ux = util[0];
    IntVar* uy = util[1];

    vec<int>        argsux(3);
    vec<IntVar*>    varsux(3);
    argsux[0] = +1; varsux[0] = ux;
    argsux[1] = +2; varsux[1] = y;
    argsux[2] = -1; varsux[2] = x;
    int_linear(argsux,varsux,IRT_EQ,2);

    vec<int>        argsuy(3);
    vec<IntVar*>    varsuy(3);
    argsuy[0] = +1; varsuy[0] = uy;
    argsuy[1] = +2; varsuy[1] = x;
    argsuy[2] = -1; varsuy[2] = y;    
    int_linear(argsuy,varsuy,IRT_EQ,2);

    equilibrium(vars,util);

    branch(vars, VAR_INORDER, VAL_MIN);
    output_vars(vars);
}

//------------------------------------------------------------

SubProblem::SubProblem() 
:   vars(*this,2,0,1),
    util(*this,2,0,3) {
    
    Gecode::IntVar  x = vars[0];
    Gecode::IntVar  y = vars[1];
    Gecode::IntVar ux = util[0];
    Gecode::IntVar uy = util[1];

    rel(*this, ux == 3-((y*2)-x+1));
    rel(*this, uy == 3-((x*2)-y+1));

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
