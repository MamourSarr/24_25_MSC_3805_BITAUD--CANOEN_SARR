# Commande d'un MCC à Hacheur 4 Quadrants

Ce projet vise à développer une solution complète pour la commande d'un moteur à courant continu (MCC) à l'aide d'un hacheur 4 quadrants en utilisant un shell en C sur STM32. Il couvre plusieurs étapes, allant de la mise en place d'une communication UART jusqu'à l'asservissement en temps réel.
- Objectifs principaux

    - Shell UART : Réalisation d'un shell pour commander le hacheur.
    - Commande des transistors : Contrôle des 4 transistors du hacheur en commande complémentaire décalée.
    - Acquisition de capteurs : Collecte et traitement des données issues des capteurs.
    - Asservissement en temps réel : Implémentation d'un système d'asservissement pour le MCC.

## TP n°1 : Console UART

La première étape consiste à développer un shell interactif, communicant via la liaison UART de la carte STM32-G474RE. Ce shell permet de tester et de commander les fonctionnalités du système.
- Fonctionnalités du Shell

    - Écho des caractères : Vérifie et renvoie les caractères reçus pour validation.
        - Commandes disponibles :
           - help : Liste toutes les commandes disponibles.
            - pinout : Affiche toutes les broches connectées et leurs fonctions.
            - start : Démarre l'étage de puissance du moteur (affichage "Power ON").
            - stop : Arrête l'étage de puissance du moteur (affichage "Power OFF").
            - Par défaut, une commande non reconnue affiche "Command not found".

## TP n°2 : Commande MCC basique

Dans cette partie, nous avons cherché à commander les 2 bras de pont U et V du hacheur en utilisant 4 PWM (complémentaires 2 à 2). Pour ce faire, nous utilisons un timer pour générer les 4 PWM sur 4 chanels différents.
Nous réglons ensuite le prescaler du timer à 8 et le Counter Period à 1024, afin que le timer fonctionne à 20 kHz selon la formule suivante :
ftimer = ftimer_clock/((Prescaler+1)×(ARR+1))

Nous avons également réglé le "Counter Mode" sur Center Aligned, afin que le timer compte puis décompte.
Nous réglons également le dead time à 200 ns afin d'éviter de créer des appels de courant dans les transistors, ce qui pourrait les endommager.

Par la suite, nous avons implémenté une fonction speed, appelable dans le shell, permettant de régler la vitesse du moteur.
Cette fonction modifie en fait le rapport cyclique des PWM à l'aide de la fonction __HAL_TIM_SET_COMPARE.

## TP n°3 : Commande en boucle ouverte, mesure de vitesse et de courant

Dans cette partie, nous avons commencé par modifier la fonction start afin qu'elle démarre les PWM, mais avec un rapport cyclique de 50 %, ce qui ne fait pas tourner le moteur.
Nous avons ensuite cherché à améliorer la fonction speed afin de minimiser les appels de courant dans le moteur lorsque l'utilisateur modifie la vitesse du moteur (le problème étant qu'une modification brutale du rapport cyclique des PWM crée des appels de courant dangereux dans les transistors).

Pour résoudre ce problème, nous avons implémenté, à l'aide d'une boucle for, une rampe permettant de modifier progressivement le rapport cyclique des PWM en l'incrémentant toutes les 10 ms vers la valeur cible.

Nous nous sommes ensuite attelés à la mesure du courant dans les bras de pont du hacheur.
Pour ce faire, nous avons implémenté une fonction ADC, appelable depuis le shell, afin de mesurer ce courant.
Cette mesure s'effectue initialement avec l'ADC en polling. Nous avons ensuite modifié cette fonction pour qu'elle effectue des mesures de courant à intervalles réguliers.

Pour cela, nous déclenchons les mesures au moment où le timer des PWM commence à décompter, car c'est le moment où le courant est le plus stable (instant le plus éloigné des commutations des transistors). Cette mesure est ensuite convertie par l'ADC en une valeur numérique, qui est stockée dans le DMA.
