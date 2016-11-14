#pragma once

#include "pnl/pnl_vector.h"
#include "pnl/pnl_matrix.h"

/// \brief Classe Option abstraite
class Option
{
public:
    double T_; /// maturité
    int nbTimeSteps_; /// nombre de pas de temps de discrétisation
    int size_; /// dimension du modèle, redondant avec BlackScholesModel::size_
    PnlVect * weights_; /// poids de chaque sous-jacent


    Option(double T = 2, int nbTimeSteps = 100, int size = 3, PnlVect * weights = NULL);


    /**
    *
    *Destructeur de BlackScholesModel
    *
    */
    virtual ~Option();

    /**
     * Calcule la valeur du payoff sur la trajectoire
     *
     * @param[in] path est une matrice de taille (N+1) x d
     * contenant une trajectoire du modèle telle que créée
     * par la fonction asset.
     * @return phi(trajectoire)
     */
    virtual double payoff(const PnlMat *path) = 0;

private:
    /**
     * Permet de vérifier la bonne constitution d'un vecteur de poids
     *
     * @param[in] Le vecteur de poids
     */
    void CheckWeights(PnlVect * vectorToCheck);

    /**
     *
     * Permet de verifier si les arguments n'ont pas d'erreurs en entrée
     *
     */
    void CheckArguments(double T, int nbTimeSteps, int size);
};




