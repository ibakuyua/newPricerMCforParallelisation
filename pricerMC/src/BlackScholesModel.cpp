#include <iostream>
#include <stdexcept>
#include "cstdlib"


using namespace std;


#include "BlackScholesModel.hpp"

#define DEFAULT_VALUE_FOR_TREND 0.03

//////////////// PROTOTYPES //////////////////
/**
 * Permit to get Cholesky matrix from correlation matrix Cor = (rho)*ones + (1-rho)Id
 */
PnlMat *getCholeskyFromRho(int size, double rho);


//////////////// IMPLEMENTATION OF BLACKSCHOLESMODEL CLASS ///////////////////////////

BlackScholesModel::BlackScholesModel(int size, double r, double rho, PnlVect *sigma, PnlVect *spot, PnlVect *trend, int H,double T) :
        size_(size), r_(r), rho_(rho), sigma_(sigma), spot_(spot), H_(H),T_(T){
    if (sigma->size != size)
        throw new std::invalid_argument("Size of sigma is different of the number of shares");
    if (spot->size != size)
        throw new std::invalid_argument("Size of spot is different of the number of shares");
    if (rho >= 1 || rho <= (-(double)1/(size-1)))
        throw new std::invalid_argument("Correlation not in ]-1/(D-1);1[");
    if (trend == NULL) {
        cout << "## WARNING : Trend is null, default value " << DEFAULT_VALUE_FOR_TREND << " is used";
        trend_ = pnl_vect_create_from_scalar(size,DEFAULT_VALUE_FOR_TREND);
    }
    else
        trend_ = trend;
    if (trend_->size != size)
        throw new std::invalid_argument("Size of trend is different of the number of shares");

    // Cholesky initialisation (computed just one time and not for each asset call)
    L = getCholeskyFromRho(size,rho);
}

