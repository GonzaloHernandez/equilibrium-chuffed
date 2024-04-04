#include "myproblem.h"

int main(int argc, char *argv[])
{
    n = 4;
    parseOptions(argc,argv);
    so.nof_solutions = 0;
    // so.lazy = false;
    MyProblem* problem = new MyProblem();
    engine.solve(problem);
    delete problem;
    return 0;
}
