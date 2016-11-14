#include <cstdlib>
#include <iostream>
#include "pnl/pnl_random.h"
#include "pnl/pnl_vector.h"
#include "pnl/pnl_matrix.h"
#include "pnl/pnl_finance.h"

using namespace std;

#include "../BlackScholesModel.hpp"
#include "../MonteCarlo.hpp"
#include "../Options/OptionBasket.hpp"

void displayParameter(int size, double r, double rho, double sigmaV, int n, double maturity, double t, double** past, double K, int M);

#define NB_TIMEVALUE_KNOWN 8 // Must be equal to (int)(t*N/maturity)+2]
#define NB_ASSET 1 // Must be equal to size

int main (int argc, char** argv) {

    int size = NB_ASSET;
    double r = 0.02;
    double rho = 0.5;
    double sigmaV = 0.2;
    int N = 20;
    double maturity = 10;
    double t = 3.3;
    double K = 100;
    int M = 50000;

    double pastArray[NB_TIMEVALUE_KNOWN][NB_ASSET] = {{100},
                                                      {107.2},
                                                      {110.9},
                                                      {112.6},
                                                      {115.9},
                                                      {113.9},
                                                      {116.2},
                                                      {177.4}}; // S(t)


    /////////////////////////////////////////////////////////////////////////////////
    // Pour le display parameter
    double **pastArrayPtr = (double **) malloc(NB_TIMEVALUE_KNOWN * sizeof(double *));
    for (int i = 0; i < NB_TIMEVALUE_KNOWN; ++i) {
        pastArrayPtr[i] = (double *) malloc(NB_ASSET * sizeof(double));
        for (int d = 0; d < NB_ASSET; ++d) {
            pastArrayPtr[i][d] = pastArray[i][d];
        }
    }

    cout << "\n---- TEST MonteCarlo delta at t ----\n\n";
    displayParameter(size,r,rho,sigmaV,N,maturity,t,pastArrayPtr,K,M);

    cout << "Initialisation de sigma : \n";
    PnlVect *sigma = pnl_vect_create(size);
    for (int d = 0; d < size; ++d) {
        PNL_SET(sigma, d, sigmaV);
    }
    cout << "CHECK : \n\n";
    pnl_vect_print(sigma);

    PnlVect *spot = pnl_vect_create(size);

    cout << "\n\nInitialisation du modèle : ";
    BlackScholesModel *mod = new BlackScholesModel(size, r, rho, sigma, spot);
    cout << "CHECK\n";

    cout << "\n\nCréation de l'option : ";
    PnlVect *weights = NULL;
    OptionBasket *basket = new OptionBasket(maturity, N, size, weights, K);
    cout << "CHECK\n";

    cout << "\n\nInitialisation de past : \n";
    PnlMat *past = pnl_mat_create(NB_TIMEVALUE_KNOWN,NB_ASSET);
    for (int i = 0; i < NB_TIMEVALUE_KNOWN; ++i) {
        for (int d = 0; d < NB_ASSET; ++d) {
            PNL_MSET(past,i,d,pastArray[i][d]);
        }
    }

    cout << "CHECK : \n\n";
    pnl_mat_print(past);

    cout << "\n\nInitialisation de l'objet montecarlo : ";
    MonteCarlo *monteCarlo = new MonteCarlo(mod, basket, M);
    cout << "CHECK\n";

    PnlVect *delta = pnl_vect_create(1);
    monteCarlo->delta(past,t,delta);

    cout << "Delta sur l'option Call à t : \n";
    cout << "\n---> Delta : ";
    pnl_vect_print(delta);

    double prixFF, deltaFF;
    prixFF = pnl_cf_call_bs(pastArray[NB_TIMEVALUE_KNOWN-1][0],K,maturity-t,r,0,sigmaV, &prixFF, &deltaFF);
    cout << "\n---> Delta formule fermée : " << deltaFF;

    cout << "\n\nCHECK";

    return EXIT_SUCCESS;
}

void displayParameter(int size, double r, double rho, double sigmaV, int n, double maturity, double t, double** past, double K, int M) {
    cout << "\n********************";
    cout << "\n* Nombre d'actifs : " << size;
    cout << "\n* Taux sans risque : " << r;
    cout << "\n* Corrélation : " << rho;
    cout << "\n* Valeur des volatilités : " << sigmaV;
    cout << "\n* Nombre de pas : " << n;
    cout << "\n* Maturité : " << maturity;
    cout << "\n* Strike : " << K;
    cout << "\n* Nombre de simulation : " << M;
    cout << "\n* Temps t : " << t;
    cout << "\n* Valeurs passées : \n";
    for (int i = 0; i < NB_TIMEVALUE_KNOWN; ++i) {
        cout << "\n*-- Temps " << i;
        for (int d = 0; d < NB_ASSET; ++d) {
            cout << "\n*-- Action " << d << "  : " << past[i][d];
        }
    }
    cout << "\n********************\n\n";
}
