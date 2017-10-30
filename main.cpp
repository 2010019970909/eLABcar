/*
  Programme de contrôle d'un mobile (direction, vitesse, feux)
  Vitesse sur le port UDP 175 (-255 <-> 255)
  Direction sur le port UDP 177 (-255 <-> 255)

  Activer/Désactiver l'autoOn des feux sur le port UDP 178 (1/0)

  Dimmer les feux avant sur le port UDP 179 (0 <-> 255)
  Dimmer les feux arrière sur le port UDP 180 (0 <-> 255)
  Changer le mode des feux de direction sur le port UDP 181 (0 <-> 3)
*/

#include <Arduino.h>
#include <ESP8266WiFi.h> // Bibliothèque à utiliser pour se connecter au WiFi
#include <WiFiUdp.h>
#include <Servo.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>

class voiture {
  private:
    int vitesse = 0;  int direction = 0;

    int mode = -1;  int modeE = -1;

    int pins[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};  Servo servomoteur;

    bool autoOn = 0;  int ClignoState = 0;  int period = 1000;

  public:
    /// Méthodes de configuration
    // Initialisation d'uniquement 2 moteurs (vitesse du second moteur optionnel)
    void config(int pinM1v, int pinM1d, int pinM2d, int pinM2v = -1) {
      pinMode(pinM1v, OUTPUT); this->pins[0] = pinM1v;
      pinMode(pinM1d, OUTPUT); this->pins[1] = pinM1d;
      pinMode(pinM2d, OUTPUT); this->pins[2] = pinM2d;

      if (pinM2v != -1) {
        pinMode(pinM2v, 0);  this->pins[3] = pinM2v;
        this->mode = 1; // 2 Moteurs, vitesses et direction pour les 2
      }
      else  this->mode = 0; // 2 Moteurs, direction pour les 2
    };

    // Initialisation moteur et servomoteur (pour la direction)
    void configS(int pinM1v, int pinM1d, int servo) {
      pinMode(pinM1v, OUTPUT); this->pins[0] = pinM1v;
      pinMode(pinM1d, OUTPUT); this->pins[1] = pinM1d;
      servomoteur.attach(servo);
      this->mode = 2; // Un moteur + un servomoteur;
    };

    // Initialisation des feux de signialisation
    void configE(int pinFAv, int pinFAr = -1 , int pinFDD = -1, int pinFDG = -1)  { // Feux avant
      pinMode(pinFAv, OUTPUT); this->pins[5] = pinFAv;
      this->modeE = 0;
      if (pinFAr != -1) {
        pinMode(pinFAr, OUTPUT); this->pins[6] = pinFAr;
        this->modeE = 1;
        if (pinFDD != -1 && pinFDG != -1 ) {
          pinMode(pinFDD, OUTPUT); this->pins[7] = pinFDD;
          pinMode(pinFDG, OUTPUT); this->pins[8] = pinFDG;
          this->modeE = 2;
        }
      }
    };

    // Activer l'automatisation de l'allumage des feux
    void setAutoOn(bool etat = 1) { // Feux en mode automatique
      this->autoOn = etat;
    };

