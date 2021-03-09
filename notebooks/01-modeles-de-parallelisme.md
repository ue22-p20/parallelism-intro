---
celltoolbar: Slideshow
jupytext:
  cell_metadata_filter: all,-hidden,-heading_collapsed,-run_control,-trusted
  notebook_metadata_filter: all,-language_info,-toc,-jupytext.text_representation.jupytext_version,-jupytext.text_representation.format_version
  text_representation:
    extension: .md
    format_name: myst
kernelspec:
  display_name: Python 3
  language: python
  name: python3
nbhosting:
  title: "mod\xE8les de parall\xE9lisme"
---

+++ {"slideshow": {"slide_type": "slide"}}

# Modèles de parallélisme

+++ {"slideshow": {"slide_type": "slide"}}

## Quatre modèles bien établis

+++ {"slideshow": {"slide_type": "subslide"}}

Nous avons donc vu qu'il existe, en terme de moyens de calculs, différentes échelles :
* l'échelle de l'unité de calcul (le cœur)
* l'échelle du processeur qui comporte plusieurs cœurs
* l'échelle de l'ordinateur ou du nœud qui comporte plusieurs processeurs
* l'échelle du cluster qui est constitué d'un ensemble de nœuds.

+++ {"slideshow": {"slide_type": "subslide"}}

Afin d'exploiter toutes ces échelles, diverses approches et solutions techniques ont été développées au fil du temps. Nous verrons cela peu après. Mais tout d'abord parlons modèle de parallélisme. En effet, suivant les applications cible et la finalité visée, les usages des architectures parallèles diffèrent beaucoup.

Il est communément admis qu'il existe 4 modèles de parallélisme différents :
* Le modèle ***Single Instruction Single Data*** qui est en fait le cas particulier du calcul séquentiel classique. C'est-à-dire une unité de calcul traite une donnée
* Le modèle ***Single Instruction Multiple Data*** qui correspond au cas des instructions de vectorisation. Donc littéralement : le CPU sait travailler avec des vecteurs. C'est-à-dire qu'une unité de calcul applique une instruction à plusieurs données en même temps ce qui donne plusieurs résultats. 
* Le modèle ***Multiple Instruction Multiple Data*** qui correspond au cas des processeurs multi-cœurs et également au cas du cluster. C'est-à-dire que différentes unités de calcul traitent des données différentes. Il s'agit donc du cas le plus général.
* Le modèle ***Multiple Instruction Single Data***, modèle plus atypique et peu utilisé

+++ {"slideshow": {"slide_type": "subslide"}}

Dans la suite de ce cours, nous nous concentrerons uniquement sur le modèle de type ***Multiple Instructions Multiple Data***. Car il s'agit du modèle le plus répandu pour ce qui concerne le calcul parallèle. 

Je tiens toute fois à préciser que le modèle ***SIMD*** est de plus en plus en vogue et permet sur les générations récentes de processeurs, d'avoir des gains de performances assez remarquables. En revanche ce modèle se situe à un niveau de granularité relativement fin puisqu'il opère au niveau du cœur et ne travaille qu'avec des données de petite taille (en moyenne 8 données de 32s bit en simultané).

Pour ceux qui s'intéresseraient aux aspects vectorisation des instructions, je vous invite à [regarder cette présentation de l'IDRIS](http://www.idris.fr/media/formations/simd/idrissimd.pdf)

+++ {"slideshow": {"slide_type": "slide"}}

## Un modèle mais deux cas de figure

+++ {"slideshow": {"slide_type": "subslide"}}

Ainsi nous avons choisi notre modèle théorique, le ***MIMD***. En revanche il faut faire attention au fait que l'on a deux cas de figure bien distincts à considérer et cela vient de l'aspect multi-échelles des architectures de calcul modernes.

+++ {"slideshow": {"slide_type": "subslide"}}

Le **premier cas** est celui qui se limite à l'échelle de l'ordinateur ou du nœud de calcul. Dans ce cas il y a un ou plusieurs processeurs (peu importe) comprenant plusieurs cœurs. Pas de difficulté majeure ici, chaque cœur exécute ses instructions, récupère ses données en RAM, prend les données de son voisin s'il en a besoin, ... C'est ce que l'on va appeler notamment du multi-threading. Attention : ne pas confondre multi-threading (qui est le fait d'utiliser en parallèle plusieurs cœurs d'un même processeur dans un programme) et hyper-threading qui est une technologie Intel permettant de multiplier par deux le nombre d'unités de calcul par rapport au nombre de cœurs du processeur). Concrètement l'hyper-threading vous fait croire que vous avez 4 cœurs sur votre ordinateur, alors qu'en réalité vous n'en avez que 2.

+++ {"slideshow": {"slide_type": "subslide"}}

Là où les choses se compliquent un peu c'est dans **le second cas** qui se situe à l'échelle du cluster donc de plusieurs ordinateurs connectés entre eux. Au début tout se passe bien, chaque cœur (sur des machines différentes) exécute ses instructions, récupère ses données en RAM, ~~prend les données de son voisin~~ ... Ah oups il ne peut pas prendre les données de son voisin... Ah bon, mais pourquoi me direz vous ? Pour une raison simple voire stupide : les données elles sont où ? Elles sont dans la mémoire vive, or dans ce cas chaque instruction tourne sur des machines physiques différentes donc avec des mémoires vives différentes !! Eh oui voilà le drame : une instruction sur un cœur de l'ordinateur A ne peut pas aller piocher des données dans la RAM de l'ordinateur B.

