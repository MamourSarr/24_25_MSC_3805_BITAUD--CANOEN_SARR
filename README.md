# Commande d'un MCC à Hacheur 4 Quadrants

Ce projet vise à développer une solution complète pour la commande d'un moteur à courant continu (MCC) à l'aide d'un hacheur 4 quadrants en utilisant un shell en C sur stm32. Il couvre plusieurs étapes allant de la mise en place d'une communication UART jusqu'à l'asservissement en temps réel.

- Objectifs principaux

    - Shell UART : Réalisation d'un shell pour commander le hacheur.
    - Commande des transistors : Contrôle des 4 transistors du hacheur en commande complémentaire décalée.
    - Acquisition de capteurs : Collecte et traitement des données issues des capteurs.
    - Asservissement en temps réel : Implémentation d'un système d'asservissement pour le MCC.

## TP n°1 : Console UART

La première étape consiste à développer un shell interactif, communicant via la liaison UART de la carte STM32-G474RE. Ce shell permet de tester et de commander les fonctionnalités du système.
Fonctionnalités du Shell

- Écho des caractères : Vérifie et renvoie les caractères reçus pour validation.
    - Commandes disponibles :
        - help : Liste toutes les commandes disponibles.
        - pinout : Affiche toutes les broches connectées et leurs fonctions.
        - start : Démarre l'étage de puissance du moteur (affichage "Power ON").
        - stop : Arrête l'étage de puissance du moteur (affichage "Power OFF").
        - Par défaut, une commande non reconnue affiche "Command not found".

## TP n°2 : Commande MCC basique


Nous avons dans cette partie cherché à commander les 2 bars de pont U et V du hacheur en utilisant 4 PWM (complémentaire 2 à 2). Pour ce faire on utilise un timmer pour genérer les 4 pwm sur 4 channels diferents.
On régle ensuite le prescaller du timer à 8 et Counter period à 1024, afin que le timer fonctionne à 20 kHz celon la formule suivante :
ftimer​ = ftimer_clock​​/((Prescaler+1)×(ARR+1))
Nous avons également réglé le "Counter Mode" sur Center Aligned afin de que le timer conte puis déconte.
On régle également le dead time à 200ns afin de ne pas créer d'appel de courant dans les transistor ce qui pourrait les détruire. 

Nous avons par la suite implémenté un fonction speed appelable dans le shell permettant de régler la vitesse du moteur. 
Cette fonction modifie en fait le rapport cyclique des pwm avec la fonction __HAL_TIM_SET_COMPARE.

## TP n°3 : Commande en boucle ouverte, mesure de vitesse et de courant


Dans cette partie nous avon commencé par modifier la fonction start afin qu'elle démare les pwm mais à un rapport ciclique de 50% ce qui ne fait pas tourner le moteur.
Nous avons ensuite cherhcé à améliorer la fonction speed afin de minimiser les appelles de courant dans le moteur quand l'utilisateur cherche à modifier la vitesse du moteur (le probléme étant que en modifiant brutalement les rapport ciclique des pwm celci créer des appels dangereux de courant dans les transistors.
Pour ce faire nous avons implémenté à l'aide de boucle for une rampe permettant de modifier progressisvement le rapport ciclique des PWM en incrémentant ce rapport cyclique toutes les 10 ms. 

Nous nous sommes par la suite attelé à la mesure du courant dans les bars de pont du hacheur.
Pour ce faire nous avons implémenter une fonction ADC appelable deupuis le shell afin de mesurer ce courant.
Cette mesure s'éffectue avec l'ADC en pooling. Nous avons par la suite modifié cette fonction afin qu'elle effectue des mesures de courant à intervalle de temps régulier. Pour ce faire on vas déclencher les mesures au moment ou le timer des pwm commence à déconter, c'est le moment ou le courant et le plus stable car c'est l'instant le plus éloigné des commutation des transistors. cette mesure est alors convertie par l'ADC en valeur numéérique qui est ensuite stocké dans le DMA.

