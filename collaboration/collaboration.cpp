#include "myproblem.h"

//-------------------------------------------------------------

uint64_t gettime() {
    using   namespace std::chrono;
    return  duration_cast<milliseconds>(
                system_clock::now().time_since_epoch()
            ).count();
}

//-------------------------------------------------------------

int main(int argc, char *argv[])
{
    n = 3;
    parseOptions(argc,argv);
    so.nof_solutions = 0;
    so.lazy = false;
    MyProblem* problem = new MyProblem();
    
    uint64_t t1 = gettime();
    engine.solve(problem);    
    uint64_t t2 = gettime();
    std::cout << n << ": " << t2-t1 << std::endl;

    delete problem;
    return 0;
}
