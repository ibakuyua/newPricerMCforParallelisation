#pragma once

#include "Options/Option.hpp"
#include "BlackScholesModel.hpp"
#include "pnl/pnl_random.h"

#define DEFAULT_VALUE_FDSTEP 0.00001
#define DEFAULT_VALUE_NBSAMPLES 10000

class MonteCarlo
{
public:
    BlackScholesModel *mod_; /*! pointeur vers le modèle */
    Option *opt_; /*! pointeur sur l'option */
    PnlRng *rng_; /*! pointeur sur le générateur */
    double fdStep_; /*! pas de différence finie */
    int nbSamples_; /*! nombre de tirages Monte Carlo */
    PnlMat *path; /*! path for simulation : initialized one times only*/
    PnlMat *pathShifted; /*! pathShifted for delta simulation : initialized one times only*/

    /**
     * Constructor
     *
     * @param[in] *model : Le modèle de Black Scholes à utiliser
     * @param[in] *option : L'option à pricer
     * @param[in] nbSamples : Le nombre de tirage de Monte Carlo
     * @param[in] fdStep : Le pas de différence finie
     */
    MonteCarlo(BlackScholesModel *model,Option *option, int nbSamples = DEFAULT_VALUE_NBSAMPLES, double fdStep = DEFAULT_VALUE_FDSTEP);

    /**
     * Calcule le prix de l'option à la date 0
     *
     * @param[out] prix valeur de l'estimateur Monte Carlo
     * @param[out] ic largeur de l'intervalle de confiance
     */
    void price(double &prix, double &ic);

    /**
     * Calcule le prix de l'option à la date t
     *
     * @param[in]  past contient la trajectoire du sous-jacent
     * jusqu'à l'instant t
     * @param[in] t date à laquelle le calcul est fait
     * @param[out] prix contient le prix
     * @param[out] ic contient la largeur de l'intervalle
     * de confiance sur le calcul du prix
     */
    void price(const PnlMat *past, double t, double &prix, double &ic);

    /**
     * Calcule le delta de l'option à la date t
     *
     * @param[in] past contient la trajectoire du sous-jacent
     * jusqu'à l'instant t
     * @param[in] t date à laquelle le calcul est fait
     * @param[out] delta contient le vecteur de delta
     * de confiance sur le calcul du delta
     */
    void delta(const PnlMat *past, double t, PnlVect *delta);

    /**
     *
     *Destructeur de MonteCarlo
     *
     */
    ~MonteCarlo();

private:
    /**
     * Permet une simulation du pay-off de l'option à partir du modèle
     *
     * @return Le pay-off de l'option avec une simulation du sous-jacent
     */
    double payOffSimulation(const PnlMat *past = NULL, double t = 0);

    /**
     * Permet une simulation de la différence de pay-off avec les simulations shiftées
     *
     * @brief La différence de pay-off de l'option avec la simulation shiftée pour chaque sous-jacent se trouve
     * @brief dans payOffDiff
     */
    void payOffSimulationShiftedDiff(PnlVect *payOffDiff ,const PnlMat *past = NULL, double t = 0);
};