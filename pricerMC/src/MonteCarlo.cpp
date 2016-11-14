#include "cstdlib"
#include <math.h>
#include <time.h>
#include "iostream"
using namespace std;

#include "MonteCarlo.hpp"

MonteCarlo::MonteCarlo(BlackScholesModel *model, Option *option, int nbSamples, double fdStep) {
    mod_ = model;
    opt_ = option;
    rng_ = pnl_rng_create(PNL_RNG_MERSENNE);
    pnl_rng_sseed(rng_, time(NULL));
    fdStep_ = (fdStep == 0) ? DEFAULT_VALUE_FDSTEP : fdStep;
    nbSamples_ = nbSamples;
    path = pnl_mat_create(opt_->nbTimeSteps_ + 1, mod_->size_);
    pathShifted = pnl_mat_create(opt_->nbTimeSteps_ + 1, mod_->size_);
}

// Price at t
void MonteCarlo::price(const PnlMat *past, double t, double &prix, double &ic) {

    double T = opt_->T_;
    double r = mod_->r_;

    double discountFactor = exp(-r*(T-t));

    double espEstimation = 0;
    double varEstimateur = 0;
    for (int j = 0; j < nbSamples_; ++j) {
        double estimation = payOffSimulation(past, t);
        espEstimation += estimation;
        varEstimateur += estimation * estimation;
    }
    espEstimation /= (double)nbSamples_;
    varEstimateur /= (double)nbSamples_;
    varEstimateur = exp(-2*r*T)*fabs(varEstimateur - espEstimation * espEstimation);

    prix = discountFactor * espEstimation;
    ic = (1.96) * sqrt(varEstimateur/(double)nbSamples_);
}

// Price at t=0
void MonteCarlo::price(double &prix, double &ic) {
    price(NULL,0,prix,ic);
}

void MonteCarlo::delta(const PnlMat *past, double t, PnlVect *delta) {

    // Get useful values
    double T = opt_->T_;
    double r = mod_->r_;
    double M = nbSamples_;
    double discountFactor = exp(-r * (T-t));
    PnlVect *St = pnl_vect_new();
    pnl_mat_get_row(St, past, past->m - 1);

    PnlVect *payOffDiff = pnl_vect_create(delta->size);
    pnl_vect_set_zero(delta);
    // For each simulation
    for (int m = 0; m < nbSamples_; ++m) {
        // Simulation at t
        payOffSimulationShiftedDiff(payOffDiff,past,t);
        for (int d = 0; d < delta->size; ++d)
            LET(delta,d) += GET(payOffDiff,d);
    }
    // Delta
    for (int d = 0; d < delta->size; ++d){
        LET(delta,d) *= (discountFactor / (M * 2 * GET(St,d) * fdStep_));
    }


    // Free
    pnl_vect_free(&St);
    pnl_vect_free(&payOffDiff);
}

double MonteCarlo::payOffSimulation(const PnlMat *past, double t) {
    double payOffSimu = 0;
    double T = opt_->T_;
    int nbTimeSteps = opt_->nbTimeSteps_;

    // Simulation
    if (past == NULL || t == 0)
        mod_->asset(path, T, nbTimeSteps, rng_);
    else
        mod_->asset(path, t, T, nbTimeSteps, rng_, past);

    payOffSimu = opt_->payoff(path);

    return payOffSimu;
}

void MonteCarlo::payOffSimulationShiftedDiff(PnlVect *payOffDiff, const PnlMat *past, double t) {
    double T = opt_->T_;
    int nbTimeSteps = opt_->nbTimeSteps_;
    double timeStep = T / nbTimeSteps;
    int D = mod_->size_;

    // Simulation
    if (past == NULL || t == 0)
        mod_->asset(path, T, nbTimeSteps, rng_);
    else
        mod_->asset(path, t, T, nbTimeSteps, rng_, past);

    // For each asset
    for (int d = 0; d < D; ++d) {
        mod_->shiftAsset(pathShifted,path,d,fdStep_,t,timeStep);
        double simu = opt_->payoff(pathShifted);
        mod_->shiftAsset(pathShifted,path,d,(-fdStep_),t,timeStep);
        simu -= opt_->payoff(pathShifted);
        LET(payOffDiff,d) = simu;
    }
}

MonteCarlo::~MonteCarlo() {
    // delete mod_;
    // delete opt_;
    pnl_rng_free(&rng_);
    pnl_mat_free(&path);
    pnl_mat_free(&pathShifted);
}