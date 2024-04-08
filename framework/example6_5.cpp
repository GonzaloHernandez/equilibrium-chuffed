#include "equilibrium.h"
#include "iostream"

//------------------------------------------------------------

MainProblem::MainProblem() {
    createVars(vars,3,1,9);
    createVars(util,3,0,1);

    IntVar* x = vars[0];
    IntVar* y = vars[1];
    IntVar* z = vars[2];

    int_rel(z,IRT_LE,3);                // z = {1..3}

    BoolView ux = newBoolVar();
    BoolView uy = newBoolVar();
    BoolView uz = newBoolVar();
    int_rel_reif(util[0],IRT_EQ,1,ux);
    int_rel_reif(util[1],IRT_EQ,1,uy);
    int_rel_reif(util[2],IRT_EQ,1,uz);

    // Utility for x
    IntVar* aux = newIntVar(0,27);
    int_times(y,z,aux);
    int_rel_reif(aux,IRT_EQ,x,ux);

    // Utility for y
    IntVar* auy = newIntVar(0,27);
    int_times(x,z,auy);
    int_rel_reif(auy,IRT_EQ,y,uy);

    // Utility for z
    vec<BoolView> b(3);
    b[0] = newBoolVar();
    b[1] = newBoolVar();
    b[2] = newBoolVar();

    IntVar* xtimesy = newIntVar(0,81);
    int_times(x,y,xtimesy);
    int_rel_reif(xtimesy,IRT_LE,z,b[0]);        // b[0] <- ((x*y)<=z)

    IntVar* xplusy = newIntVar(0,18);
    int_plus(x,y,xplusy);
    int_rel_reif(z,IRT_LE,xplusy,b[1]);         // b[1] <- (z<=(x+y))

    IntVar* xplus1      = newIntVar(0,10);
    IntVar* yplus1      = newIntVar(0,10);
    IntVar* x1timesy1   = newIntVar(0,100);
    IntVar* ztimes3     = newIntVar(0,9);
    int_plus(x,newIntVar(1,1),xplus1);          // (x+1)
    int_plus(y,newIntVar(1,1),yplus1);          // (y+1)
    int_times(xplus1,yplus1,x1timesy1);         // (x+1)*(y+1)
    int_times(z,newIntVar(3,3),ztimes3);        // (z*3)
    int_rel_reif(x1timesy1,IRT_NE,ztimes3,b[2]);// b[2] <- ((x+1)*(y+1)) != (z*3)

    array_bool_and(b,uz);                       // uz = (b[0] /\ b[1] /\ b[2])

    new Equilibrium(vars,util);

    branch(vars, VAR_INORDER, VAL_MIN);
    output_vars(vars);
}

//------------------------------------------------------------

SubProblem::SubProblem() 
:   vars(*this,3,1,9), 
    util(*this,3,0,1) {

    rel(*this,vars[2],Gecode::IRT_LQ,3);

    Gecode::IntVar x = vars[0];
    Gecode::IntVar y = vars[1];
    Gecode::IntVar z = vars[2];
    Gecode::BoolVar ux = expr(*this, util[0] == true);
    Gecode::BoolVar uy = expr(*this, util[1] == true);
    Gecode::BoolVar uz = expr(*this, util[2] == true);

    rel(*this, ux == (x==y*z));
    rel(*this, uy == (y==x*z));
    rel(*this, uz == (((x*y)<=z) && (z<=(x+y)) && (((x+1)*(y+1)) != (z*3))) );
    
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
    so.lazy = false;
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
