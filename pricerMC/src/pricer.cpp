#include <stdexcept>
#include <cstdlib>
#include <iostream>

#include "MonteCarlo.hpp"
#include "BlackScholesModel.hpp"
#include "Options/Option.hpp"
#include "../src/parser.hpp"
#include "Options/OptionBasket.hpp"
#include "Options/AsianOption.hpp"
#include "Options/PerformanceOption.hpp"


using namespace std;

//////////////////////////////////// PROTOTYPE ///////////////////////////////////////////

/**
 * parserData is a structure due to store datas when parsing input file.
 */
typedef struct {
    string type;
    Option *option; /// Option
    double r; /// Free risk rate of the model
    double rho; /// Correlation between assets
    int nbSamples; /// Number of samples
    int nbHedging; /// Number of hedging dates
    double fdstep; /// fdstep number
    PnlVect* sigma; /// Asset volatilities
    PnlVect* spot; /// Asset spots
    PnlVect* trend; /// Asset trends
} ParserDatas;

/**
 * Permet de pricer à t=0 une option pour différents paramètres
 *
 * @param[in] ParserDatas contenant l'option et les paramètres pour le pricing
 */
void priceAtZero(ParserDatas *datas);
/**
 * Permet de calculer le PnL d'une option avec différents paramètres
 *
 * @param[in] ParserDatas contenant l'option et les paramètres pour le pricing
 */
void computePnL(ParserDatas *datas);
/**
 * Permet de parser le fichier en input
 *
 * @param[in] infile est le chemin vers le fichier d'input
 * @return ParserDatas contenant les données nécessaires à la création du pricer
 */
ParserDatas *parseInputFile(char *infile);
/**
 * Permet de checker la bonne conformité des paramètres
 *
 * @param[in] les datas parsées au préalable
 * @thrown Des exceptions s'il y a des erreurs de conformité
 */
void checkParameters(ParserDatas *datas);

//////////////////////////////////// MAIN ///////////////////////////////////////////

int main(int argc, char **argv){

    ParserDatas *datas;
    char *infile;
    try {
        switch (argc) {
            case 2:
                infile = argv[1];
                datas = parseInputFile(infile);
                checkParameters(datas);
                priceAtZero(datas);
                break;
            case 3:
                infile = argv[2];
                datas = parseInputFile(infile);
                checkParameters(datas);
                computePnL(datas);
                break;
            default:
                throw invalid_argument(
                        "(Invalid Arguement) Please call pricer executable as follows : \n./pricer fichier_input \n./pricer -c fichier_input \n");
        }
    }catch(exception const& e){
        cerr << "[ERREUR] : " << e.what();
        return EXIT_FAILURE;
    }

    // Free
    free(datas);

    return EXIT_SUCCESS;

}

//////////////////////////////////// IMPLEMENTATION ///////////////////////////////////////////

void displayParameters(ParserDatas *datas);
void priceAtZero(ParserDatas *datas) {

    // Model initialisation
    BlackScholesModel* model = new BlackScholesModel(
            datas->option->size_,datas->r,datas->rho,
            datas->sigma,datas->spot);

    // MonteCarlo initialisation
    MonteCarlo* monteCarlo = new MonteCarlo(model,datas->option,datas->nbSamples,datas->fdstep);

    // compute price
    double price;
    double ic;
    monteCarlo->price(price,ic);
    PnlMat *past = pnl_mat_create_from_zero(1,model->size_);
    pnl_mat_set_row(past,model->spot_,0);
    PnlVect *delta = pnl_vect_create(model->size_);
    monteCarlo->delta(past,0,delta);

    // Display results
    displayParameters(datas);
    cout << "\n -----> Price [ " << price << " ]\n";
    cout << "\n -----> IC [ " << ic << " ]\n\n";
    cout << "\n -----> Delta : \n";
    pnl_vect_print(delta);

    // Free
    delete model;
    delete monteCarlo;
}

/**
 * One step of Vi
 */
double hedging(MonteCarlo *monteCarlo, double& V_iMinus1, double capitalizationFactor,
               PnlVect *deltas_iMinus1, PnlVect *Stau_i, PnlMat *past, double t, double &pi);
