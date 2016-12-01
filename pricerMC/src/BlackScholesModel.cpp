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
    Gi_ = pnl_vect_new();
    LGi_ = pnl_vect_new();
}

void BlackScholesModel::asset(PnlMat *path, double t, double T, int nbTimeSteps, PnlRng *rng, const PnlMat *past) {

    // Copy of the past matrix in the path matrix before S_t
    for (int i = 0; i < past->m - 1; ++i) {
        for (int d = 0; d < past->n; ++d) {
            PNL_MSET(path,i,d,MGET(past,i,d));
        }
    }
    // NB : The last row is S(t), doesn't belong to a step of constatation !
    PnlVect *St = pnl_vect_create(path->n);
    pnl_mat_get_row(St, past, past->m - 1);
    // Size of a step
    double step = T/nbTimeSteps;
    double sqrtStep = sqrt(step);
    // For the Gaussian vector
    //PnlVect *Gi = pnl_vect_new();
    // For the multiplication between L and Gi
    //PnlVect *LGi;
    // The index after t
    int indexAftert = past->m - 1;
    // Number of value to simulate
    int nbToSimulate = nbTimeSteps - indexAftert + 1;
    // Initialisation of previous value due to simulate S~
    double value_tiMinus1[path->n];
    // Size of the first step (t_(i+1) - t)
    double firstStep = MAX(step * indexAftert - t,0);
    double sqrtFirstStep = sqrt(firstStep);
    // G_0 : First gaussian vector
    pnl_vect_rng_normal_d(Gi_,path->n,rng);
    // All the LdGi
    LGi_ = pnl_mat_mult_vect(L,Gi_);
    for (int d = 0; d < path->n; ++d) {
        double sigma_d = GET(sigma_,d);
        double Sd_t = GET(St, d);
        double LdGi = GET(LGi_, d);

        double value = exp((r_ - sigma_d * sigma_d / 2) * firstStep + sigma_d * sqrtFirstStep * LdGi);

        // Store in path
        MLET(path, indexAftert, d) = (Sd_t * value);
        // Store in value_tiMinus1
        value_tiMinus1[d] = value;
    }
    // For all other simulation
    for (int i = 1; i < nbToSimulate; ++i) {
        // Same processus that beside but with a different step
        pnl_vect_rng_normal_d(Gi_, path->n, rng);
        LGi_ = pnl_mat_mult_vect(L, Gi_);
        // For each asset
        for (int d = 0; d < path->n; ++d) {
            double sigma_d = GET(sigma_,d);
            double Sd_t = GET(St, d);
            double LdGi = GET(LGi_, d);

            double value = value_tiMinus1[d] * exp((r_ - sigma_d * sigma_d/2) * step + sigma_d * sqrtStep * LdGi);

            // Store in path and value_tiMinus1
            PNL_MSET(path, (indexAftert + i), d, (Sd_t * value));
            value_tiMinus1[d] = value;
        }
    }
    // Free
    pnl_vect_free(&St);
    //pnl_vect_free(&Gi);
    //pnl_vect_free(&LGi);
}

void BlackScholesModel::asset(PnlMat *path, double T, int nbTimeSteps, PnlRng *rng) {
    // Case multidimensional with correlation
    // Step initialisation
    double step = T/(double)nbTimeSteps;
    double sqrtStep = sqrt(step);
    // Spot initialisation
    for (int d = 0; d < path->n; ++d)
        PNL_MSET(path, 0, d, GET(spot_,d));
    // For the Gaussian vector
    //PnlVect *Gi = pnl_vect_new();
    // For multiplication between L and Gi
    //PnlVect *LGi = pnl_vect_new();
    // For each time
    for (int i = 1; i < path->m; ++i) {
        pnl_vect_rng_normal_d(Gi_,path->n,rng); // Gi gaussian vector
        LGi_ =  pnl_mat_mult_vect(L,Gi_); // All the LdGi
        // For each asset
        for (int d = 0; d < path->n; ++d) {
            double sigma_d = GET(sigma_,d);
            double Sd_tiMinus1 = PNL_MGET(path, (i-1), d);
            double LdGi = GET(LGi_,d);

            double Sd_ti = Sd_tiMinus1 * exp((r_ - sigma_d * sigma_d / 2) * step + sigma_d * sqrtStep * LdGi);

            PNL_MSET(path,i,d,Sd_ti);
        }
    }
    // Free
    //pnl_vect_free(&Gi);
    //pnl_vect_free(&LGi);
}

void BlackScholesModel::shiftAsset(PnlMat *shift_path, const PnlMat *path, int d, double h, double t,
                                   double timestep) {
    // Index i after t
    int indexAfter_t = (t / timestep - (int)(t / timestep) < 0.000000001)
                       ? (int)(t/timestep)
                       : (int)(t/timestep) + 1;
    //int indexAfter_t = (int)(t / timestep) + 1;
    // shift_path is path with asset d shifted after t
    pnl_mat_clone(shift_path, path);
    for (int i = indexAfter_t; i < path->m; ++i)
        MLET(shift_path,i,d) *= (1+h);
}

void BlackScholesModel::simul_market(int nbAssets, PnlMat *market){

    double timeStep = T_ / (double)H_;
    int m = H_ + 1;
    int n = nbAssets;

    pnl_mat_resize(market, m,n);
    if(trend_ == NULL)
        throw new invalid_argument("trend is null, historical simulation can't be done");
    double sqrtStep = sqrt(timeStep);
    // Spot Init
    for (int d = 0; d < n; ++d)
        PNL_MSET(market,0,d,GET(spot_,d));
    //For the gaussian vector
    PnlVect *Gi = pnl_vect_new();
    // For multiplication between L and Gi
    PnlVect *LGi = pnl_vect_new();
    // rng initialisation
    PnlRng *rng = pnl_rng_create(PNL_RNG_MERSENNE);
    pnl_rng_sseed(rng, time(NULL));
    // For each time
    for (int i = 1; i < m; ++i) {
        pnl_vect_rng_normal_d(Gi, n, rng); // Gi gaussian vector
        LGi = pnl_mat_mult_vect(L, Gi); // All the LdGi
        // For each asset
        for (int d = 0; d < n; ++d) {
            double sigma_d = GET(sigma_, d);
            double Sd_tiMinus1 = PNL_MGET(market, (i - 1), d);
            double LdGi = GET(LGi, d);

            double mu_d = GET(trend_, d);
            double Sd_ti = Sd_tiMinus1 * exp((mu_d - sigma_d * sigma_d / 2) * timeStep + sigma_d * sqrtStep * LdGi);

            PNL_MSET(market, i, d, Sd_ti);
        }
    }
    // Free
    pnl_vect_free(&Gi);
    pnl_vect_free(&LGi);
    pnl_rng_free(&rng);

}

BlackScholesModel::~BlackScholesModel() {
    pnl_mat_free(&L);
    pnl_vect_free(&trend_);
    pnl_vect_free(&spot_);
    pnl_vect_free(&sigma_);
    pnl_vect_free(&Gi_);
    pnl_vect_free(&LGi_);

}

/////////////////////// IMPLEMENTATION OF FUNCTIONS /////////////////////////

PnlMat *getCholeskyFromRho(int size, double rho) {
    // Correlation matrix
    PnlMat *res = pnl_mat_create_from_scalar(size,size,rho);
    for (int i = 0; i < res->n; ++i)
        PNL_MSET(res,i,i,1);
    // Cholesky of the correlation matrix
    pnl_mat_chol(res);
    return res;
}
