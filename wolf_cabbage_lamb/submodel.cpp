#include "gecode/int.hh"
#include "gecode/minimodel.hh"

class SubModel : public Gecode::Space {
protected:
    Gecode::IntVarArray vars;
    Gecode::IntVarArray util;
    int optvar;
public:
    SubModel() : vars(*this,3,0,1), util(*this,3,0,1) {
        optvar = 0;

        Gecode::BoolVar w  = Gecode::expr(*this, vars[0] == 1);  // Wolf
        Gecode::BoolVar c  = Gecode::expr(*this, vars[1] == 1);  // Cabbage
        Gecode::BoolVar l  = Gecode::expr(*this, vars[2] == 1);  // Lamb

        Gecode::BoolVar uw = Gecode::expr(*this, util[0] == 1);  // Wolf utility
        Gecode::BoolVar uc = Gecode::expr(*this, util[1] == 1);  // Cabbage tility
        Gecode::BoolVar ul = Gecode::expr(*this, util[2] == 1);  // Lamb utility
        
        rel(*this, uw == (w && l));
        rel(*this, uc == 0);
        rel(*this, ul == ((!w && c && l) || (w && !l)));

        branch(*this, vars, Gecode::INT_VAR_NONE(), 
                            Gecode::INT_VAL_MIN() );
    }
    //---------------------------------------------------------
    SubModel(SubModel& source) 
    : Gecode::Space(source) {
        vars.update(*this, source.vars);
        util.update(*this, source.util);
        optvar = source.optvar;
    }
    //---------------------------------------------------------
    virtual Gecode::Space* copy() {
        return new SubModel(*this);
    }
    //---------------------------------------------------------
    void setOptVar(int i) {
        optvar = i;
    }
    //---------------------------------------------------------
    virtual void constrain(const Gecode::Space& current) {
        const SubModel& candidate = 
            static_cast<const SubModel&>(current);
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
};
