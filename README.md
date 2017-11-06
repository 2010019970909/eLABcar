# eLABcar (BROUILLON)
Programme Arduino pour contrôler un mobile motorisé avec une ESP8266 (par le biais de requêtes UDP)

# Description de la classe voiture
## Membres privés de la classe voiture
### Variables de vitesse et de direction
    1. int vitesse = 0;  
    2. int direction = 0;
    
On stocke la vitesse et la direction pour pouvoir les utiliser dans les méthodes de la classe.
    
### Variables de configuration
    3. int mode[2] = {-1, -1}
    4. int pins[2][10] = {{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}}  
    5. Servo servomoteur
    
- La variable "mode\[0]" stocke le mode de configuration imposé par l'interface de puissance (pont en H, servomoteur,...). La variable est à -1 s'il n'y a pas eu de configuration (pas d'appel de .config() ou de .configS()).

- La variable "mode\[1]" stocke le mode de configuration imposé par les pins des feux.  La variable est à -1 s'il n'y a pas eu de configuration (pas d'appel de .configE()).

- Le tableau "pins" stocke le numéro de chaque broche utilisée pour contrôler les moteurs (pins\[0]\[i]) et les feux (pins\[1]\[i]).

- La variable "servomoteur" de type Servo est utiliser uniquement si l'on se sert d'un servomoteur pour assurer la direction.
    
### Variables de gestion des feux
    6. bool autoOn = 0
    7. int ClignoState = 0
    8. int period = 1000
    
- La variable "autoOn" permet de choisir entre une gestion "automatique" des feux et le mode "manuel".

- La variable "ClignoState" permet de choisir entre 4 modes de fonctionnement pour les feux (0 = éteint, 1 = clignotement à gauche, 2 = clignotement à droite, 3 = feux de détresses).

- La variable "period" définie le temps que passe le feux clignotant à être allumé (c'est la demi-période d'un cycle de clignotement).
    
## Membres public de la classe voiture
### Méthodes de configuration
    1. void config(int pinM1v, int pinM1d, int pinM2d, int pinM2v = -1)
    2. void configS(int pinM1v, int pinM1d, int servo)
    3. void configE(int pinFAv, int pinFAr = -1 , int pinFDD = -1, int pinFDG = -1)

### Méthode modifiant le comportement des feux
    4. void setAutoOn(bool etat = 1)

### Méthodes de contrôle du mobile et du/des moteur(s).
    5. void avancer(int vit, int dir)
    6. void changeDir(int dir)
    7. void changeVit(int vit)
    8. void arreter()

### Méthodes de contrôle du mobile et du/des moteur(s).
    9. void FeuxAvant(bool etat = 1)
    10. void FeuxArriere(bool etat = 1)
    11. void Cligno(int state = 4)
    12. void dimFAv(int intensite)
    13. void dimFAr(int intensite)

### Accesseurs de la classe
    14. int getDir()
    15. int getVit()
    
- L'accesseurs "getDir()" retourne la valeur de "direction".

- L'accesseurs "getVit()" retourne la valeur de "vitesse".
 
### Watchdog
    16. void watchdog(int dv = 2000, int dd = 4000)

- Le "watchdog" ou chien de garde, remet à zéro les variables "vitesse" et "direction" s'ils n'ont pas changé au bout de certains délais (par défaut 2000 ms pour "vitesse" et 4000 ms pour "direction"). Utile si la télécommande perd le signal.

### Test
    17. void test()
    
- Cette méthode renvoit toutes les valeurs du tableau "pins", et la valeur de "mode", ainsi que de "modeE".

# Fonctions utilisées pour récupérer les valeurs envoyées par la télécommande en UDP