    /// Contrôle du mobile
    // Fonction générique du contrôle
   void avancer(int vit, int dir)  {
      this->vitesse = vit;  this->direction = dir;
      bool dir1 = 1; bool dir2 = 1;
      int vit1; int vit2;

      vit1 = constrain(vit, -255, 255);  vit2 = constrain(dir, -255, 255);
      vit1 = abs(vit1);  vit2 = abs(vit2);
      if (vit < 0) dir1 = 0; if (dir < 0) dir2 = 0;
/*
      Serial.println("\nvit1 : " + (String) vit1);
      Serial.println("\nvit2 : " + (String) vit2);
      Serial.println("\ndir1 : " + (String) dir1);
      Serial.println("\ndir2 : " + (String) dir2);
*/
      switch (this->mode) {
        case 0: // 2 Moteurs, direction pour les 2
          analogWrite(this->pins[0], vit1); // 0->255
          digitalWrite(this->pins[1], dir1); // 0/1
          digitalWrite(this->pins[2], dir2); // 0/1
          break;

        case 1: // 2 Moteurs, vitesses et direction pour les 2
          analogWrite(this->pins[0], vit1); // 0->255
          digitalWrite(this->pins[1], dir1); // 0/1
          analogWrite(this->pins[2], vit2); // 0->255
          digitalWrite(this->pins[3], dir2); // 0/1
          break;

        case 2:
          int angle; angle = 90 + constrain(dir, -90, 90);
          analogWrite(this->pins[0], vit1); // 0->255
          digitalWrite(this->pins[1], dir1); // 0/1
          servomoteur.write(angle); // 0->180
          break;

        default:
          Serial.println("Erreur (mode inconnu)");
          break;
      }

          //  Serial.println(this->modeE);
          //  Serial.println(this->autoOn);
      switch (this->modeE) {
        case 0: // Feux avant
          if (this->autoOn)  {
            if (dir1) digitalWrite(this->pins[5], 1);
            else  digitalWrite(this->pins[5], 0);
          }
          // Serial.println("a");
          break;

        case 1: // + Feux arrière
          if (this->autoOn)  {
            if (dir1) digitalWrite(this->pins[5], 1);
            else  digitalWrite(this->pins[5], 0);
            if (!dir1) digitalWrite(this->pins[6], 1);
            else  digitalWrite(this->pins[6], 0);
          }
          // Serial.println("b");
          break;

        case 2: // + Feux de direction
          if (this->autoOn)  {
            if (dir1) digitalWrite(this->pins[5], 1);
            else  digitalWrite(this->pins[5], 0);
            if (!dir1) digitalWrite(this->pins[6], 1);
            else  digitalWrite(this->pins[6], 0);
            if (vit1 == 0 || vit2 == 0) {
              if (dir2)  this->ClignoState = 1;
              else this->ClignoState = 2;
            }
            else  this->ClignoState = 0;
          }
          // Serial.println("c");
          break;

        default:  break;
      }
    };

    void changeDir(int dir)  {
      avancer(this->getVit(), dir);
    };  // Changer uniquement la direction

    void changeVit(int vit)  {
      avancer(vit, this->getDir());
    };  // Changer uniquement la vitesse

    void arreter()  {
      this->avancer(0, 0);
    };  // Couper les moteurs

    /// Fonctions de contrôle des feux
    void FeuxAvant(bool etat = 1) {
      if (this->modeE > -1)  digitalWrite(this->pins[5], etat);
    }; // Allumer les feux avant
    void FeuxArriere(bool etat = 1) {
      if (this->modeE > 0) digitalWrite(this->pins[6], etat);
    }; // Allumer les feux arrière

    void Cligno(int state = 4)  { // Allumer les feux directionnel
      if (state == 4) state = this->ClignoState;
      /*Serial.println(state);
      Serial.println(this->ClignoState);*/
      static unsigned long d = millis();

      if (this->modeE > 1) {

        switch (state) {
          case 0: // Éteint
            digitalWrite(this->pins[7], 0); digitalWrite(this->pins[8], 0);
            d = millis(); this->ClignoState = 0;
            break;

          case 1: // Droite
            if (d < millis() - this->period) {
              digitalWrite(this->pins[7], !digitalRead(this->pins[7]));
              d = millis();
            }

            digitalWrite(pins[8], 0); this->ClignoState = 1;
            break;

          case 2: // Gauche
            if (d < millis() - this->period) {
              digitalWrite(this->pins[8], !digitalRead(this->pins[8]));
              d = millis();
            }

            digitalWrite(this->pins[7], 0); this->ClignoState = 2;
            break;

          case 3: // Feux de détresse
            if (d < millis() - this->period) {
              digitalWrite(this->pins[7], !digitalRead(this->pins[7]));
              digitalWrite(this->pins[8], digitalRead(this->pins[7]));
              d = millis();
            }
            this->ClignoState = 3;
            break;

          default:  break;
        }
      }
    };

    void dimFAv(int intensite) {
      analogWrite(this->pins[5], intensite);
    };

    void dimFAr(int intensite) {
      analogWrite(this->pins[6], intensite);
    };

    /// Accesseurs de la classe
    int getDir() {
      return this->direction;
    };  // Valeur de la variable direction

    int getVit() {
      return this->vitesse;
    };  // Valeur de la variable vitesse

    /// Focntion de routine (watchdog)
    void watchdog(int dv = 2000, int dd = 4000) {
      static int dir = 0; static int vit = 0;
      static unsigned long d1 = millis();
      static unsigned long d2 = millis();

      if (d1 < (millis() - dv)) {
        if (vit == vitesse) this->changeVit(0);
        d1 = millis();
      }

      if (d2 < (millis() - dd)) {
        if (dir == direction) this->changeDir(0);
        d2 = millis();
      }
    };

