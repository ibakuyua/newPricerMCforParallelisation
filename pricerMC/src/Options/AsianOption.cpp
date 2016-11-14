#include <iostream>
#include "AsianOption.hpp"
#include <stdexcept>

AsianOption::AsianOption(double T, int nbTimeSteps, int size, PnlVect * weights,double strike):Option(T,nbTimeSteps,size,weights) {

    if (strike < 0)
    {
        throw std::invalid_argument("The strike of asian option is negative");
    }
    strike_ = strike;
}

double AsianOption::underlyingMean(const PnlMat *path, int underlyingNumber) {

    double mean = 0;

    for (int i = 0; i < path->m; ++i) {
        mean += MGET(path,i,underlyingNumber);
    }

    mean = mean / path->m;

    return mean;
}

double AsianOption::payoff(const PnlMat *path) {

    /*if(path->m != nbTimeSteps_ + 1){
        throw std::invalid_argument("Wrong Path (nbTimeSteps)");
    }
    if(path->n != size_){
        throw std::invalid_argument("Wrong Path (size)");
    }*/

    double payoff = 0;

    for(int d = 0; d < size_; ++d) {
        payoff += GET(weights_,d) * underlyingMean(path,d);
    }
    payoff -= strike_;
    if(payoff < 0){
        return 0;
    } else{
        return payoff;
    }

}
