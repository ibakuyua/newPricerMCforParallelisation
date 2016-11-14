#ifndef MC_PRICER_ASIANOPTION_H
#define MC_PRICER_ASIANOPTION_H

#include "Option.hpp"

class AsianOption : public Option
{
public:
    double strike_;
    //Constructor
    AsianOption(double T = 2, int nbTimeSteps = 100, int size = 3, PnlVect * weights = NULL, double strike_ = 10);

    double payoff(const PnlMat *path);


private:
    double underlyingMean(const PnlMat *path, int underlyingNumber);
};


#endif //MC_PRICER_ASIANOPTION_H
