#include <stdexcept>
#include "cstdlib"
#include <stdexcept>
#include <iostream>
#include "cmath"

using namespace std;

#include "Option.hpp"

Option::Option(double T, int nbTimeSteps, int size, PnlVect * weights){
    CheckArguments(T, nbTimeSteps, size);
    T_ = T;
    nbTimeSteps_ = nbTimeSteps;
    size_ = size;

    weights_ = pnl_vect_create_from_zero (size);
    if(weights == NULL){
        for ( int i = 0; i < size; i++) {
            LET(weights_,i) = 1.0/size;
        }

    } else {
        CheckWeights(weights);
        for (int i = 0; i < size; i++) {
            LET(weights_,i) = GET(weights,i);
        }
    }


}

void Option::CheckWeights(PnlVect * weights) {
    if ( weights->size != size_ )
        throw invalid_argument("The size of PnlVect weights_ doesn't match with int size_");

    double SumWeights = 0.0;
    for ( int i = 0; i < size_; i++ )
        SumWeights += GET(weights,i);


     if ( fabs(SumWeights - 1) > 0.0000001 )
         throw invalid_argument("The sum of weights must be equal to 1" );
}

void Option::CheckArguments(double T, int nbTimeSteps, int size) {
    if ( size < 0 )
        throw invalid_argument("size must be > 0");

    if ( nbTimeSteps < 1)
        throw invalid_argument("nbTimeSteps must be >= 1");

    if ( T < 0 )
        throw invalid_argument("T must be > 0");
}

Option::~Option() {
    pnl_vect_free(&weights_);
}
