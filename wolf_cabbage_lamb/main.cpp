#include "mainmodel.h"

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

// int main(int argc, char const *argv[])
// {
//     SubModel* model = new SubModel();
//     Gecode::DFS<SubModel> engine(model);
//     delete model;
//     while (SubModel* solution = engine.next()) {
//         solution->print();
//         delete solution;
//     }
//     return 0;
// }
