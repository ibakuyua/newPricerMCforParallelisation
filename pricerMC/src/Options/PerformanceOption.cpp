#include "PerformanceOption.hpp"
#include <math.h>
//#define MAX(a,b) ((a) > (b) ? (a) : (b))

PerformanceOption::PerformanceOption(double T, int nbTimeSteps, int size,PnlVect * weights):Option(T,nbTimeSteps,size,weights) { }

double PerformanceOption::payoff(const PnlMat *path) {
    double payoff = 1;
    double numerateur;
    double denominateur;
    double terme_somme;

    //on boucle sur le nombre de discretisations
    for ( int i = 1; i < path->m ; i ++)
    {
        numerateur = 0;
        denominateur = 0;
        //on boucle sur le nombre d'actifs
        for ( int d = 0; d < path->n;  d++ )
        {
            numerateur += GET(weights_,d) * MGET(path, i , d);
            denominateur += GET(weights_,d) * MGET(path, i - 1, d);
        }
        terme_somme = numerateur / denominateur - 1 ;

        payoff += fmax(terme_somme, 0);
    }

    return payoff;
}