void computePnL(ParserDatas *datas) {
    double r = datas->r;
    double T = datas->option->T_;
    int N = datas->option->nbTimeSteps_;
    int H = (datas->nbHedging == 0) ? 2 * N : datas->nbHedging;
    int size = datas->option->size_;
    double marketStep = T/(double)H;

    double capitalizationFactor = exp(( r * T) / (double)H);

    // Model initialisation
    BlackScholesModel *model = new BlackScholesModel(
            datas->option->size_, r, datas->rho,
            datas->sigma, datas->spot, datas->trend, H, T);

    // MonteCarlo initialisation
    MonteCarlo* monteCarlo = new MonteCarlo(model, datas->option, datas->nbSamples,datas->fdstep);

    // Market initialisation
    PnlMat *market = pnl_mat_new();
    model->simul_market(size,market);
    // Création du portefeuille et du vecteur de PnL (PnL à chaque date)
    PnlVect *pnlAtDate = pnl_vect_create(H + 1);
    // Initialisation
    //Delta prec
    PnlVect *deltas_iMinus1 = pnl_vect_create(size);
    //Stau courant, les prix au temps i des actifs
    PnlVect *Stau_i = pnl_vect_new();
    pnl_mat_get_row(Stau_i, market, 0);
    PnlMat *past = pnl_mat_create(1, size);
    pnl_mat_set_row(past, model->spot_, 0);
    //pnl_mat_set_row(past, model->spot_, 1);
    double price,ic;
    monteCarlo->price(price,ic);
    monteCarlo->delta(past,0,deltas_iMinus1);
    double V_iMinus1 = price  - pnl_vect_scalar_prod(deltas_iMinus1,Stau_i);
    LET(pnlAtDate, 0) = 0; // Par construction
    // Foreach date
    int iN = 1;
    for (int i = 1; i < pnlAtDate->size; ++i) {
        double t = i * marketStep;
        pnl_mat_get_row(Stau_i, market, i);
        // Si on a passé le prochain pas du modèle ( non du marché ) // TODO Modifier le test dans le cas où N non multiple de H ?

        if ((i * marketStep) >= iN * T/N){
            pnl_mat_add_row(past, past->m, Stau_i);
            iN++;
        }else{
            pnl_mat_set_row(past, Stau_i, past->m - 1);
        }

        LET(pnlAtDate, i) = hedging(monteCarlo, V_iMinus1, capitalizationFactor, deltas_iMinus1, Stau_i, past,t,price);
    }

    // Display result
    displayParameters(datas);
    cout << "\n-----> Pay-Off [ " << price << " ]";
    cout << "\n-----> PnL [ " << GET(pnlAtDate, H) << " ]\n";
    cout << "\n\n\n Marché : \n\n";
    pnl_mat_print(market);
    cout << "\n\n PnL at date : \n";
    pnl_vect_print(pnlAtDate);

    // Free
    delete model;
    delete monteCarlo;
    pnl_mat_free(&market);
    pnl_vect_free(&pnlAtDate);
    pnl_vect_free(&deltas_iMinus1);
    pnl_vect_free(&Stau_i);
    pnl_mat_free(&past);

}

double hedging(MonteCarlo *monteCarlo, double& V_iMinus1, double capitalizationFactor, PnlVect *deltas_iMinus1, PnlVect *Stau_i, PnlMat *past, double t, double &price){
    PnlVect *delta_i = pnl_vect_create(deltas_iMinus1->size);
    monteCarlo->delta(past,t,delta_i);

    double deltas_iStau_i = pnl_vect_scalar_prod(delta_i,Stau_i);
    double deltas_iMinus1Stau_i = pnl_vect_scalar_prod(deltas_iMinus1,Stau_i);

    double Vi = V_iMinus1 * capitalizationFactor - (deltas_iStau_i - deltas_iMinus1Stau_i);

    double ic;
    monteCarlo->price(past,t,price,ic);

    double pi = deltas_iStau_i + Vi;

    V_iMinus1 = Vi;

    return pi - price;
}

