#include <cstdlib>
#include <iostream>
#include "pnl/pnl_random.h"
#include "pnl/pnl_vector.h"
#include "pnl/pnl_matrix.h"
#include "pnl/pnl_finance.h"
#include <typeinfo>

using namespace std;

#include "../MonteCarlo.hpp"
#include "../Options/OptionBasket.hpp"

void displayParameter(int size, double r, double rho, double sigmaV, double spotV, int n, double maturity, double K, int M);
void displayOption(OptionBasket opt);


int main(int argc, char** argv){

    int size = 1;
    double r = 0.02;
    double rho = 0.0;
    double sigmaV = 0.2;
    double spotV = 100;
    double maturity = 2;
    double K = 100;
    int N = 10;
    int M = 50000;

    cout << "\n---- TEST MonteCarlo Delta0 ----\n\n";
    displayParameter(size,r,rho,sigmaV,spotV,N,maturity,K,M);

    cout << "Initialisation de sigma : \n";
    PnlVect *sigma = pnl_vect_create(size);
    for (int d = 0; d < size; ++d) {
        PNL_SET(sigma, d, sigmaV);
    }
    cout << "CHECK : \n\n";
    pnl_vect_print(sigma);

    cout << "\n\nInitialisation de spot : \n";
    PnlVect *spot = pnl_vect_create(size);
    for (int d = 0; d < size; ++d) {
        PNL_SET(spot, d, spotV);
    }
    cout << "CHECK : \n\n";
    pnl_vect_print(spot);

    cout << "\n\nInitialisation du modèle : ";
    BlackScholesModel *mod = new BlackScholesModel(size, r, rho, sigma, spot);
    cout << "CHECK\n";

    cout << "\n\nCréation de l'option CALL ";
    PnlVect *weights = NULL;
    OptionBasket *basket = new OptionBasket(maturity, N, size, weights, K);

    cout << "CHECK\n";

    cout << "\n\nInitialisation de l'objet montecarlo : ";
    MonteCarlo *monteCarlo = new MonteCarlo(mod, basket, M);
    cout << "CHECK\n";

    PnlVect *delta = pnl_vect_create(1);
    PnlMat *past = pnl_mat_create_from_zero(1,size);
    pnl_mat_set_row(past,spot,0);
    monteCarlo->delta(past,0,delta);
    cout << "Delta sur l'option en 0 : \n";
    displayOption(*basket);
    cout << "\n---> Delta ";
    pnl_vect_print(delta);

    double prixFF;
    double deltaFF;

    prixFF = pnl_cf_call_bs(spotV,K,maturity,r,0,sigmaV,&prixFF, &deltaFF);
    cout << "\n---> Delta Formule fermée : " << deltaFF;
    cout << "\n\nCHECK";

    return EXIT_SUCCESS;
}
void displayParameter(int size, double r, double rho, double sigmaV, double spotV, int n, double maturity, double K, int M) {
    cout << "\n********************";
    cout << "\n* Nombre d'actifs : " << size;
    cout << "\n* Taux sans risque : " << r;
    cout << "\n* Corrélation : " << rho;
    cout << "\n* Valeur des volatilités : " << sigmaV;
    cout << "\n* Valeur des spots : " << spotV;
    cout << "\n* Nombre de pas : " << n;
    cout << "\n* Maturité : " << maturity;
    cout << "\n* Strike : " << K;
    cout << "\n* Nombre de simulation : " << M;
    cout << "\n********************\n\n";
}
void displayOption(OptionBasket opt){
    cout << "\n********************";
    cout << "\n* Style de l'option : " << typeid(opt).name();
    cout << "\n* Nombre d'actifs : " << opt.size_;
    cout << "\n* Maturité : " << opt.T_;
    cout << "\n* Nombre de pas : " << opt.nbTimeSteps_;
    cout << "\n* Weights : \n";
    pnl_vect_print(opt.weights_);
    cout << "\n********************\n\n";
}
