# Asteroids

Projet astéroid - Programmation avancée C++ M1 SIL.
NASSABAIN Marco, WENDLING Nicolas

## Installation

**IMPORTANT - Si vous lancez l'installation complète, c'est à dire le setup qui installe SDL2 et SDL2_image, il est normal que l'installation soit lente**

Le programme est testé et compile sur `Ubuntu 16.04` et `Ubuntu 18.04`. Nous n'avons pas accès aux machines de la fac mais elles sont sur `Ubuntu 18.04` donc normalement ça devrait marcher

```
# build sdl2 & sdl2_image dans le projet & build jeu - peut prendre quelques minutes
make

# ALTERNATIVE: si vous avez deja sdl2 et sdl2_image installé:
make setup-asteroids
```

## Comment jouer

Une fois le jeu lancé cliquer sur **Entrée** pour commencer le jeu.

Touches:
* Z - accélérer
* Q - tourner à gauche
* D - tourner à droite
* Espace - lancer roquettes
* LMaj - téléporter

## Règles

Le jeu se termine quand vous avez 0 vies. Le jeu démarre avec 3 vies. Pour chaque 1000 points gagnés, vous gagnez une vie, avec un maximum de 5 vies.

Essayez d'éviter les astéroïdes et d'en détruire le maximum possible, afin d'obtenir le highscore!