    /// Fonction de test
    void test() {
      Serial.println("");
      for(int i = 0; i < 10; i++) Serial.println(this->pins[i]);

      Serial.println("\nMode et ModeE");
      Serial.println(this->mode);
      Serial.println(this->modeE);
    };
};

/// Fonctions réception des données
void listenUDPvit();  // vitesse
void listenUDPdir();  // direction
void listenUDPautoOn(); // autoOn
void listenUDPFAv();  // Feux avant
void listenUDPFAr();  // Feux arrière
void listenUDPFD(); // Feux directionnels

int convert_to_int(char* str, int len);

voiture eLABcar; WiFiUDP udpVit; WiFiUDP udpDir;
WiFiUDP udpAutoOn; WiFiUDP udpFAv; WiFiUDP udpFAr; WiFiUDP udpFD;
// Serveur TCP qur le port 80 qui va répondre aux requêtes HTTP
WiFiServer serveur(80);

IPAddress local_IP(192,168,2,1);
IPAddress gateway(192,168,2,1);
IPAddress subnet(255,255,255,0);

void setup()  {
  // WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP("eLABcar", "electroLAB");
  eLABcar.config(D2, D1, D7, D8); // Configuration des moteurs
  eLABcar.configE(D5, D6, D3, D4); // Configuration  de l'éclairage
  eLABcar.setAutoOn(); // Auto allumage des phares

  /// Configuration des ports UDP
  udpVit.begin(175);  udpDir.begin(177);
  udpAutoOn.begin(178); udpFAv.begin(179);
  udpFAr.begin(180); udpFD.begin(181);

  Serial.begin(115200);
  //eLABcar.test();

  //Serial.print("On");
/*
  // Connect to WiFi network
  WiFi.begin("eLABcar", "electroLAB");
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(BUILTIN_LED, digitalRead(BUILTIN_LED));
  }

  Serial.print("\nConnecte a : ");
  Serial.println("eLABcar");
  Serial.print("Adresse IP : ");
  Serial.println(WiFi.localIP());
*/

  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin("eLABcar")) {
    Serial.println("Erreur de configuration du service mDNS");
    while(1) delay(1000);
  }
  Serial.println("Repondeur mDNS allume");

  // Start TCP (HTTP) server
  serveur.begin();
  Serial.println("Serveur TCP allume");

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
}

void loop() {
  eLABcar.watchdog(); // On éteint les moteurs si on ne reçois pas de nouvelles commandes
  eLABcar.Cligno(); // On s'assure que les feux directionnels fonctionnent
  listenUDPvit();
  listenUDPdir();
  listenUDPautoOn(); // autoOn
  listenUDPFAv();  // Feux avant
  listenUDPFAr();  // Feux arrière
  listenUDPFD();  // Feux directionnels
}

void listenUDPvit() {
  char inPacket[255];
  int size = udpVit.parsePacket();
  if(size)  {
    int len = udpVit.read(inPacket, 255);
    if (len > 0)
      inPacket[len] = 0;
    int commande = convert_to_int(inPacket, len);
    eLABcar.changeVit(commande);
    itoa(commande, inPacket, 10); // Entier, CharArray, Base
    udpVit.beginPacket(udpVit.remoteIP(), udpVit.remotePort());
    udpVit.write(inPacket);
    udpVit.endPacket();
  }
}

void listenUDPdir() {
  char inPacket[255];
  int size = udpDir.parsePacket();
  if(size)  {
    int len = udpDir.read(inPacket, 255);
    if (len > 0)
      inPacket[len] = 0;
    int commande = convert_to_int(inPacket, len);
    eLABcar.changeDir(commande);
    itoa(commande, inPacket, 10); // Entier, CharArray, Base
    udpDir.beginPacket(udpDir.remoteIP(), udpDir.remotePort());
    udpDir.write(inPacket);
    udpDir.endPacket();
  }
}

void listenUDPautoOn()  {
  char inPacket[255];
  int size = udpAutoOn.parsePacket();
  if(size)  {
    int len = udpAutoOn.read(inPacket, 255);
    if (len > 0)
      inPacket[len] = 0;
    int commande = convert_to_int(inPacket, len);
    commande = constrain(commande, 0, 1);
    eLABcar.setAutoOn(commande);
    itoa(commande, inPacket, 10); // Entier, CharArray, Base
    udpAutoOn.beginPacket(udpAutoOn.remoteIP(), udpAutoOn.remotePort());
    udpAutoOn.write(inPacket);
    udpAutoOn.endPacket();
  }
} // autoOn

