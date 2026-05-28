# Bilan de projet — Early Civilization

Projet PP2I - 1A Telecom Nancy 2025-2026  
Sujet : Civilization

**Membres du groupe :**  
Lydéric SINGER, Jules GOETZ

---

## Ce qui s'est bien passé

La séparation core / interfaces a tenu du début à la fin. Lydéric sur le moteur, Jules sur les interfaces et EXT-4. On n'a quasiment pas touché aux mêmes fichiers, ce qui a évité les conflits et rendu l'intégration finale propre.

L'EXT-4 (sauvegarde, chargement, scores, nom du joueur) s'est bien intégrée sans que le moteur ait à changer. Le format texte pour les saves s'est avéré pratique pour débugger : on pouvait ouvrir un `.txt` et lire directement l'état du jeu.

---

## Ce qui s'est moins bien passé

### Bugs découverts tard

Plusieurs bugs ont été trouvés lors de l'intégration plutôt qu'en cours de développement. Des comportements incorrects sur le combat (unités dans un état incohérent après une attaque), des problèmes de chargement de sauvegarde sur fichiers mal formés, un warrior qui mourrait au démarrage à cause d'un bug sur l'or initial. Ces bugs étaient corrigeables une fois identifiés, mais ils ont pris du temps à debugger parce qu'ils n'apparaissaient que dans des cas précis.

### Les tests

Les tests ont été écrits à la fin du projet, pas progressivement. Ça a permis de valider que le core tenait, mais c'est l'inverse de ce qu'on avait prévu au sprint 1. Les bugs mentionnés ci-dessus auraient probablement été trouvés plus tôt avec des tests écrits en même temps que le code.

### Équilibrage

La génération de carte surreprésentait les plaines et forêts au départ. Les paramètres de survie des villes rendaient les conditions de défaite quasi inaccessibles. Ça a été corrigé, mais ça a pollué les tests manuels du sprint 2.

---

## Écarts par rapport au planning

Le sprint 1 a tenu globalement. Le core était en place à la deadline du 29/04.

Le sprint 2 a pris plus de temps que prévu. Les corrections de bugs et l'équilibrage ont retardé le démarrage SDL. Tests et documentation ont été finalisés en dernière semaine.

---

## Ce qu'on referait différemment

- Écrire les tests au fur et à mesure, pas en bloc à la fin
- Travailler l'équilibrage dès le sprint 1 plutôt que de le laisser pour plus tard

---

## Compétences acquises

- Structurer un projet en C avec séparation stricte des modules
- Implémenter une interface SDL2 complète : rendu, événements, caméra, menus
- Sérialiser et désérialiser un état de jeu complet
- Travailler en binôme avec répartition claire des responsabilités
