# État de l’art — Génération de la carte

---

## 1. Idée de départ : une table de floats

Au début, la génération était pensée de manière très directe :

- une grille 2D de `float`
- chaque valeur représente un niveau “naturel” de la carte
- interprétation des valeurs ensuite en types de terrain

L’objectif était uniquement d’obtenir une base exploitable, sans chercher encore à reproduire des modèles classiques comme Perlin noise ou autres algorithmes connus. L’idée de départ ne s’appuyait pas sur une méthode académique précise.

---

## 2. Évolution de la réflexion

En avançant, deux problèmes apparaissent :

- une seule variable ne suffit pas à générer des biomes variés
- le rendu devient trop uniforme et peu lisible

J’ai donc enrichi le modèle en introduisant une deuxième dimension implicite : une forme de “sécheresse” ou d’humidité, en plus de l’altitude.

Sans le formaliser au départ, cela revient à :

- une première table de float pour l’altitude
- une seconde table de float pour l’humidité

---

## 3. Comparaison avec les approches existantes

En parallèle, j’ai regardé les solutions classiques utilisées en génération procédurale :

- **Perlin noise** : génération de valeurs continues lisses, très utilisée pour créer des terrains naturels (montagnes, océans) avec des transitions progressives.
- **Simplex noise** : variante plus moderne et plus performante que Perlin, donnant des résultats similaires mais avec moins d’artefacts et une meilleure efficacité en grande dimension.
- **Diamond-square** : algorithme fractal basé sur une subdivision récursive, permettant de générer des heightmaps avec une structure de relief plus “rugueuse” et naturelle.

Mais ces approches ne correspondaient pas immédiatement à ma démarche :

- elles sont plus abstraites
- moins directement contrôlables
- plus complexes à intégrer dans un système gameplay déjà structuré

J’avais déjà une logique en tête, donc je suis resté sur une approche plus directe et paramétrique.

---

## 4. Finalisation de l’approche retenue

La solution finale repose sur :

- une heightmap (`hmap`)
- une map d’humidité (`mmap`)
- une seed pour garantir la reproductibilité
- des règles de classification par seuils

Extrait central :

```c
float h = hmap[y * W + x];
float m = mmap[y * W + x];

if (h < 0.28f) {
    t = TERRAIN_WATER;
} else if (h < 0.40f) {
    t = (m > 0.60f) ? TERRAIN_TUNDRA : TERRAIN_DESERT;
} else if (h < 0.60f) {
    t = (m > 0.60f) ? TERRAIN_FOREST : TERRAIN_PLAIN;
} else if (h < 0.80f) {
    t = (m > 0.55f) ? TERRAIN_FOREST : TERRAIN_PLAIN;
} else {
    t = TERRAIN_MOUNTAIN;
}
```