void listenUDPFAv() {
  char inPacket[255];
  int size = udpFAv.parsePacket();
  if(size)  {
    int len = udpFAv.read(inPacket, 255);
    if (len > 0)
      inPacket[len] = 0;
    int commande = convert_to_int(inPacket, len);
    commande = constrain(commande, 0, 255);
    eLABcar.dimFAv(commande);
    /*if(!commande) eLABcar.setAutoOn(1);
    else eLABcar.setAutoOn(0);*/
    itoa(commande, inPacket, 10); // Entier, CharArray, Base
    udpFAv.beginPacket(udpFAv.remoteIP(), udpFAv.remotePort());
    udpFAv.write(inPacket);
    udpFAv.endPacket();
  }
} // Feux avant

void listenUDPFAr() {
  char inPacket[255];
  int size = udpFAr.parsePacket();
  if(size)  {
    int len = udpFAr.read(inPacket, 255);
    if (len > 0)
      inPacket[len] = 0;
    int commande = convert_to_int(inPacket, len);
    commande = constrain(commande, 0, 255);
    eLABcar.dimFAr(commande);
    /*if(!commande) eLABcar.setAutoOn(1);
    else eLABcar.setAutoOn(0);*/
    itoa(commande, inPacket, 10); // Entier, CharArray, Base
    udpFAr.beginPacket(udpFAr.remoteIP(), udpFAr.remotePort());
    udpFAr.write(inPacket);
    udpFAr.endPacket();
  }
}  // Feux arrière

void listenUDPFD()  { // Mode de clignottement des feux directionnels
  char inPacket[255];
  int size = udpFD.parsePacket();
  if(size)  {
    int len = udpFD.read(inPacket, 255);
    if (len > 0)
      inPacket[len] = 0;
    int commande = convert_to_int(inPacket, len);
    commande = constrain(commande, 0, 3);
    eLABcar.Cligno(commande);
    itoa(commande, inPacket, 10); // Entier, CharArray, Base
    udpFD.beginPacket(udpFD.remoteIP(), udpFD.remotePort());
    udpFD.write(inPacket);
    udpFD.endPacket();
  }
} // Feux directionnels

int convert_to_int(char* str, int len)  {
  // on suprime les caractères non désirés
  char temp[len];
  int j(0);
  int tempNum = 1;
  int integrer = 0;

  for(int i=0; i<len; i++) {
    if(str[i]=='-' || str[i]=='0' || str[i]=='1' || str[i]=='2' || str[i]=='3'
    || str[i]=='4' || str[i]=='5' || str[i]=='6' || str[i]=='7' || str[i]=='8' || str[i]=='9')  {
      temp[j]=str[i];
      j++;
    }
  }
  temp[j] = 0;
  //if(j<len) Serial.println("\nErreur(s) detectée(s), il y a " + (String) (len-j) + " erreur(s).");
  if(temp[0]=='-')  tempNum = -1;
  if(temp[0]=='0') return 0;

  // On enlève les '-'
  char temp2[j];
  int k(0);
  for(int i=0; i<j; i++) {
    if(temp[i]=='0' || temp[i]=='1' || temp[i]=='2' || temp[i]=='3'
    || temp[i]=='4' || temp[i]=='5' || temp[i]=='6' || temp[i]=='7' || temp[i]=='8' || temp[i]=='9')  {
      temp2[k]=temp[i];
      k++;
    }
  }

  //if(((k<j) && tempNum == 1) || ((k<(j-1)) && tempNum == -1))
  //    Serial.println("Nouvelles erreur(s) detectée(s), il y a " + (String) (len-j) + " erreur(s).");

  // Conversion
  for (int i = 0; i < k; i++) {
    switch (temp2 [i]) {
      case '0':
      break;
      case '1':
      integrer += pow(10, (k-1-i));
      break;
      case '2':
      integrer += pow(10, (k-1-i))*2;
      break;
      case '3':
      integrer += pow(10, (k-1-i))*3;
      break;
      case '4':
      integrer += pow(10, (k-1-i))*4;
      break;
      case '5':
      integrer += pow(10, (k-1-i))*5;
      break;
      case '6':
      integrer += pow(10, (k-1-i))*6;
      break;
      case '7':
      integrer += pow(10, (k-1-i))*7;
      break;
      case '8':
      integrer += pow(10, (k-1-i))*8;
      break;
      case '9':
      integrer += pow(10, (k-1-i))*9;
      break;
    }
  }
  integrer *= tempNum;
  //Serial.println(integrer);
  return integrer;
}
