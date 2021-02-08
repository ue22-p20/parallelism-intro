---
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
  title: "m\xE9moire partag\xE9e"
---

# Parallélisme à mémoire distribuée

+++

## Les limitations de Python

+++

Dans cette partie nous allons voir comment se déroule en pratique le parallélisme à mémoire partagée, plus communément appelé multi-thread. 

Il faut cependant noter quelque chose d'assez cocasse, c'est que Python n'est pas le langage le plus adapté pour faire du parallélisme à mémoire partagée, il est en fait incapable d'en faire à cause du GIL. C'est qui ce GIL ? C'est le ***Global Interpreter Lock***, et il porte bien son nom car il a pour seul vocation de bloquer l'exécution parallèle de Python. C'est à dire qu'il a pour seul et unique but de s'assurer que lorsqu'un thread travaille les autres sont *bloqués et attendent*.

Je suis sûr que vous n'avez alors qu'une seule question qui vous vient à l'esprit, c'est pourquoi introduire ce GIL alors que toutes les machines disposent maintenant de processeur permettant de faire du multi-threading ? La réponse le plus concise et la plus complète est : **c'est historique**. Bienvenue dans le monde du développement. Pour les curieux vous pouvez suivre ce [lien](https://wiki.python.org/moin/GlobalInterpreterLock) pour avoir plus d'informations. Mais assez grossièrement c'est essentiellement pour prévenir tout problème de *race condition*. 

Néanmoins, malgré les limitations du langage nous pouvons quand même faire des choses qui ressemblent à du multithread avec Python. Nous allons pour cela utiliser dans un premier temps le module `threading` qui permet de générer des threads qui s'exécuteront de manière concurrente, donc sur un seul cœur. Nous verrons ensuite à l'aide du module `multiprocessing` comme générer des process qui eux permettront l'exécution simultanée sur plusieurs cœurs. 

Pour finir je vous montrerai deux exemples de multi-threading faits en C++ afin que vous ayez une vision plus réaliste du fonctionnement.

+++

## Du vrai multi-thread en Python - le module `threading`

+++

Pour commencer nous allons voir comment nous pouvons via le module `threading` de Python faire des choses qui en terme de développement s'apparentent très fortement à du multi-threading classique mais qui en revanche en terme de performance n'offriront pas les résultats attendus puisque les threads seront tous exécutés sur le même cœur.

+++

### Création et utilisation des threads

+++

Tout d'abord rappelons ce qu'est un `thread`. Un thread, également appelé processus léger, correspond à une partie d'un processus. Il s'agit donc d'un ensemble d'instruction machine.

La différence entre *thread* et *process* se situe au niveau du fait qu'un processus dispose de sa mémoire virtuelle propre (il ne la partage avec personne d'autre) tandis qu'un *thread* fait partie intégrante du processus qui l'a créé et donc n'a pas sa mémoire virtuelle à lui, il utilise celle de son process "père".

Un process peut créer autant de thread qu'il le souhaite cela implique alors que tous les threads vont se partager la même mémoire et donc pouvoir accéder aux mêmes variables en lecture et en écriture. C'est pour cette raison que l'on parle de parallélisme à **mémoire partagée**.

+++

Le processus de création et d'exécution d'un thread peut se définir de la manière suivante : 

1. On crée depuis notre programme un ou plusieurs threads
2. On attache à ces threads des tâches à effectuer
3. On lance l'exécution des threads qui vont alors se mettre à effectuer les instructions qui leurs sont propres
4. On demande dans le programme principal, si besoin, l'attente de la terminaison des threads.

+++

Pour illustrer cela nous allons faire un programme qui va lancer deux threads, chaque thread n'aura pour tâche que d'afficher un message en boucle 10 fois. Pour cela en Python il faut utiliser l'object `Thread` du module `threading`.

```{code-cell} ipython3
from threading import Thread
```