+++ {"slideshow": {"slide_type": "subslide"}}

Il s'agit là de la grande vérité : il y en fait deux types de parallélisme dans le modèle ***MIMD***. 

1. Le parallélisme dit à mémoire partagée, dans ce cas toutes les unités de calcul (cœurs) sont reliées à la mémoire vive et peuvent donc facilement piocher dans les données des voisins. 
2. Le parallélisme dit à mémoire distribuée, dans ce cas toutes les unités de calcul ont leurs propres mémoires vives indépendantes de celles des voisins.

+++ {"slideshow": {"slide_type": "subslide"}}

Mais du coup on fait comment en mémoire distribuée si on a besoin d'une valeur d'un voisin ? Eh bien il faut alors faire des opérations de communication entre les unités de calcul. C'est-à-dire ? Eh bien à un moment l'ordinateur A va dire : tiens je vais envoyer la valeur de la variable `ma_super_variable` à l'ordinateur B. Si ce n'était que cela ce serait trop simple, car la difficulté c'est que quand l'ordinateur A envoie sa variable à B il faut que dans le même temps B se dise : tiens, je vais recevoir une variable de A donc je me prépare. Et ça il n'y a pas de miracle il faut que ce soit prévu à la conception du programme. 

+++ {"slideshow": {"slide_type": "subslide"}}

Mais rassurez vous pour mettre en place ce système d'envoi/réception de variables il existe des protocoles bien définis et relativement simples. Pendant plusieurs années les choix ont été multiples mais depuis maintenant plus de 10 ans on peut dire qu'il n'existe qu'une seule manière raisonnable de faire cela c'est en utilisant le protocole ***Message Passing Interface*** (MPI). Il s'agit d'une norme qui définit comment l'échange de données entre processus distants doit être réalisée. Ensuite cette norme est implémentée dans différents standards (Open Source ou propriétaire) et disponible dans de nombreux langages de programmation. Nous verrons dans le dernier notebook comment utiliser MPI en Python pour faire de la programmation parallèle. 

+++ {"slideshow": {"slide_type": "subslide"}}

Au passage j'aimerais insister sur un point. Avec l'utilisation de MPI on peut procéder à l'échange de données entre processus distants. Mais à votre avis, est-ce que ce transfert est instantané ? La réponse est bien évidemment non. J'ajouterai même que le point clé dans l'exploitation des architectures fortement parallèles de type cluster, c'est le réseau entre les nœuds de calcul. En effet une donnée qui transite de A vers B passe par le réseau. Or si le réseau n'est pas assez rapide alors le temps de transfert peut devenir prépondérant dans les applications. Par exemple pour illustrer cela je vous propose de regarder le tableau suivant qui évalue les performances réseau entre le cluster moderne du centre de matériaux et notre ancien cluster (aujourd'hui en retraite). Pour réaliser ce test on fait juste un ping-pong d'un paquet de double. 

| Vector size | Time exchange (modern network) | Time exchange (old network) |
|-------------|--------------------------------|-----------------------------|
| 1           |             0.43               |   0.0019                    |
| 10          |             1.21e-5            | 5.69e-5                     |
| 100         |               0.00034231       | 0.002723222                 |
| 1000        |              0.0002660         | 0.00471687                  |
| 10000       |             0.0006988          | 0.016027                    |
| 100000      |            0.005841            | 0.1366                      |
| 10000000    |          0.0567                | 1.3633                      |
| 100000000   |            0.654               |  13.601                     |
| Better data rate |               2819 Mo/s   | 117 Mo/s                    |


On constate alors que sur les gros volumes de données on obtient des facteurs 20 en terme de rapidité de transfert, ce qui n'est pas négligeable.

+++ {"slideshow": {"slide_type": "subslide"}}

Je tiens également à vous faire remarquer que ce n'est pas parce que j'ai dit que le parallélisme à mémoire distribuée était plus délicat à mettre en œuvre, qu'il faut en déduire que la version mémoire partagée est triviale ! Ce n'est pas le cas elle soulève un certain nombre de problématiques et c'est ce que nous allons voir dans le notebook qui vient juste après. 

Une question que vous pouvez alors vous poser est : pourquoi se compliquer la vie avec le parallélisme distribué, qui nécessite des infrastructures plus lourdes notamment en terme de réseau ? Pour diverses raisons en fait : 

* Le nombre de cœurs pouvant raisonnablement être contenu sur une seule machine est aujourd'hui limité à une grosse centaine environ
* De la même manière la mémoire vive est une limitation significative. L'intérêt de dispatcher un gros problème sur un ensemble de machines distinctes permet de répartir la charge en terme d'occupation mémoire. En effet pour les problématiques de simulation numérique, les ressources en terme de mémoire vive peuvent vite devenir monstrueuses. Si on reprend par exemple le calcul de l'aube de turbine que je vous ai montré dans l'introduction, la réalisation de ce calcul en séquentiel nécessiterait une machine disposant d'un peu plus de 700 Go de RAM. Ce type de machine existe de nos jours mais ces machines restent quand même relativement rares et excessivement chères.
