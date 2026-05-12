/*
*  IMPORTANT! S'assurer que le Wifi est configuré avant de téléverser ce code.
*  Projet : Examples --> WifiEspAT --> Tools --> SetupWifiPersistentConnection
*/

#define HOME 1
#include <OneButton.h>
#include <HCSR04.h>
#include <DHT.h>
#include <LCD_I2C.h>
#include <AccelStepper.h>
#include "Convoyeur.h"
#include "Irrigation.h"
#include <WiFiEspAT.h>
#include <PubSubClient.h>


#define AT_BAUD_RATE 115200

#if HOME
#define DEVICE_NAME "NickHome"
#else
#define DEVICE_NAME "NickProf"
#endif

#define MQTT_PORT 1883
#define MQTT_USER "etdshawi"
#define MQTT_PASS "shawi123"

// Serveur MQTT du prof
const char* mqttServer = "216.128.180.194";
enum EtatLCD { BOOT,
               DHT_STATE,
               CALIB_STATE,
               LUM_DIST_STATE,
               VANNE_STATE } etatLCD = BOOT;


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


Convoyeur convoyeur(MOTEUR_PIN1, MOTEUR_PIN2, MANETTE, BOUTON_PIN, AFF_CLK, AFF_DIN, AFF_CS);
Irrigation irrigation(LED_PIN, IN_1, IN_2, IN_3, IN_4);
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
WiFiClient wifiClient;
PubSubClient client(wifiClient);







void wifiInit() {
  // Initialisation du module WiFi.
  Serial1.begin(AT_BAUD_RATE);
  WiFi.init(&Serial1);

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println();
    Serial.println("La communication avec le module WiFi a échoué!");
    // Ne pas continuer
    while (true) {
      // Clignoter rapidement pour annoncer l'erreur
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(50);
    }
  }

  // En attendant la connexion au réseau Wifi configuré avec le sketch SetupWiFiConnection
  Serial.println("En attente de connexion au WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print('.');
  }
  Serial.println();

  IPAddress ip = WiFi.localIP();
  Serial.println();
  Serial.println("Connecté au réseau WiFi.");
  Serial.print("Adresse : ");
  Serial.println(ip);

  printWifiStatus();
}

