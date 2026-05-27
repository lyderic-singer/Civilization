# Compte rendu 1

**Date :** 08/04/2026 | **Durée :** 30m | **Participants :** Lydéric SINGER, Jules GOETZ

---

## Choix du sujet

**Civilization** retenu (vs Carcassonne).**Raisons :**

- Carcassonne: Complexité des connexions de tuiles + assets spécifiques difficiles a trouver
- Civilization: Jeu solo, carte = tableau 2D, mécaniques modulaires, assets SDL faciles

---

## Architecture

- **Séparation stricte** moteur (règles) <-> affichage (CLI/SDL).
- **1 fichier par concept** : `tech.c`, `cities.c`, `units.c`, `barbarians.c` ...
- **Tests unitaires** écrits au fur et à mesure.

---

## Répartition des tâches

| Tâche                                                                                                     | Responsable |
| ---------------------------------------------------------------------------------------------------------- | ----------- |
| **Moteur de jeu: core** (carte, techs, villes, unités, barbares, tours, victoire/défaite)          | Lyderic     |
| **Interfaces** (CLI, SDL, `main.c`, Makefile, README) + **EXT-4** (sauvegarde, menu, scores) | Jules       |

---

## Organisation

- **Dépôt GitLab** : branches `main` (stable) + `develop` (travail).
- **Workflow** : Commits réguliers sur `develop` puis merge vers `main` si compilation + tests OK.
- **Réunions** : Hebdomadaires (prochaine : bilan Sprint 1).

---

## Actions pour prochaines 2 semaines

| Tâche                                          | Qui             | Deadline |
| ----------------------------------------------- | --------------- | -------- |
| Créer dépôt GitLab + structure dossiers      | Jules           | 26/04    |
| Écrire `types.h` (toutes les structs)        | Lyderic         | 27/04    |
| Makefile (cibles de base) + squelette `cli.c` | Jules           | 27-28/04 |
| Implémenter le core                            | Lyderic         | 28/04    |
| Vérification compilation commune et tests      | Lyderic + Jules | 29/04    |

---

*Objectif Sprint 1 : core fonctionnel avec bugs dans pire des cas*
