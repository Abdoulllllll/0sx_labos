#include <OneButton.h>
#include <HCSR04.h>
#include <DHT.h>
#include <LCD_I2C.h>
#include <AccelStepper.h>
#include "Convoyeur.h"
enum EtatLCD { BOOT,
               DHT_STATE,
               CALIB_STATE,
               LUM_DIST_STATE,
               VANNE_STATE } etatLCD = BOOT;
enum EtatIrrigation { FERME,
                      OUVERTURE,
                      OUVERT,
                      FERMETURE,
                      ARRET } etatIrrigation = OUVERT;

const int LED_PIN = 9;
const int BTN_PIN = 4;
const int ECHO_PIN = 11;
const int TRIGGER_PIN = 12;
const int DHT_PIN = 7;
const int LCD_ADDR = 0x27;
const int LCD_ROWS = 2;
const int LCD_COLS = 16;
const int PHOTO_PIN = A0;

const int MOTEUR_PIN1 = 5;
const int MOTEUR_PIN2 = 6;
const int MANETTE = A2;
const int BOUTON_PIN = 2;
const int AFF_CLK = 30;
const int AFF_DIN = 34;
const int AFF_CS = 32;

#define MOTOR_INTERFACE_TYPE 4

#define IN_1 35
#define IN_2 33
#define IN_3 37
#define IN_4 39
OneButton btn(BTN_PIN);
HCSR04 hc(TRIGGER_PIN, ECHO_PIN);
DHT dht(DHT_PIN, DHT11);
LCD_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
AccelStepper myStepper(MOTOR_INTERFACE_TYPE, IN_1, IN_3, IN_2, IN_4);

Convoyeur convoyeur(MOTEUR_PIN1, MOTEUR_PIN2, MANETTE, BOUTON_PIN, AFF_CLK, AFF_DIN, AFF_CS);
unsigned long currentTime;
bool clique = false;
bool double_clique = false;
int photoR;
int min = 1023;
int max = 0;
int valeur = 0;
int minimum = 0;
int maximum = 0;
int pourcentage = 0;
int maxPourcent = 100;
int minPourcent = 0;
int dist = 0;
float valeurHum = 0;
float valeurTemp = 0;
const int delai_appui = 100;
int pourcentVanne = 0;
const int cible = 2038;
const int cibleFermeture = 0;
void setup() {
  Serial.begin(115200);

  initMateriel();
  convoyeur.begin();
}

void loop() {
  currentTime = millis();
  btn.tick();
  dhtState(currentTime);
  photoResistance(currentTime);

  affichagePortSerie(currentTime);
  //int distanceTache(currentTime);
  //tempHumTache(currentTime);

  gestionnaireEtatLCD(currentTime);
  gestionnaireEtatIrr(currentTime);
  convoyeur.update();
  // envoieSerieTache(currentTime);
}

// Méthode pour initialiser le matériel
void initMateriel() {

  pinMode(LED_PIN, OUTPUT);
  lcd.begin();
  dht.begin();
  // initialize the lcd
  lcd.backlight();
  btn.setup(BTN_PIN);
  btn.attachClick(cliquer);
  btn.attachDoubleClick(doubleCliquer);
  myStepper.setMaxSpeed(500);  // Vitesse max en pas/seconde
  myStepper.setAcceleration(300);
  myStepper.setCurrentPosition(cible);
  // myStepper.setSpeed(100);
  // Ajouter au besoin
}
int setMinPhotoR(int val) {
  if (val < min) {
    min = val;
  }
  return min;
}

int setMaxPhotoR(int val) {
  if (val > max) {
    max = val;
  }
  return max;
}


void gestionnaireEtatLCD(unsigned long ct) {
  switch (etatLCD) {
    case BOOT:
      bootState(ct);
      break;
    case DHT_STATE:
      affichageDHT(ct);
      if (clique) {
        clique = false;
        etatLCD = LUM_DIST_STATE;
      }

      if (double_clique) {
        double_clique = false;
        etatLCD = CALIB_STATE;
      }
      break;
    case CALIB_STATE:
      calibrer(ct);
      if (clique) {
        etatLCD = DHT_STATE;
        clique = false;
      }
      break;
    case LUM_DIST_STATE:
      affichageLumDist(ct);
      if (clique) {
        etatLCD = DHT_STATE;
        clique = false;
      }
      if (double_clique) {
        etatLCD = CALIB_STATE;
        double_clique = false;
      }
      break;
    case VANNE_STATE:
      affichageVanne(ct);
      break;
  }
}

void affichagePortSerie(unsigned long ct) {
  static unsigned long lastTime = 0;
  const int rate = 3000;

  if (ct - lastTime > rate) {
    lastTime = ct;
    Serial.print("Lum:");
    Serial.print(pourcentage);
    Serial.print(",Min:");
    Serial.print(minimum);
    Serial.print(",Max:");
    Serial.print(maximum);
    Serial.print(",Dist:");
    Serial.print(dist);
    Serial.print(",T:");
    Serial.print(valeurTemp);
    Serial.print(",H:");
    Serial.print(valeurHum);
    Serial.print(",Van:");
    Serial.print(pourcentVanne);
    Serial.print(",Conv:");
    Serial.println(convoyeur.getVitesseMoteur());
    // Exemple de tâche
    // Lire distance
  }
}

