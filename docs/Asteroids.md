# PROJET PROGRAMMATION AVANCEE - ASTEROIDS
## WENDLING Nicolas, NASSABAIN Marco

## Ecrans

Le jeu aura deux écrans:
* Ecran de menu
* Ecran de jeu

Ils sont très similaires et donc on peut se contenter d'implémenter un objet, avec une variable `running`. Si cette variable est vraie, on considère qu'on est sur l'écran du jeu. Sinon, on considère qu'on est sur l'écran de menu.

## Ecran de menu

L’écran de menu sera le même comme le jeu original. Il y aura quelques astéroïdes dans le fond qui vont bouger comme dans le jeu. Le titre **ASTEROIDS** sera affiché au milieu avec le highscore. La phrase *Press Enter to Play* va clignoter. Si l’utilisateur appuis sur la touche Entrée, on enlève le contenu de l’écran et on passe à l’écran de jeu.

## Ecran de jeu

Au début d’une partie on affichera le score ainsi que le nombre de vies que possède le joueur (3). Ensuite le vaisseau du joueur sera ajouté et des astéroïdes vont commencer à se rajouter.

L’utilisateur va pouvoir naviguer son vaisseau à l’aide des touches Z, Q et D. Les touches Q et D vont changer l’orientation du vaisseau en faisant une rotation, et la touche Z va permettre au vaisseau d'accélérer. En cliquant sur la touche espace, le vaisseau va tirer des balles. Les balles sont infinies mais il n’est pas possible de tirer sans arrêt. L’utilisateur peut cliquer sur la touche MAJ pour se téléporter à un endroit aléatoire sur l’écran, avec une possibilité de toucher un astéroïde.

L’écran est circulaire c’est à dire que si un objet arrive au bord de l’écran il réapparaît de l’autre côté. Ceci est vrai pour les astéroïdes et le vaisseau du joueur, mais pas pour les balles.

Il existe trois types d'astéroïdes selon leur taille. Si touchés par une balle du joueur, les astéroïdes les plus grands seront transformés en deux astéroïdes de taille inférieure. Si ces astéroïdes sont touchés ils sont transformés en deux astéroïdes petits. Ces astéroïdes seront détruits s'ils sont touchés par une balle. La destruction d’un astéroïde remporte 20, 50 et 100 points respectivement. Chaque “niveau” aura un nombre d’astéroïdes. Quand tous les astéroïdes sont détruits on chargera un nouvel ensemble d'astéroïdes.

Si l’utilisateur est touché par un astéroïde son vaisseau est détruit et une vie est soustraite. Le joueur a perdu si il n’a plus de vies. Après chaque 10,000 points, l’utilisateur gagne une vie, avec un maximum de cinq vies.

Après avoir implémenté cela nous allons implémenter les OVNI.


## Spécification

Le noyau du jeu sera séparé de SDL. Ceci nous permettra d’avoir une distinction avec le jeu ainsi que d’abstraire les détails techniques d’affichage et de gestion des touches d’utilisateur pour pouvoir les modifier facilement.

### Noyau

Les objets de base seront les objets de classe **Object**. Chaque objet possède doit contenir sa **hitbox** (position + dimensions), **orientation**, **vitesse** et **accélération** (Penser à créer une variable de classe maxSpeed). La classe possèdera deux fonctions virtuelles **update()** et **display()** qui doivent être implementés par les classes qui héritent.
    
La classe **Asteroid** va hériter de cette classe. Sa vitesse sera constante (accélération nulle). et son orientation ne changera pas. Ces données vont être générés automatiquement et aléatoirement à l’instant de la création. L'astéroïde possède aussi un indicateur de sa taille (1, 2 ou 3, ou macro), ainsi qu'un getter publique **getSize()** qui permettra au jeu de gérer le traitement de sa destruction.

La classe **Spaceship** va aussi hériter de la classe Object. Au moment de l'instanciation ceci est initialisé avec des données nulles (accélération, etc), et avec trois vies. Cette classe contiendra aussi le score. Cette classe doit fournir des fonctions publiques **accelerate(), decelerate(), turn(rot), oneUp(), destroy(), teleport(x, y), shoot(), addPoints(p), getScore()**.

La classe **Rocket** va hériter de la classe Object. Ceci va être initialisé à l’aide de la classe Spaceship pour avoir l’orientation. La vitesse est fixe.

Ces objets **seront stockés dans un tableau**. Elles seront **triés par leur position sur l’axe X**, pour permettre d’optimiser l’algorithme de détection de collision. Ce tableau sera stocké dans l’objet **Game**. Cette classe contiendra aussi le highscore. Cette classe va fournir des fonctions qui vont être appelés dans le main: **init(), start(), stop()**. Elle doit aussi contenir des variables internes qui vont gerer le nombre d'asteroides et le niveau.