```{code-cell} ipython3
def thread_function(tid):
    """
    La fonction utilisée dans les threads
    tid : le numéro du thread
    """
    for i in range(10):
        print(f"In the thread {tid}")
        
n_thread = 2
## Création des threads en leur specifiant leurs tâches
threads = [ Thread(target=thread_function, args=(i,)) for i in range(n_thread)]
## A ce stade les threads sont créés mais ne font rien 
## On active les threads
for t in threads:
    t.start()

print("Threads are started")
## A ce stade les threads font leur vie et le programme principal continue
## On va maintenant demander à attendre la fin des threads
for t in threads:
    print(f"Join thread")
    t.join()
```

Dans le cas précédent les deux threads exécutent la même fonction avec des arguments d'entrée différents mais il est tout à fait possible de donner aux threads des tâches complètement différentes. Par exemple faisons le cas d'un thread qui va afficher un message pendant qu'un autre écrit un fichier texte.

```{code-cell} ipython3
def task_1():
    for i in range(10):
        print("I am the first thread I do nothing")
        
def task_2():
    with open("/tmp/stupid.txt", "w") as fid:
        fid.write("I am the second thread")
        
threads = [ Thread(target=task_1), Thread(target=task_2)]
for t in threads:
    t.start()
    
for t in threads:
    t.join()

with open("/tmp/stupid.txt", "r") as fid:
    print(f"File content : {fid.read()}")
 
```

Quel intérêt d'avoir un thread qui s'occupe d'écrire un fichier me direz vous ! Et bien beaucoup en fait. Par exemple dans un certain nombre d'applications les étapes d'écriture/lecture (que l'on appelle étape d'IO) peuvent représenter un pourcentage non négligeable du temps d'exécution global de l'application. Il est donc dans ce cas pertinent de déléguer ces tâches à un thread qui s'occupe de ça pendant que le programme principale fait d'autres tâches.

Un autre intérêt majeur de ce genre d'application est dans la conception d'interfaces graphiques. Par exemple dans de nombreuses interfaces il est pertinent de mettre en place un système de sauvegarde automatique, afin de s'éviter les foudres des utilisateurs en cas de plantages/interruptions impromptus de l'interface. Pour mettre en place ce genre de mécanismes la solution standard est de créer un thread qui va s'occuper de faire une sauvegarde de l'état de l'interface de manière régulière.

+++

Pour le moment me direz vous on ne voit pas bien l'aspect mémoire partagé. Et vous avez raison, pour remédier à cela je vous propose tout de suite un exemple d'un programme qui va être chargé de tirer aléatoirement une série de nombres, chaque thread (on a n threads) s'occupera de tirer N/n nombres aléatoires et les rangera dans un tableau commun.

```{code-cell} ipython3
import random

def fill_random( tid, N, tab ):
    offset = tid*N
    for i in range(offset, offset+N):
        tab[i] = tid*100 + random.random()
        
N = 10
nb_threads = 2
N_by_threads = N // nb_threads

res = [None]*N  ### On crée ici le tableau commun que chaque thread va venir compléter
threads = [ Thread(target=fill_random, args=(i,N_by_threads,res)) for i in range(nb_threads) ]

for t in threads:
    t.start()
    
for t in threads:
    t.join()

print(res)
```

Avec cet exemple on a donc deux threads qui opèrent sur une zone mémoire commune. En revanche vous pouvez noter que j'ai bien fait attention à ce que chaque thread écrive dans sa partie du tableau commun car si ce n'est pas le cas les valeurs vont s'écraser entre elles et le résultat sera indéterminé.

+++

### Notion de vérouillage

+++

Mais parfois il n'est pas possible de faire en sorte que chaque thread écrive des données dans une case mémoire distincte de celle de son voisin. Dans ce cas là il existe un mécanisme permettant de garantir le fait que quand un thread "touche" une variable les autres threads pouvant exister n'y touchent pas.

Considérons par exemple le cas d'un programme où tous les threads doivent incrémenter la même variable. Dans ce cas afin de garantir que le programme est ***thread-safe*** il est nécessaire de verrouiller la variable lorsqu'un thread écrit dans cette dernière. Ce verrouillage se fait à l'aide de la méthode `acquire` du `Lock`. **Attention** il est impératif de déverrouiller le lock ensuite sinon votre programme va rester bloqué indéfiniment.

