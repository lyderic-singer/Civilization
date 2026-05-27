## Compte rendu 2

**Date :** 12/05/2026

**État :** Core fonctionnel mais instable sur certains points

**Participants :** Lydéric SINGER, Jules GOETZ

---

## État actuel du core

Le moteur de jeu est globalement en place et permet déjà de lancer une partie de test via une CLI temporaire (mélangée au core pour faciliter le debug).

### Points positifs

* Core globalement opérationnel (carte, tours, structures principales OK)
* Première version jouable en CLI de test
* Architecture modulaire respectée (fichiers par système)
* Base solide pour continuer l’itération

---

## Bugs et problèmes identifiés

### 1. Équilibrage / règles de survie

* Paramètres de base empêchent la ville de mourir même sans actions
* Résultat : boucle de survie artificielle → casse les conditions de défaite
* À corriger en priorité (impact gameplay direct)

### 2. Barbares

* Comportement parfois bloqué (pathfinding ou logique de tour)
* Unités qui cessent d’agir dans certains cas de tour
* Bug intermittent donc difficile à reproduire proprement

### 3. Génération de carte

* Surreprésentation des plaines et forêts
* Déséquilibre global du terrain (probabilité de spawn mal calibrée)
* Bug “positif” mais non réaliste pour le design voulu

### 4. Intégration CLI

* CLI actuelle mélange encore affichage et core
* Utilisée comme outil de debug rapide, mais dette technique introduite

---

## Décisions prises

* Le core est **validé comme base stable de travail**
* On accepte temporairement les bugs non bloquants
* Priorité absolue : corriger les règles critiques (survie + barbares bloqués)
* Ensuite seulement : nettoyage architecture + séparation propre CLI/core

---

## Plan d’action (Jules)

* Nettoyage du core (séparation stricte CLI / moteur)
* Correction des bugs critiques :
* survie des villes
* blocage des barbares
* Ajustement génération de map (probabilités équilibrées)
* Refonte CLI propre (outil de test uniquement)
* Mise en place :
* UI CLI propre
* début SDL
* Makefile final structuré

---

## État cible prochaine étape

* Core stable sans comportement “invincible” artificiel
* Barbares entièrement fonctionnels en boucle de jeu
* Map cohérente et reproductible
* CLI propre séparée du moteur
* Base prête pour SDL + interface utilisateur

---

## Conclusion

Le projet est dans une phase normale de développement :  **core en place, mais encore sale et déséquilibré** .

La prochaine étape est clairement une phase de  **stabilisation + nettoyage** , avant d’ajouter toute couche graphique.
