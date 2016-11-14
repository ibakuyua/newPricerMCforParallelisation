#include <cstdlib>
#include <iostream>
#include "pnl/pnl_random.h"
#include "pnl/pnl_vector.h"
#include "pnl/pnl_matrix.h"

using namespace std;

#include "../BlackScholesModel.hpp"

void displayParameter(int size, double r, double rho, double sigmaV, int n, double maturity, double t, double** past);

#define NB_TIMEVALUE_KNOWN 5 // Must be equal to (int)(t*N/maturity)+1+1]
#define NB_ASSET 1 // Must be equal to size

int main (int argc, char** argv){

    int size = NB_ASSET;
    double r = 0.02;
    double rho = 0.5;
    double sigmaV = 0; // without random
    int N = 20;
    double maturity = 10;
    double t = 1.7;

    double pastArray[NB_TIMEVALUE_KNOWN][NB_ASSET] = {{10},
                                                      {10.7},
                                                      {11.9},
                                                      {15.6},
                                                      {16.4}}; // S(t)


    /////////////////////////////////////////////////////////////////////////////////
    // Pour le display parameter
    double **pastArrayPtr = (double**) malloc(NB_TIMEVALUE_KNOWN * sizeof(double*));
    for (int i = 0; i < NB_TIMEVALUE_KNOWN; ++i) {
        pastArrayPtr[i] = (double*) malloc(NB_ASSET * sizeof(double));
        for (int d = 0; d < NB_ASSET; ++d) {
            pastArrayPtr[i][d] = pastArray[i][d];
        }
    }

    cout << "\n---- TEST BlackScholesModel Asset at t ----\n\n";
    displayParameter(size,r,rho,sigmaV,N,maturity,t, pastArrayPtr);

    cout << "Initialisation de sigma : \n";
    PnlVect *sigma = pnl_vect_create(size);
    for (int d = 0; d < size; ++d) {
        PNL_SET(sigma, d, sigmaV);
    }
    cout << "CHECK : \n\n";
    pnl_vect_print(sigma);

    cout << "\n\nInitialisation de past : \n";
    PnlMat *past = pnl_mat_create(NB_TIMEVALUE_KNOWN,NB_ASSET);
    for (int i = 0; i < NB_TIMEVALUE_KNOWN; ++i) {
        for (int d = 0; d < NB_ASSET; ++d) {
            PNL_MSET(past,i,d,pastArray[i][d]);
        }
    }

    cout << "CHECK : \n\n";
    pnl_mat_print(past);

    cout << "\n\nInitialisation du modèle : ";
    PnlVect *spot = pnl_vect_new();
    pnl_mat_get_row(spot,past,0);
    BlackScholesModel *mod = new BlackScholesModel(size, r, rho, sigma, spot);
    cout << "CHECK\n";

    cout << "Initialisation de la matrice : ";
    PnlMat *path = pnl_mat_create(N, size);
    cout << "CHECK\n";

    cout << "Initialisation du rng : ";
    PnlRng *rng = pnl_rng_create(PNL_RNG_MERSENNE);
    pnl_rng_sseed(rng, time(NULL));
    cout << "CHECK\n";

    cout << "Simulation de la matrice : ";
    mod->asset(path, t, maturity, N, rng, past);
    cout << "CHECK : \n\n";

    pnl_mat_print(path);

    cout << "\n\n Libération des objets : ";
    pnl_vect_free(&spot);
    pnl_vect_free(&sigma);
    pnl_mat_free(&path);
    pnl_rng_free(&rng);
    for (int i = 0; i < NB_TIMEVALUE_KNOWN; ++i) {
        free(pastArrayPtr[i]);
    }
    free(pastArrayPtr);
    cout << "CHECK \n\n";

    return EXIT_SUCCESS;
}

void displayParameter(int size, double r, double rho, double sigmaV, int n, double maturity, double t, double** past) {
    cout << "\n********************";
    cout << "\n* Nombre d'actifs : " << size;
    cout << "\n* Taux sans risque : " << r;
    cout << "\n* Corrélation : " << rho;
    cout << "\n* Valeur des volatilités : " << sigmaV;
    cout << "\n* Nombre de pas de discrétisation : " << n;
    cout << "\n* Maturité : " << maturity;
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