```{code-cell} ipython3
from threading import Lock 

class Counter:
    def __init__(self):
        self._counter = 0
        
    def incr(self):
        self._counter += 1

    def current(self):
        return self._counter
        
def worker( counter, mutex ):
    for i in range(100):
        mutex.acquire()
        counter.incr()
        mutex.release()

        
counter = Counter()
mutex = Lock()
nb_threads = 4
threads = [ Thread(target=worker, args=(counter,mutex)) for _ in range(nb_threads) ]

for t in threads:
    t.start()
    
for t in threads:
    t.join()
    
print(f"Counter = {counter.current()}")
```

Il est important de noter que le `Lock` est unique et partagé entre tous les threads c'est d'ailleurs cela qui lui permet d'assurer le verrouillage des variables entre threads.

+++

### Les limitations

+++

Pour finir sur l'usage du multi-threading faisons un exemple classique qui est le calcul de $\pi$. Pour rappel il existe une zoologie complète de méthodes pour approximer $\pi$ nous allons ici utiliser l'approximation par une intégrale. Cette approche nous permet d'exprimer $\pi$ de la manière suivante :

$$ \pi =  \int_{0}^{1} \frac{4}{1+x^2} $$

C'est joli mais pas très parlant en terme de programmation. En utilisant un schéma d'intégration, type rectangle, nous pouvons approximer cette intégrale de la manière suivante : 

$$ \pi \simeq \sum_{i=0}^{N-1}   \frac{1}{N} \cdot \frac{4}{1+ (\frac{ i + 0.5}{N})^2 } $$

Cette expression nous permet alors de voir comment implémenter notre calcul de $\pi$.

Commençons par implémenter la version séquentielle du calcul de $\pi$. Cela donne la fonction suivante :

```{code-cell} ipython3
def compute_pi_sequential( nbpoint ):
    s = 0
    l = 1./nbpoint
    for i in range(nbpoint):
        x = l * ( i + 0.5 )
        s += l * ( 4. / (1. + x**2 ) )
    return s

pi_estimated = compute_pi_sequential(100000000)
print(f"PI = {pi_estimated}")
from math import pi
print(f"math.pi = {pi} => error = {abs(pi - pi_estimated)/pi * 100}%")
```

On obtient donc une estimation de la valeur de $\pi$ vraie à $10^{-11}$ donc relativement bonne. Regardons maintenant le temps que met cette estimation à être réalisée.

```{code-cell} ipython3
%timeit compute_pi_sequential(100000000)
```

On constate donc qu'il nous faut environ 15 secondes pour calculer une approximation de $\pi$ précise à $10^{-11}$.

+++

Maintenant réalisons le même calcul mais en utilisant des **threads**. Pour cela il suffit de séparer notre somme en autant de sommes qu'il y a de threads.


$$ \pi \simeq \sum_{k=0}^{n_{thread}-1} \left(  \sum_{i=k*\frac{N}{n_{thread}}}^{(k+1)*\frac{N}{n_{thread}}}   \frac{1}{N} \cdot \frac{4}{1+ (\frac{ i + 0.5}{N})^2 } \right) $$

On peut alors à partir de cette expression définir la fonction qui sera utilisée dans chaque *thread* pour calculer sa contribution à $\pi$.

```{code-cell} ipython3
def pi_thread_worker( nbpoint, tid, nbthread, output ):
    s = 0
    l = 1./nbpoint
    start = tid*(nbpoint//nbthread)
    stop  = (tid+1)*(nbpoint//nbthread)
    if tid == nbthread -1: ## C'est le dernier thread
        stop += nbpoint%nbthread
        
    for i in range(start, stop):
        x = l * ( i + 0.5 )
        s += l * ( 4. / (1. + x**2 ) )
    output[tid] = s
```

