#ifndef MC_PRICER_OPTIONPERFORMANCE_H
#define MC_PRICER_OPTIONPERFORMANCE_H

#include "Option.hpp"

class PerformanceOption : public Option {

public:

    //Constructor
    PerformanceOption(double T = 2, int nbTimeSteps = 100, int size = 3, PnlVect * weights = NULL);

    double payoff(const PnlMat *path);
};


#endif //MC_PRICER_OPTIONPERFORMANCE_H
