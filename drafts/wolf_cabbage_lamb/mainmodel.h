#ifndef MYMODEL_H
#define MYMODEL_H

#include "chuffed/vars/modelling.h"
#include "iostream"

class MainProblem : public Problem {
public:
    vec<IntVar*>    vars;
    vec<IntVar*>    util;
    friend class Equilimbrium;
public:
    //-----------------------------------------------------
    MainProblem();
    void print(std::ostream& out) override ;
    std::string toStr();
};

#include "equilibrium.h"

//-------------------------------------------------------------

MainProblem::MainProblem() {
    createVars(vars,3,0,1);
    createVars(util,1,0,1);

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

    vec<BoolView> bv1(2); bv1[0]=w; bv1[1]=l;
    array_bool_and(bv1,uw);

    bool2int(uw,util[0]);

    bool_rel(uc,BRT_EQ,bv_false);

    vec<BoolView> bv2(3); bv2[0]=~w; bv2[1]=c; bv2[2]=l;
    BoolView b2 = newBoolVar();
    array_bool_and(bv2,b2);

    vec<BoolView> bv3(2); bv3[0]=w; bv3[1]=~l;
    BoolView b3 = newBoolVar();
    array_bool_and(bv3,b3);

    vec<BoolView> bv4(2); bv4[0]=b2; bv4[1]=b3;
    array_bool_or(bv4,ul);

    new Equilibrium(*this);

    branch(vars, VAR_INORDER, VAL_MIN);
    output_vars(vars);
}
//-----------------------------------------------------
void MainProblem::print(std::ostream& out) {
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
std::string MainProblem::toStr() {
    std::stringstream os;
    print(os);
    return os.str();
}

#endif // MYMODEL_H