// Procédure Afficher le status de la connection
// WiFi sur le port série
void printWifiStatus() {

  // imprimez le SSID du réseau auquel vous êtes connecté:
  char ssid[33];
  WiFi.SSID(ssid);
  Serial.print("SSID: ");
  Serial.println(ssid);

  // imprimez le BSSID du réseau auquel vous êtes connecté:
  uint8_t bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  uint8_t mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  printMacAddress(mac);

  // imprimez l'adresse IP de votre carte:
  IPAddress ip = WiFi.localIP();
  Serial.print("Adresse IP: ");
  Serial.println(ip);

  // imprimez la force du signal reçu:
  long rssi = WiFi.RSSI();
  Serial.print("force du signal (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}

// Vous pouvez remplacez cette fonction par la votre
// par exemple pour allumer une lumière


// Gestion des messages reçues de la part du serveur MQTT
void mqttEvent(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message recu [");
  Serial.print(topic);
  Serial.print("] ");

  payload[length] = '\0';
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();

  if (strcmp(topic, "etu_09/convVit") == 0) {
    int vitesse = atoi((char*)payload);


    convoyeur.setVitesseMoteur(vitesse);
  }
}

void periodicTask() {
  static unsigned long lastTime = 0;
  const unsigned int rate = 3000;

  static char message[200] = "";
  static char szTemp[6];
  static char szHum[6];


  if (currentTime - lastTime < rate) return;

  lastTime = currentTime;



  // On convertit les valeurs en chaîne de caractères
  dtostrf(valeurTemp, 4, 1, szTemp);
  dtostrf(valeurHum, 4, 1, szHum);


  sprintf(message, "{\"name\":\"2412433\",\"temp\":\"%s\",\"hum\":\"%s\",\"millis\":%lu,\"lum\":%d,\"irrState\":%d,\"irrPos\":%d,\"convVit\":%d}",
          szTemp,
          szHum,
          currentTime,
          pourcentage,
          irrigation.getCurrentState(),
          irrigation.getPositionPct(),
          convoyeur.getVitesseMoteur());

  Serial.print("Envoie : ");
  Serial.println(message);


  // Changer le topic pour celui qui vous concerne.
  if (!client.publish("etd/9", message)) {
    reconnect();
    Serial.println("Incapable d'envoyer le message!");
  } else {
    Serial.println("Message envoyé");
  }
}

bool reconnect() {
  bool result = client.connect(DEVICE_NAME, MQTT_USER, MQTT_PASS);
  if (!result) {
    Serial.println("Incapable de se connecter sur le serveur MQTT");
  }
  return result;
}

void setup() {
  Serial.begin(115200);

  initMateriel();






  convoyeur.begin();
  irrigation.begin();
  irrigation.setClosedOpenedPos(cibleFermeture, cible);
  irrigation.setDistance(dist);
  irrigation.setBtnClickFlag(clique);
  wifiInit();
  client.setServer(mqttServer, MQTT_PORT);
  client.setCallback(mqttEvent);

  if (!client.connect(DEVICE_NAME, MQTT_USER, MQTT_PASS)) {
    Serial.println("Incapable de se connecter sur le serveur MQTT");
    Serial.print("client.state : ");
    Serial.println(client.state());
  } else {
    Serial.println("Connecté sur le serveur MQTT");
  }

  // S'abonner au topic commandes du serveur avec un QoS 0
  client.subscribe("etu_09/#", 0);
  // Configuration terminée
  Serial.println("Setup complété");
  delay(1000);
}


void loop() {
  currentTime = millis();
  // Mettre le code à exécuter continuellement
  client.loop();
  btn.tick();
  dhtState(currentTime);
  photoResistance(currentTime);

  affichagePortSerie(currentTime);
  //int distanceTache(currentTime);
  //tempHumTache(currentTime);
  irrigation.update();
  gestionnaireEtatLCD(currentTime);



  convoyeur.update();
  periodicTask();

  // Appeler périodiquement pour maintenir
  // la connexion au serveur MQTT
}
void initMateriel() {

  pinMode(LED_PIN, OUTPUT);
  lcd.begin();
  dht.begin();
  // initialize the lcd
  lcd.backlight();
  btn.setup(BTN_PIN);
  btn.attachClick(cliquer);
  btn.attachDoubleClick(doubleCliquer);
  //myStepper.setMaxSpeed(500);  // Vitesse max en pas/seconde
  //myStepper.setAcceleration(300);
  //myStepper.setCurrentPosition(cible);
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
  int irrState = irrigation.getCurrentState();

  if (irrState == OUVERTURE || irrState == FERMETURE || irrState == ARRET) {
    etatLCD = VANNE_STATE;
  }
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
      if (clique && !irrigation.isMoving() && irrigation.getCurrentState() != ARRET) {
        clique = false;
        etatLCD = DHT_STATE;
      }
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






void affichageVanne(unsigned long ct) {

  const int rate = 250;
  static unsigned long previousTime = 0;
  int currentState = irrigation.getCurrentState();
  if (ct - previousTime >= rate) {
    previousTime = ct;

    pourcentVanne = irrigation.getPositionPct();

    lcd.setCursor(0, 0);
    lcd.print("Vanne : ");
    lcd.print(pourcentVanne);
    lcd.print("%");
    lcd.print("                      ");
    lcd.setCursor(0, 1);
    if (currentState == FERMETURE) {

      lcd.print("Etat : ");
      lcd.print("Fermeture");

    } else if (currentState == FERME) {
      lcd.print("Etat : ");
      lcd.print("Ferme");

    } else if (currentState == OUVERTURE) {
      lcd.print("Etat : ");
      lcd.print("Ouverture");
    }

    else if (currentState == OUVERT) {
      lcd.print("Etat : ");
      lcd.print("Ouvert");
    } else if (currentState == ARRET) {
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