void photoResistance(unsigned long ct) {
  static int valeurPhotoR = 0;
  static unsigned long lastTime = 0;
  static unsigned long lcdRate = 250;
  static unsigned long previousTime = 0;
  const int rate = 1000;

  if (ct - lastTime > rate) {
    lastTime = ct;
    valeurPhotoR = analogRead(PHOTO_PIN);
    if (etatLCD == CALIB_STATE) {
      maximum = setMaxPhotoR(valeurPhotoR);
      minimum = setMinPhotoR(valeurPhotoR);
    }
    if (minimum != maximum) {
      pourcentage = map(valeurPhotoR, minimum, maximum, minPourcent, maxPourcent);

    } else {

      pourcentage = map(valeurPhotoR, min, max, minPourcent, maxPourcent);
    }
    if (etatLCD != VANNE_STATE) {
      if (pourcentage < 30) {
        digitalWrite(LED_PIN, HIGH);
      } else {
        digitalWrite(LED_PIN, LOW);
      }
    }
    // Exemple de tâche
    // Lire distance
  }
  distance();
}
void distance() {
  unsigned long ct = millis();
  const int delai = 250;
  static unsigned long previousTime = 0;
  if (ct - previousTime >= delai) {
    previousTime = ct;
    dist = hc.dist();
  }
}

void bootState(unsigned long ct) {
  static unsigned long lastTime = 0;
  const int exitTime = 3000;
  const int lcdRate = 250;

  if (ct - lastTime >= lcdRate) {
    lastTime = ct;

    lcd.setCursor(0, 0);

    lcd.print("Etd 1: 2412433");
    lcd.print("                   ");
  }

  if (ct < exitTime) return;

  etatLCD = VANNE_STATE;
}

void dhtState(unsigned long ct) {
  static unsigned long lastTime = 0;
  const int lcdRate = 250;
  static unsigned long previousTime = 0;
  const int rate = 5000;
  if (ct - previousTime >= rate) {
    previousTime = ct;
    valeurHum = dht.readHumidity();
    valeurTemp = dht.readTemperature();
  }
}
void affichageLumDist(unsigned long ct) {
  static unsigned long lcdRate = 250;
  static unsigned long previousTime = 0;
  const int rate = 1000;
  const int delai_distance = 250;

  if (ct - previousTime >= lcdRate) {
    previousTime = ct;
    lcd.setCursor(0, 0);
    lcd.print("lum : ");
    lcd.print(pourcentage);
    lcd.print("%");
    lcd.print("                           ");
    lcd.setCursor(0, 1);
    lcd.print("Dist : ");
    lcd.print(dist);
    lcd.print(" cm");
    lcd.print("                           ");
  }
}

void affichageDHT(unsigned long ct) {

  const int lcdRate = 250;
  static unsigned long previousTime = 0;

  if (ct - previousTime >= lcdRate) {
    previousTime = ct;
    lcd.setCursor(0, 0);
    lcd.print("Temp : ");
    lcd.print(valeurTemp);
    lcd.print(" C");
    lcd.setCursor(0, 1);
    lcd.print("Hum : ");
    lcd.print(valeurHum);
    lcd.print(" %");
  }
}
void calibrer(unsigned long ct) {
  const int lcdRate = 250;
  static unsigned long lastTime = 0;
  if (ct - lastTime >= lcdRate) {
    lastTime = ct;
    lcd.setCursor(0, 0);
    lcd.print("Lum min: ");
    lcd.print(minimum);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("Lum max: ");
    lcd.print(maximum);
    lcd.print("                ");
  }
}




void gestionnaireEtatIrr(unsigned long ct) {
  switch (etatIrrigation) {
    case FERME:
      fermerState();
      break;
    case OUVERTURE:
      ouvertureState(ct);
      break;
    case OUVERT:
      ouvertState();
      break;
    case FERMETURE:
      fermetureState(ct);
      break;
    case ARRET:
      arretState();
      break;
  }
}

void ouvertureState(unsigned long ct) {

  static bool firstTime = true;
  static unsigned long previousTime = 0;
  int rate = 100;
  static bool state = LOW;
  if (firstTime) {
    firstTime = false;
    // Code à exécuter une seule fois au début
    // Initialisation des variables
    etatLCD = VANNE_STATE;

    myStepper.moveTo(cible);
  }


  // Si nécessaire


  // Code à exécuter à chaque appel de la fonction
  // Mise à jour des variables

  bool transition = false;  // Mettre à vrai si on change d'état
  if (clique) {
    etatIrrigation = ARRET;
    transition = true;
    clique = false;
  } else if (myStepper.distanceToGo() == 0) {
    etatIrrigation = OUVERT;
    transition = true;

  } else {
    myStepper.run();
  }
  if (ct - previousTime >= rate) {

    state = !state;  //clignoter
    digitalWrite(LED_PIN, state);
    previousTime = ct;
  }
  if (transition) {
    // Code à exécuter une seule fois à la fin
    // Nettoyage des variables
    digitalWrite(LED_PIN, LOW);

    firstTime = true;
  }


  // Vous pouvez ajouter d'autres transitions ici
}