Nous pouvons alors écrire la fonction `compute_pi_thread` qui va s'occuper de créer et lancer les threads et de faire la somme des contributions de chaque thread.

```{code-cell} ipython3
def compute_pi_thread( nbpoint, nbthread ):
    
    pi_contrib = [0]*nbthread
    threads = [ Thread(target=pi_thread_worker, args=(nbpoint, i, nbthread, pi_contrib)) for i in range(nbthread) ]
    for t in threads:
        t.start()
        
    for t in threads:
        t.join()
    
    return sum(pi_contrib)

pi_est_thread = compute_pi_thread(100000000,2)
print(f"PI = {pi_est_thread}")
from math import pi
print(f"math.pi = {pi} => error = {abs(pi - pi_est_thread)/pi * 100}%")
```

Si maintenant on regarde en terme de temps de calcul pour différents nombres de threads utilisés on obtient le résultat suivant :

```{code-cell} ipython3
%timeit compute_pi_thread(10000000,1)
%timeit compute_pi_thread(10000000,2)
%timeit compute_pi_thread(10000000,4)
```

Alors là, le risque est que vous fermiez le notebook en vous disant que je raconte n'importe quoi depuis le début car les temps sont les mêmes peu importe le nombre de threads et il s'agit du même temps que la version séquentielle. 

Je répondrai à cette remarque (si vous n'êtes pas encore parti m'insulter sur les réseaux sociaux) que je vous avais prévenus ! Python, à cause du fameux GIL, n'est pas capable d'exploiter plusieurs cœurs en multi-threading. Si on schématise ce qu'il se passe cela donne la figure suivante : 

![gil](../media/gil.png)

Donc à chaque fois qu'un thread travaille, les autres sont à l'arrêt à attendre des ressources. C'est ce qui au final conduit à ce temps multi-thread qui est le même qu'en exécution séquentielle.

+++

### Exemple de vrai multi-threading en c++

+++

Pour que vous ne restiez pas frustré par les limitations de Python. Je vous propose de reprendre le calcul de $\pi$ mais en c++ pour vous montrer l'intérêt de la démarche. Voici ci-dessous un code c++ qui correspond exactement à ce que nous avons implémenté en Python.

```{code-cell} ipython3
!cat ../cpp/pi_thread.cpp
```

On compile alors ce code avec gcc en activant le support du c++11 et en faisant le *link* avec la librairie de gestion des threads du système d'exploitation, sous Linux, **pthread**.

```{code-cell} ipython3
!g++ -std=c++11 ../cpp/pi_thread.cpp -lpthread -o ../cpp/pi_thread
```

```{code-cell} ipython3
!/usr/bin/time --format "%E elapsed %PCPU" ../cpp/pi_thread 1 display
```

```{code-cell} ipython3
!/usr/bin/time --format "%E elapsed %PCPU" ../cpp/pi_thread 2 
!/usr/bin/time --format "%E elapsed %PCPU" ../cpp/pi_thread 3
!/usr/bin/time --format "%E elapsed %PCPU" ../cpp/pi_thread 4
```

On constate donc que dans l'implémentation c++ on a bien un véritable apport du multi-threading en terme de temps de calcul, puisque l'on a une réduction notable du temps de calcul avec le nombre de threads.

On peut également remarquer au passage que le pourcentage de CPU utilisé augmente proportionnellement au nombre de threads ce qui implique donc que notre programme utilise plusieurs cœurs du processeur.

+++

## Du vrai-faux multi-thread en Python - le module `multiprocessing`

+++

Nous venons de voir que le multithread standard en Python n'apporte rien en terme de performance et ne permet pas d'exploiter les architectures multi-cœurs des processeurs modernes.

La question qui se pose alors est : Comment fait-on pour exploiter à fond notre processeur ? 

La solution est simple : plutôt que d'utiliser des **threads** nous allons utiliser des **process**. La différence c'est que dans Python le GIL bloque l'exécution concurrente de différents threads mais en revanche il n'a pas son mot à dire en ce qui concerne les process. 

