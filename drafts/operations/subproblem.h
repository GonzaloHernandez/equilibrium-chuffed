#include "gecode/int.hh"
#include "gecode/minimodel.hh"

class SubProblem : public Gecode::Space {
protected:
    Gecode::IntVarArray vars;
    Gecode::IntVarArray util;
    int optvar;
public:
    SubProblem() : vars(*this,3,1,3), util(*this,3,-10,30) {
        optvar = 0;

        Gecode::IntVar x = vars[0];
        Gecode::IntVar y = vars[1];
        Gecode::IntVar z = vars[2];
        rel(*this, util[0] == x + y + z);
        rel(*this, util[1] == x * y * z);
        rel(*this, util[2] == x - y - z);

        branch(*this, vars, Gecode::INT_VAR_NONE(), 
                            Gecode::INT_VAL_MIN() );
    }
    //---------------------------------------------------------
    SubProblem(SubProblem& source) 
    : Gecode::Space(source) {
        vars.update(*this, source.vars);
        util.update(*this, source.util);
        optvar = source.optvar;
    }
    //---------------------------------------------------------
    virtual Gecode::Space* copy() {
        return new SubProblem(*this);
    }
    //---------------------------------------------------------
    void setOptVar(int i) {
        optvar = i;
    }
    //---------------------------------------------------------
    virtual void constrain(const Gecode::Space& current) {
        const SubProblem& candidate = 
            static_cast<const SubProblem&>(current);
        Gecode::rel(*this, 
                    util[optvar], 
                    Gecode::IRT_GR, 
                    candidate.util[optvar]);
    }
    //---------------------------------------------------------
    void fixValue(int i,int val) {
        Gecode::rel(*this, vars[i], Gecode::IRT_EQ, val);
    }
    //------------------------------------------------------------
    void setPreference(int i, int val) {
        rel(*this, util[i], Gecode::IRT_GR, val);
    }
    //------------------------------------------------------------
    int getUtility(int i) {
        return util[i].val();
    }
    //------------------------------------------------------------
    void print() {
        std::cout << vars << " " << util << std::endl;
    }
};