void fermetureState(unsigned long ct) {
  static unsigned long previousTime = 0;
  int rate = 100;
  static bool firstTime = true;
  static bool state = LOW;

  if (firstTime) {
    firstTime = false;
    // Code à exécuter une seule fois au début
    // Initialisation des variables


    myStepper.moveTo(cibleFermeture);
    etatLCD = VANNE_STATE;
  }

  if (ct - previousTime >= rate) {

    state = !state;  //clignoter
    digitalWrite(LED_PIN, state);
    previousTime = ct;
  }

  // Code à exécuter à chaque appel de la fonction
  // Mise à jour des variables
  myStepper.run();

  bool transition = myStepper.distanceToGo() == 0;  // Mettre à vrai si on change d'état
  bool sortie = false;

  if (transition) {
    etatIrrigation = FERME;
    sortie = true;
  }

  if (clique) {
    etatIrrigation = ARRET;
    sortie = true;
    clique = false;
  }

  if (sortie) {
    // Code à exécuter une seule fois à la fin
    // Nettoyage des variables

    digitalWrite(LED_PIN, LOW);
    firstTime = true;
  }


  // Vous pouvez ajouter d'autres transitions ici
}

void fermerState() {

  static bool firstTime = true;


  if (firstTime) {
    firstTime = false;
    // Code à exécuter une seule fois au début
    // Initialisation des variables
    myStepper.moveTo(myStepper.currentPosition());
  }
  myStepper.run();


  // Code à exécuter à chaque appel de la fonction
  // Mise à jour des variables

  bool transition = false;  // Mettre à vrai si on change d'état
  if (dist < 20) {
    etatIrrigation = OUVERTURE;
    transition = true;
  }
  if (clique) {
    etatLCD = DHT_STATE;

    clique = false;
  }
  if (transition) {
    // Code à exécuter une seule fois à la fin
    // Nettoyage des variables


    firstTime = true;
  }

  // Vous pouvez ajouter d'autres transitions ici
}

void ouvertState() {

  static bool firstTime = true;


  if (firstTime) {
    firstTime = false;
    // Code à exécuter une seule fois au début
    // Initialisation des variables
    myStepper.moveTo(myStepper.currentPosition());
  }
  myStepper.run();


  // Code à exécuter à chaque appel de la fonction
  // Mise à jour des variables

  bool transition = false;  // Mettre à vrai si on change d'état
  if (dist > 25) {
    etatIrrigation = FERMETURE;
    transition = true;
  }
  if (clique) {
    etatLCD = DHT_STATE;

    clique = false;
  }
  if (transition) {
    // Code à exécuter une seule fois à la fin
    // Nettoyage des variables


    firstTime = true;
  }


  // Vous pouvez ajouter d'autres transitions ici
}
void arretState() {

  static bool firstTime = true;


  if (firstTime) {
    firstTime = false;
    // Code à exécuter une seule fois au début
    // Initialisation des variables
    // myStepper.setCurrentPosition(myStepper.currentPosition());
    // myStepper.moveTo(myStepper.currentPosition());
  }



  // Code à exécuter à chaque appel de la fonction
  // Mise à jour des variables


  bool transition = false;  // Mettre à vrai si on change d'état
  if (clique) {
    etatIrrigation = OUVERTURE;
    transition = true;
    clique = false;
  }

  if (transition) {
    // Code à exécuter une seule fois à la fin
    // Nettoyage des variables


    firstTime = true;
  }

  // Vous pouvez ajouter d'autres transitions ici
}


void affichageVanne(unsigned long ct) {
  int departVanne = 0;
  int minPct = 0;
  int maxPct = 100;
  const int rate = 250;
  static unsigned long previousTime = 0;

  if (ct - previousTime >= rate) {
    previousTime = ct;
    int position = myStepper.currentPosition();
    pourcentVanne = map(position, departVanne, cible, minPct, maxPct);

    lcd.setCursor(0, 0);
    lcd.print("Vanne : ");
    lcd.print(pourcentVanne);
    lcd.print("%");
    lcd.print("                      ");
    lcd.setCursor(0, 1);
    if (etatIrrigation == FERMETURE) {

      lcd.print("Etat : ");
      lcd.print("Fermeture");

    } else if (etatIrrigation == FERME) {
      lcd.print("Etat : ");
      lcd.print("Ferme");

    } else if (etatIrrigation == OUVERTURE) {
      lcd.print("Etat : ");
      lcd.print("Ouverture");
    }

    else if (etatIrrigation == OUVERT) {
      lcd.print("Etat : ");
      lcd.print("Ouvert");
    } else if (etatIrrigation == ARRET) {
      lcd.print("Etat : ");
      lcd.print("Arret");
    }
    lcd.print("                      ");
  }
}

void cliquer() {
  clique = true;
}
void doubleCliquer() {
  double_clique = true;
}
