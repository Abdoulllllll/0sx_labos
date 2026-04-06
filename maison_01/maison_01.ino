

#include <LiquidCrystal_I2C.h>
#include <OneButton.h>
#include <DHT.h>
#include <HCSR04.h>

#define LED_PIN 9
#define PHOTO_PIN A0
#define BTN_PIN 4
#define DHTPIN 7
#define DHTTYPE DHT11
#define TRIGGER_PIN 12
#define ECHO_PIN 11
#define LCD_ADDR 0x27

// Traduction
// State = État
// current = actuel
// rate = taux

enum AppState { DEMARRAGE,
                DHT_STATE,
                LUM_DIST,
                CALIBRATION };

AppState currentState = DEMARRAGE;

// Définir les variables globales
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
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
OneButton button(BTN_PIN);
DHT dht(DHTPIN, DHTTYPE);
HCSR04 hc(TRIGGER_PIN, ECHO_PIN);

void setup() {
  Serial.begin(9600);

  initPins();
lcdInit();
  buttonInit();
  dht.begin();
}

void initPins() {
  pinMode(LED_PIN, OUTPUT);
}

void lcdInit() {
  lcd.init();  // initialize the lcd
  lcd.backlight();
}

void buttonInit() {
  // Faire le code pour l'initialisation du bouton
  button.attachClick(cliquer);
  button.attachDoubleClick(doubleCliquer);
  button.setClickMs(delai_appui);
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


void loop() {
  currentTime = millis();
  button.tick();
  switch (currentState) {
    case DEMARRAGE:

      bootState(currentTime);
      break;
    case DHT_STATE:

      affichageDHT(currentTime);
      if (clique) {
        clique = false;
        currentState = LUM_DIST;
      }

      if (double_clique) {
        double_clique = false;
        currentState = CALIBRATION;
      }
      break;
    case LUM_DIST:

      affichageLumDist(currentTime);
      if (clique) {
        currentState = DHT_STATE;
        clique = false;
      }
      if (double_clique) {
        currentState = CALIBRATION;
        double_clique = false;
      }
      break;
    case CALIBRATION:

      calibrer(currentTime);
      if (clique) {
        currentState = DHT_STATE;
        clique = false;
      }
      break;
  }
  dhtState(currentTime);
  photoResistance(currentTime);

  affichagePortSerie(currentTime);
}

// Représente une tâche quelconque qui doit être exéctué de façon continue
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
    Serial.println(valeurHum);
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
    maximum = setMaxPhotoR(valeurPhotoR);
    minimum = setMinPhotoR(valeurPhotoR);
    pourcentage = map(valeurPhotoR, minimum, maximum, minPourcent, maxPourcent);

    // Exemple de tâche
    // Lire distance
  }
  distance();
  if (pourcentage < 30) {
    digitalWrite(LED_PIN, HIGH);

  } else {
    digitalWrite(LED_PIN, LOW);
  }
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

  currentState = DHT_STATE;
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


void cliquer() {
  clique = true;
}
void doubleCliquer() {
  double_clique = true;
}
