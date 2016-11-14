#ifndef MC_PRICER_OPTIONBASKET_H
#define MC_PRICER_OPTIONBASKET_H

#include "Option.hpp"

class OptionBasket : public Option{
public:

    double K_;

    OptionBasket(double T = 2, int nbTimeSteps = 100, int size = 3, _PnlVect * weights = NULL,double K = 10);
    double payoff(const PnlMat *path);

};

#endif //MC_PRICER_OPTIONBASKET_H