Mais c'est génial du coup !! Pourquoi ne pas avoir parlé des **process** dès le début et nous prendre la tête avec les threads qui dans Python sont limités ?

Pour la simple est bonne raison que le multi-processing ce n'est plus du parallélisme à mémoire **partagée**... En effet la différence notable entre un paquet de threads et un paquet de process c'est que dans le premier cas la mémoire est commune (il s'agit de la mémoire virtuelle du process qui crée les threads) alors que dans le second cas chaque process à sa propre mémoire et ne peut pas accéder à celle de son voisin…

Fort heureusement les personnes qui ont développé le module `multiprocessing` ont pensé à tout et ont mis en place un mécanisme de queue permettant aux process de communiquer entre eux ! De la même manière ces gens (loués soient-ils) se sont arrangés pour avoir une API dans le module `multiprocessing` la plus proche possible de celle du module `threading`.

```{code-cell} ipython3
from multiprocessing import Process
```

Concrètement si on reprend le tout premier exemple, celui où deux threads affichent en boucle un message et qu'on l'adapte en multiprocessing cela donne :

```{code-cell} ipython3
def process_function(tip):
    """
    La fonction utilisée dans les process
    tid : le numéro du process
    """
    for i in range(10):
        print(f"In the process {tip}")
        
        
n_process = 2
## Création des threads en leur specifiant leurs taches
procs = [ Process(target=thread_function, args=(i,)) for i in range(n_process)]
## A ce stade les threads sont créés mais ne font rien 
## On active les threads
for p in procs:
    p.start()

print("Process are started")
## A ce stade les threads font leur vie et le programme principal continue
## On va maintenant demander à attendre la fin des threads
for p in procs:
    print(f"Join process")
    p.join()
```

La première remarque à faire est que dans ce cas les différences avec la version multithread sont plus que minimes, il suffit de remplace `Thread` par `Process` et ça fonctionne.

Ensuite on peut constater que l'affichage se fait de manière complètement aléatoire ce qui est caractéristique de plusieurs process s’exécutant en même temps. Bien évidemment comme pour le module `threading` il est tout à fait possible d'affecter à chaque process des tâches différentes. 

Vous pouvez pour vous entraîner, écrire l'exemple où un thread affiche un message tandis que l'autre écrit dans un fichier texte, en mode multiprocessing.

+++

Reprenons maintenant l'exemple plus intéressant de plusieurs threads chargés de remplir un tableau de nombres aléatoires.  La transposition de cet exemple avec le module `multiprocessing` donne le code suivant :

```{code-cell} ipython3
import random
def fill_random( tid, N, tab):
    offset = tid*N
    for i in range(offset, offset+N):
        tab[i] = tid*100 + random.random()
    print(f"Proc {tid} tab = {tab}")

        
N = 10
nb_process = 2
N_by_process = N // nb_process

res = [None]*N  ### On crée ici le tableau commun que chaque process va venir compléter 
procs = [ Process(target=fill_random, args=(i,N_by_process,res)) for i in range(nb_process) ]

for p in procs:
    p.start()
    
for p in procs:
    p.join()

print(res)
```

Et là c'est le drame... le tableau n'est pas rempli alors que lorsqu'on l'affiche au niveau des process il y a bien des valeurs dedans !!!!

Et bien c'est normal, je vous l'ai dit la différence entre un thread et un process se joue au niveau de la mémoire, qui est commune entre un thread et le process qui l'a créé mais différente entre un process fils et son process père. 

Donc dans ce cas il va falloir faire un peu plus de modifications pour que notre programme donne le bon résultat.

+++

Pour obtenir le bon résultat nous allons devoir utiliser l'objet `Queue`. Celui-ci permet de créer un espace de stockage commun entre le process père et les process fils qu'il a créés. Cela va ainsi nous permettre de faire remonter au process père les valeurs générées par les sous-process.

Dans les faits l'objet `Queue` n'est pas vraiment un espace mémoire partagé. Il s'agit en réalité d'un canal de communication permettant aux différents process de s'échanger des données. L'utilisation de la `Queue` est très simple et se fait uniquement à l'aide de deux méthodes : 

* `put` qui permet à un process de stocker une donnée
* `get` qui permet de récupérer la dernière donnée stockée

Il existe également la méthode `empty` que l'on va utiliser et qui permet de vérifier s'il reste des données dans la queue ou non.

```{code-cell} ipython3
from multiprocessing import Queue

import random
def fill_random( tid, N, q, n_proc):
    offset = tid*N
    
    
    ret = [0.]*(N*n_proc)
    
    for i in range(offset, offset+N):
        ret[i] = tid*100 + random.random()
    q.put( ret )
        
N = 10
nb_process = 2
N_by_process = N // nb_process

res = Queue()
procs = [ Process(target=fill_random, args=(i,N_by_process,res, nb_process)) for i in range(nb_process) ]

for p in procs:
    p.start()
    
for p in procs:
    p.join()
    
tabs = []
while not res.empty():
    tabs.append( res.get() )
    print(f"From process : {tabs[-1]}")

final = []
for x in zip(*tabs):
    final.append( sum(x) )
    
print(f"Final tab: {final}")
```

On constate donc que le code précédent permet de faire exactement ce que l'on souhaite en revanche, je vous l'accorde, cela nécessite un peu plus de code notamment à la fin pour reconstruire le tableau final. C'est le prix à payer pour pouvoir exploiter réellement les différents cœurs de votre processeur avec Python: ***no pain, no gain***.

Certains d'entre vous auront peut-être remarqué aussi que le code précédent à également un petit désavantage en terme d'occupation mémoire. En effet dans chaque sous-process la première chose que je fais c'est recréer le tableau complet, y compris les zones du tableau qui ne concernent pas le process courant. Donc au lieu d'avoir seulement $N$ entiers à stocker en mémoire et à partager j'ai $N_{process}*N$ entiers, ce qui dans le cas d'un grand nombre de process et de tableaux très grands peut être très très pénalisant.

+++

**Exercice** : refaites une version précédente du problème de remplissage d'un tableau aléatoire sans avoir le problème de l'occupation mémoire qui augmente.
***Indice*** : il y a plusieurs manières de faire ça, uniquement avec ce qui est présenté ici et un peu de débrouillardise. Ou alors en étant encore plus malin et en allant faire un tour dans la documentation du module `multiprocessing`.

+++

#### Les locks ?

+++

Et bien les lock il y en a dans le module `multiprocessing` mais il ne servent à rien. En effet aucun risque que deux process écrivent dans la même case mémoire en même temps vu qu'ils n'ont pas accès aux mêmes zones mémoires ! 


Je nuance quand même ma réponse en précisant que les `Lock` de multiprocessing ont quand même une raison d'être. C'est dans le cas où les sous-process doivent interagir avec l'extérieur. Par exemple si je demande à tous mes sous-process d'ouvrir un fichier, d'écrire une ligne dedans et de le refermer, et bien sans le mécanisme de verrouillage ça va plutôt très mal se passer. Alors qu'en mettant un petit lock au moment de l'ouverture et écriture du fichier et bien ça va fonctionner. Cela donnerait par exemple quelque chose du genre :

```{code-cell} ipython3
from multiprocessing import Lock
import random
def worker(pid, fname, lock):
    count = 0
    while random.random() > 0.1:
        ## Some very complex computation
        count += 1
    lock.acquire()
    with open(fname, "a") as fid:
        fid.write(f"Process {pid} count = {count}\n")
    lock.release()
    
n_procs = 2
fname = "/tmp/bidon.txt"

with open(fname, "w") as fid:
    fid.write("")  ## Juste pour effacer le contenu du fichier s'il existe déjà
    

lock = Lock()
procs = [ Process(target=worker, args=(i, fname, lock)) for i in range(n_procs) ]
for p in procs:
    p.start()
    
for p in procs:
    p.join()

with open(fname) as fid:
    print("File content : ")
    print(fid.read())
    
```

Ainsi en utilisant le `Lock` on a pu écrire dans le même fichier depuis deux process différents sans faire de catastrophe. Il faut cependant faire attention à l'usage abusif des `Lock` car si on met des verrouillages partout l'aspect multicœurs perd de son intérêt car le principe même du lock c'est de faire en sorte que lorsqu'un process fait quelque chose tous les autres attendent. Donc parfois il vaut mieux faire des choses un peu plus coûteuses en calcul mais sans `Lock`. C'est problème dépendant.

+++

#### Retour sur le calcul de $\pi$

+++

Pour finir ce tour d'horizon sur les processus pour exploiter l'architecture multi-coeurs de votre ordinateur revenons ur notre problème de calcul d'une approximation de $\pi$. On rappelle que $\pi$ peut se calculer numériquement de la manière suivante : 


$$ \pi \simeq \sum_{k=0}^{n_{thread}-1} \left(  \sum_{i=k*\frac{N}{n_{thread}}}^{(k+1)*\frac{N}{n_{thread}}}   \frac{1}{N} \cdot \frac{4}{1+ (\frac{ i + 0.5}{N})^2 } \right) $$

A partir de là nous pouvons facilement transposer le code multi-thread présenté plus haut à du multiprocessing. Cela donnerait la solution suivante :

```{code-cell} ipython3
def pi_process_worker( nbpoint, pid, nbproc, output ):
    s = 0
    l = 1./nbpoint
    start = pid*(nbpoint//nbproc)
    stop  = (pid+1)*(nbpoint//nbproc)
    if pid == nbproc -1: ## C'est le dernier thread
        stop += nbpoint%nbproc 
        
    for i in range(start, stop):
        x = l * ( i + 0.5 )
        s += l * ( 4. / (1. + x**2 ) )
    output.put( s )
```

La seule différence notable entre la fonction `pi_process_worker` et `pi_thread_worker` se situe dans l'argument `output` qui dans la version multithread était une liste Python alors que dans la version multiprocess il s'agit d'une `Queue` dans laquelle on va ranger la contribution à $\pi$ du processus courant.

Nous pouvons alors définir la fonction qui va créer les processus de la manière suivante :

```{code-cell} ipython3
def compute_pi_process(nbpoint, nbproc):
    
    pi_contrib = Queue()
    procs = [ Process(target=pi_process_worker, args=(nbpoint, i, nbproc, pi_contrib)) for i in range(nbproc)]
    for p in procs:
        p.start()
        
    for p in procs:
        p.join()
    
    pi_val = 0.
    while not pi_contrib.empty():
        pi_val += pi_contrib.get()
    return pi_val

pi_est_proc = compute_pi_process(100000000,2)
print(f"PI = {pi_est_proc}")
from math import pi
print(f"math.pi = {pi} => error = {abs(pi - pi_est_proc)/pi * 100}%")
```

Nous pouvons maintenant regarder en terme de temps de calcul ce que l'utilisation du multiprocessing nous apporte. Pour rappel le temps de calcul en séquentiel est le suivant :

```{code-cell} ipython3
%timeit compute_pi_sequential(10000000)
```

En utilisant alors la version multiprocess avec deux sous-process on obtient

```{code-cell} ipython3
%timeit compute_pi_process(10000000,2)
```

On constate donc que l'on a un gain d'un facteur un peu moins de deux entre la version séquentielle et la version 2 process.

Essayons donc avec 4 !

```{code-cell} ipython3
%timeit compute_pi_process(10000000,4)
```

On constate alors que le gain sur 4 cœurs n'est pas au rendez vous !! Encore une fois vous commencez à vous dire que je suis un rigolo !! Mais non ne vous inquiétez pas c'est normal. Le problème ici c'est que sur mon ordinateur portable je n'ai que 4 cœurs. Or j'ai déjà des ressources occupées par mon firefox pour écrire ce notebook, le thunderbird pour lire tous les mails passionnants... Donc même si sur le principe j'ai de quoi lancer 4 process tranquillement, dans les faits les ressources ne sont pas disponibles donc les process attendent un peu leur tour pour avoir du CPU d'où l'absence de gain sur 4 cœurs.

+++

Pour vous prouver que je ne dis pas n'importe quoi j'ai repris exactement le même code et je l'ai fait tourner sur un nœud du cluster du Centre des Matériaux. Pour information les nœuds du cluster disposent de deux processeurs Intel à 12 cœurs chacun. Je suis donc monté jusqu'à 24 processus en parallèle et voici ci-dessous les résultats :

+++

1 process  => 0:01.83elapsed 99%CPU  
2 process  => 0:01.00elapsed 186%CPU  
4 process  => 0:00.52elapsed 355%CPU  
6 process  => 0:00.39elapsed 497%CPU  
8 process  => 0:00.31elapsed 624%CPU  
10 process => 0:00.28elapsed 717%CPU  
12 process => 0:00.24elapsed 826%CPU  
14 process => 0:00.22elapsed 914%CPU  
16 process => 0:00.20elapsed 1010%CPU  
18 process => 0:00.20elapsed 1103%CPU  
20 process => 0:00.19elapsed 1161%CPU  
22 process => 0:00.18elapsed 1228%CPU  
24 process => 0:00.17elapsed 1277%CPU

+++

On constate alors avec les temps de calcul mesurés sur le cluster que le multiprocessing apporte vraiment quelque chose puisque l'on arrive à atteindre un facteur presque 11 sur le temps de calcul de $\pi$.

Vous pourriez alors me faire remarquer qu'idéalement le facteur devrait être 24 ! Je vous dirai c'est vrai mais dans ce cas ce n'est pas possible, non pas à cause de la technologie mais à cause du problème. En effet le problème ici est trop "petit" ce qui fait qu'au niveau de chaque sous-process il ne reste plus grand chose à faire et donc on perd quasiment plus de temps à créer les sous-process qu'à les exécuter. Cela se voit très bien notamment au niveau de l'utilisation du CPU. On constate que l'on plafonne à 1200% d'utilisation ce qui implique que l'intensité algorithmique de notre problème est trop faible.
Il y a une manière très simple de remédier à cela dans notre cas. C'est de demander à calculer $\pi$ avec une plus grande précision donc avec plus de points. Prenons par exemple $10^{9}$ points au lieu des $10^7$ utilisés précédemment. Dans ce cas on obtient les nouveaux résultats :

+++

1 process => 2:55.53elapsed 99%CPU  
2 process => 1:27.95elapsed 199%CPU  
4 process => 0:46.03elapsed 390%CPU  
6 process => 0:31.34elapsed 597%CPU  
8 process => 0:23.84elapsed 790%CPU  
10 process => 0:20.61elapsed 945%CPU  
12 process => 0:17.20elapsed 1131%CPU  
14 process => 0:14.77elapsed 1318%CPU  
16 process => 0:12.94elapsed 1513%CPU  
18 process => 0:12.29elapsed 1696%CPU  
20 process => 0:11.08elapsed 1884%CPU  
22 process => 0:10.14elapsed 2078%CPU  
24 process => 0:10.05elapsed 2096%CPU

+++

On obtient donc au mieux un facteur d'accélération de 17. Ce qui est plus honorable que précédemment. Et on le voit notamment au niveau de l'usage du CPU avec un usage de 2000% bien plus intéressant que le 1200% précédent.

+++

Pour conclure sur les aspects de parallélisme process et thread, il est important de noter que ce n'est pas la méthode miracle et que dans de nombreuses applications l'effort nécessaire pour mettre en place ce type de mécanisme n'en vaut pas le gain.

Il faut donc bien faire attention à ce que l'on fait et ne pas se laisser prendre par la folie du parallèle en en mettant partout car vous risquez de vous pénaliser plus qu'autre chose.

Mais en revanche il existe tout une gamme d'applications/problématiques pour lesquelles cela vaut vraiment le coup de faire l'effort de programmation nécessaire !!
