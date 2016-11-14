//
// Created by ruimyb on 9/20/16.
//

#include <exception>
#include <stdexcept>
#include "OptionBasket.hpp"
#include <iostream>

OptionBasket::OptionBasket(double T, int nbTimeSteps, int size, PnlVect * weights,double K):Option(T,nbTimeSteps,size,weights){
    if(K<0)
        throw std::invalid_argument("The strike is negative");
    K_ = K;
}


double OptionBasket::payoff(const PnlMat *path){

    double res = 0;
    /*if(path->m != nbTimeSteps_ + 1){
        throw std::invalid_argument("Wrong Path (nbTimeSteps)");
    }
    if(path->n != size_){
        throw std::invalid_argument("Wrong Path (size)");
    }*/
    for(int i = 0; i < size_; i++){
        res +=  GET(weights_,i)*MGET(path, nbTimeSteps_,i);
    }
    res = res - K_;

    return (res > 0) ? res : 0;
}

