## Compte rendu 3

**Date :** 26/05/2026

**État :** Version jouable + UI SDL + intégration finalisée

**Participants :** Lydéric SINGER, Jules GOETZ

---

## État global

Le projet a atteint une  **version stable et fonctionnelle** .

Le jeu tourne correctement, l’interface SDL est en place et le rendu est propre.

---

## Travail réalisé

### Jules — correction & UI

* Correction des bugs identifiés sur le core (barbares, logique de ville, comportements incohérents)
* Nettoyage global de la base côté moteur
* Mise en place d’une **UI SDL fonctionnelle**
* couleurs ajoutées
* rendu plus lisible et structuré
* meilleure séparation visuelle des éléments de jeu
* Finalisation du Makefile (compilation propre et utilisable)

---

## Problème Git rencontré

* Mauvaise gestion des branches au départ (pas de travail isolé sur branche feature)
* Impossible de faire un merge propre au moment voulu
* Solution appliquée :
* concaténation manuelle des commits de chacun
* reconstitution de l’historique final dans un seul flux cohérent

 Résultat :

* Historique Git moins propre que prévu
* Mais **code final stable et entièrement fonctionnel**

---

## État actuel du jeu

* Jeu jouable sans crash
* Core stable
* UI SDL fonctionnelle et propre
* Makefile opérationnel
* Expérience visuelle améliorée

Globalement : **version propre et présentable**

---

## Restant à faire

* Fichier de tests propre (actuellement incomplet / non structuré)
* README à nettoyer et finaliser
* README non push pour l’instant
* EXT-4 Nom + Version
* Fix bug UI : clic sur case avec warrior et ville dessus, choisir lequel on veut

---

## Conclusion

Malgré les problèmes de gestion Git, l’intégration finale est réussie.

Le projet est désormais dans un état  **stable, jouable et visuellement abouti** .

La prochaine étape est surtout de **verrouiller la qualité du repo** (tests + documentation), pas de développement moteur.
