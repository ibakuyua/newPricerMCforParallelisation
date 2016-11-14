
#include <iostream>
#include "pnl/pnl_random.h"
#include "pnl/pnl_vector.h"
#include "pnl/pnl_matrix.h"

using namespace std;

#include "../BlackScholesModel.hpp"

void displayParameter(int size, int asset, double h, int n, double maturity, double t,  PnlMat* path);

int main (int argc, char** argv){
    int size = 3;
    double r = 0.02;
    double rho = 0.2;
    int nbTimeStep = 3;
    double T = 12;
    int d = 1;

    double t = 4.7;
    double timeStep = T / nbTimeStep;

    double h = 0.111111;

    cout << "\n---- TEST BlackScholes Asset Shift ----\n\n";

    PnlMat * shift_path = pnl_mat_create_from_zero(nbTimeStep + 1,size);
    PnlMat * path = pnl_mat_create_from_zero(nbTimeStep + 1,size);

    MLET(path,0,0) = 1;
    MLET(path,1,0) = 1;
    MLET(path,2,0) = 1;
    MLET(path,3,0) = 1;

    MLET(path,0,1) = 1;
    MLET(path,1,1) = 1;
    MLET(path,2,1) = 1;
    MLET(path,3,1) = 1;

    MLET(path,0,2) = 1;
    MLET(path,1,2) = 1;
    MLET(path,2,2) = 1;
    MLET(path,3,2) = 1;

    displayParameter(size,d,h,nbTimeStep,T,t,path);

    PnlVect *sigma = pnl_vect_create(size);
    PnlVect *spot = pnl_vect_create(size);
    cout << "Initialisation du modèle : ";
    BlackScholesModel * model = new BlackScholesModel(size,r,rho,sigma,spot);
    cout << "CHECK\n";
    cout << "\nShift Asset : \n\n";
    model->shiftAsset(shift_path,path,d,h,t,timeStep);
    pnl_mat_print(shift_path);
    cout << "\nCHECK\n";
    pnl_vect_free(&sigma);
    pnl_vect_free(&spot);
    pnl_mat_free(&shift_path);
    pnl_mat_free(&path);

    return EXIT_SUCCESS;
}

void displayParameter(int size, int asset, double h, int n, double maturity, double t, PnlMat* path) {
    cout << "\n********************";
    cout << "\n* Nombre d'actifs : " << size;
    cout << "\n* Actif choisi : " << asset;
    cout << "\n* Pas de discrétisation : " << h;
    cout << "\n* Nombre de pas de discrétisation : " << n;
    cout << "\n* Maturité : " << maturity;
    cout << "\n* Temps t : " << t;
    cout << "\n* Path : \n\n";
    pnl_mat_print(path);
    cout << "\n********************\n\n";
}
