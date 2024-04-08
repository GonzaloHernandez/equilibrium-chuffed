#include "equilibrium.h"
#include "iostream"

//------------------------------------------------------------

MainProblem::MainProblem() {
    createVars(vars,3,0,1);
    createVars(util,3,0,1);

    BoolView w = newBoolVar();
    BoolView c = newBoolVar();
    BoolView l = newBoolVar();

    BoolView uw = newBoolVar();
    BoolView uc = newBoolVar();
    BoolView ul = newBoolVar();

    int_rel_reif(vars[0],IRT_EQ,1,w);
    int_rel_reif(vars[1],IRT_EQ,1,c);
    int_rel_reif(vars[2],IRT_EQ,1,l);

    int_rel_reif(util[0],IRT_EQ,1,uw);
    int_rel_reif(util[1],IRT_EQ,1,uc);
    int_rel_reif(util[2],IRT_EQ,1,ul);

    // Utility for Wolf
    vec<BoolView> bv1(2); bv1[0]=w; bv1[1]=l;
    array_bool_and(bv1,uw);

    // Utility for Cabbage
    bool_rel(uc,BRT_EQ,bv_false);

    // Utility for Lamb
    vec<BoolView> bv2(3); bv2[0]=~w; bv2[1]=c; bv2[2]=l;
    BoolView b2 = newBoolVar();
    array_bool_and(bv2,b2);

    vec<BoolView> bv3(2); bv3[0]=w; bv3[1]=~l;
    BoolView b3 = newBoolVar();
    array_bool_and(bv3,b3);

    vec<BoolView> bv4(2); bv4[0]=b2; bv4[1]=b3;
    array_bool_or(bv4,ul);


    equilibrium(vars,util);

    branch(vars, VAR_INORDER, VAL_MIN);
    output_vars(vars);
}

//------------------------------------------------------------

SubProblem::SubProblem() 
:   vars(*this,3,0,1),
    util(*this,3,0,1) {

    Gecode::BoolVar w  = Gecode::expr(*this, vars[0] == 1);
    Gecode::BoolVar c  = Gecode::expr(*this, vars[1] == 1);
    Gecode::BoolVar l  = Gecode::expr(*this, vars[2] == 1);

    Gecode::BoolVar uw = Gecode::expr(*this, util[0] == 1);
    Gecode::BoolVar uc = Gecode::expr(*this, util[1] == 1);
    Gecode::BoolVar ul = Gecode::expr(*this, util[2] == 1);
    
    rel(*this, uw == (w && l));
    rel(*this, uc == 0);
    rel(*this, ul == ((!w && c && l) || (w && !l)));

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