ParserDatas *parseInputFile(char *infile)
{
    double T, r, strike, rho, fdStep;
    PnlVect *spot, *sigma, *divid, *weights, *trend;
    string type;
    int size;
    int nbTimeStep;
    int H;
    size_t n_samples;

    Param *P = new Parser(infile);

    //Option params
    P->extract("option type", type);
    P->extract("maturity", T);
    P->extract("option size", size);
    P->extract("strike", strike);
    P->extract("timestep number",nbTimeStep);
    P->extract("payoff coefficients",weights, size);
    //Other params
    P->extract("spot", spot, size);
    P->extract("volatility", sigma, size);
    P->extract("interest rate", r);
    P->extract("correlation",rho);
    P->extract("sample number", n_samples);
    P->extract("hedging dates number",H);
    P->extract("fd step", fdStep);

    if (!P->extract("dividend rate", divid, size))
        divid = pnl_vect_create_from_zero(size);

    // Previous compute of trend with previous data in back/ folder
    /*double trend_d = r + 0.01;
    PnlVect *trend = pnl_vect_create_from_scalar(size,trend_d);*/

    P->extract("trend", trend, size);

    //Filling the structure parserData
    ParserDatas *data = new ParserDatas();
    data -> type = type;
    data -> r = r;
    data -> rho = rho;
    data -> sigma = sigma;
    data -> spot = spot;
    data -> nbSamples = (int) n_samples;
    data -> trend = trend;
    if(H > 0) {
        data->nbHedging = H;
    }else{
        data->nbHedging = 3*nbTimeStep;
    }

    data-> fdstep = fdStep;
    //Creation de l'option basket
    if (type == "basket")
    {
        Option *basketOption = new OptionBasket(T,nbTimeStep,size,weights,strike);
        data->option = basketOption;
    }
        //Creation de l'option asian
    else if (type == "asian") {
        Option *asianOption = new AsianOption(T,nbTimeStep,size,weights,strike);
        data->option = asianOption;
    }
        //Creation de l'option performance
    else {
        Option *performanceOption = new PerformanceOption(T, nbTimeStep, size, weights);
        data->option = performanceOption;
    }

    return data;
}

void checkParameters(ParserDatas *datas){
    // Lever des erreurs de cette façon :
    // if (test[datas.x] is false)
    //      throw invalid_argument("message")

    //nbSamples
    if ( datas->nbSamples <= 0 )
        throw invalid_argument("nbSamples must be > 0");

    //spot
    if (pnl_vect_min(datas -> spot) < 0)
    {
        throw invalid_argument("Invalid negative spot in spot vector");
    }
    //frr
    if(datas -> r < 0)
    {
        throw invalid_argument("Invalid negative free risk rate ");
    }
    //volatility
    if (pnl_vect_min(datas -> sigma) < 0)
    {
        throw invalid_argument("Invalid negative volatility in volatility vector");
    }
    //nbHedging
    if (datas->nbHedging <= 0)
        throw invalid_argument("nbHedging must be > 0");
    if (datas->nbHedging <= datas->option->nbTimeSteps_)
        throw invalid_argument("nbHedging must be > nbTimeSteps");

}

void displayParameters(ParserDatas *datas){
    cout << "\n**** DATAS ****";
    cout << "\n* Free risk rate : " << datas->r;
    cout << "\n* Correlation : " << datas->rho;
    cout << "\n* Volatility (first) : " << GET(datas->sigma,0);
    cout << "\n* Samples number : " << datas->nbSamples;
    cout << "\n* Hedging dates number : " << datas->nbHedging;
    cout << "\n* Spots (first) : " << GET(datas->spot,0);
    //pnl_vect_print(datas->spot);
    if (datas->trend != NULL)
        cout << "\n* Trends (first) : " << GET(datas->trend,0);
    //pnl_vect_print(datas->trend);
    cout << "\n**** OPTION ****";
    cout << "\n* Type : " << datas->type;
    cout << "\n* Size : " << datas->option->size_;
    cout << "\n* Maturity : " << datas->option->T_;
    cout << "\n* Time step number : " << datas->option->nbTimeSteps_;
    cout << "\n*****************\n";
}