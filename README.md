# Surveillance Frigo

## Objectif
Le module de surveillance frigo est une alarme pour detecter quand la temperature du frigo depasse un certain seuil. Un SMS est envoyé en cas d'alarme.

## Description des composants
Tout tient dans le boitier dans lequel on trouve:
* Une carte Electron de la marque Particle (https://www.particle.io/). C'est elle qui execute le programme et communique avec l'antenne telephonique
* Une batterie pour alimenter l'Electron en cas de panne de courant
* Une antenne pour envoyer et recevoir des SMS
* 3 capteurs identique de type DS1820
* Une carte SIM Free pour envoyer et recevoir des SMS. Son abonnement est gratuit car lié à la Freebox de Fabien. Son numero de telephone est le : 0652328703. Le nombre de SMS est illimité.

Il ne reste qu'à brancher la carte avec le cable USB, comme un telephone portable.

## Installation du module
1. La première chose à faire est d'installer les capteurs dans les frigos et congelateurs. 
Si necessaire, les capteurs peuvent être dévisé du bornier bleu pour faciliter l'installation. 
Les capteurs peuvent être rebranché sur n'importe quel bornier, par contre il faut absolument respecter la couleur des cables (Rouge, Noir, Blanc).
2. Ensuite il faut brancher la batterie sur l'Electron. Attention le connecteur blanc a un détrompeur, ça doit rentrer sans forcer.
3. Enfin il faut brancher le cable USB sur une prise de courant.

## Fonctionnement

### Fonctionnement normal
En fonctionnement normal, le module mesure les trois temperatures toutes les 5min. 
Si une des trois temperatures est en dessus de son seuil, un SMS est envoyé. Un SMS de rappel sera envoyé toutes les heures. 

Quand la temperature redescend en dessous du seuil, un SMS est envoyé pour informer que la situation est normale.

Par défaut le module envoi des SMS au numéro de Basile (06470874xx). C'est modifiable en reprogrammant la carte à distance.

### Panne de capteur
Si un capteur ne répond pas, il sera lu comme -127°C **Note Fabien: je ne suis pas sûr que ça marche. A tester**

### Panne de courant
En cas de panne de courant, un SMS est envoyé pour prevenir. Si le module n'est plus alimenté, il se peut que les frigos non plus! 
Un SMS de rappel sera envoyé toutes les heures

Le module reste alimenté par la batterie pendant au moins 24h, et continue de mesurer les temperatures toutes les 5min. 
Quand la batterie est presque vide (moins de 20%), le module se met en veille, et ne se reveille que toutes les heures pour verifier si la panne de courant est toujours présente.

**Attention: ne pas laisser la batterie se vider completement, cela risque d'endommager et il faudra me l'envoyer par la poste. Pour debrancher le module, il faut enlever le secteur ET débrancher la batterie.**

### Codes couleurs des LEDs
Le module comporte deux LEDs. La première est le témoin de charge de la batterie
* Rouge fixe: batterie en charge
* Rouge clignotant: batterie déconnectée
* Eteinte: batterie chargée

La seconde est le témoin de connection au réseau telephonique:
* Verte ou blanche respirant: le module n'est pas connecté à internet
* Cyan respirant: le module est connecté à internet
* Bleu respirant: le module n'arrive pas à se connecter à l'antenne telephonique
* Rouge clignotant: erreur grave, appeler le SAV!

Pour plus d'info: https://docs.particle.io/guide/getting-started/modes/electron/

## Communication avec le module
On peut envoyer des SMS au module. Il est programme pour répondre à certaines commandes et réagit en conséquence (enlever les guillemets):
* "LireTemperature": Le module renvoi la dernière mesure de temperature pour les 3 capteurs
* "ConnecteCloud": Le module se connecte à internet. C'est utile pour pouvoir reprogrammer la carte à distance. Par défaut il est déconnecté, pour economiser de la batterie.
* "DeconnecteCloud": Le module se deconnecte d'internet.

|Commande|Action|
|---|---|
|LireTemperature|Le module renvoi la dernière mesure de temperature pour les 3 capteurs|
|ConnecteCloud|Le module se connecte à internet. C'est utile pour pouvoir reprogrammer la carte à distance. Par défaut il est déconnecté, pour economiser de la batterie.|
|DeconnecteCloud|Le module se deconnecte d'internet.|

# Fonctions à terminer
- [ ] Regler les seuils. Voir avec Basile
- [ ] Ajouter photo module piour les LEDs et le branchement des capteurs
- [ ] Lisser les temperatures: en cas de depassement d'un seuil, refaire 3 mesures à 1min d'intervalle. Si c'est toujours trop chaud, alors envoyer le SMS. 3*1min suffisant quand Basile revient de Metro?
- [ ] Gerer le cas d'une panne de capteur: on envoi un SMS? Une fois par jour seulement?
- [ ] Changer le numéro de telephone pour mettre celui de Basile
