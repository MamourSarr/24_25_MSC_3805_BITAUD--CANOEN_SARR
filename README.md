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

Structure du Projet

Nous avons dans cette partie cherché à commander les 2 bars de pont U et V du hacheur en utilisant 4 PWM (complémentaire 2 à 2). Pour ce faire on utilise un timmer pour genérer les 4 pwm sur 4 channels diferents.
On régle ensuite le prescaller du timer à 8 et Counter period à 1024, afin que le timer fonctionne à 20 kHz, on régle également le "Counter Mode" sur Center Aligned afin de que le timer conte puis déconte.
On régle également le dead time à 200ns afin de ne pas créer d'appel de courant dans les transistor ce qui pourrait les détruire. 
