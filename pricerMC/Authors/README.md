# PricerMonteCarlo
Le but de ce projet est de réfléchir à la structure d’un outil permettant de calculer des prix d’options par une méthode de Monte Carlo ainsi que leur dérivée par rapport au spot, puis de l’implémenter en C++.

# Comment utiliser le pricer ?
1) 	Se placer dans build, et modifier le script makeBuild.sh pour remplacer le path vers la librairie PNL
	(chemin à partir d'où se trouve le CMakeList)
2)	S'assurer des droits sur le script (chmod u+xwr makeBuild.sh)
3)	Lancer ./makeBuild.sh
4)	./pricer [-c] fichier.dat pour lancer le pricer sur le fichier .dat (-c pour simuler la couverture)

# Problèmes à résoudre 
- Problème de calcul du PnL ... 
