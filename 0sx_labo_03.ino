#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "OneButton.h"
// adresse la plus commune
LiquidCrystal_I2C lcd(0x27, 16, 2);

byte char33[8] = {
  B11100,  // ###..
  B00100,  // ..#..
  B01100,  // ..#..
  B00111,  // ###..
  B11101,  // ..#..
  B00011,  // ..#..
  B00001,  // ###..
  B00111   // .....
};



int ledPin = 8;
int PhotoResPin = A0;
int joystickX = A1;
int joystickY = A2;
int bouton = 2;
bool allumageLed = false;
bool eteindreLed = true;
unsigned long compteurAllumage = 0;
unsigned long compteurEteindre = 0;
unsigned long previousTime = 0;
int minVal = 0;
int mavalXVal = 100;
int min = 0;
int max = 1023;
int seuil = 50;
int delaiChangement = 5000;
int delaiMaj = 100;
bool clique = false;
unsigned long lastTime = 0;
int etatLed = LOW;
int minvalYX = -100;
int maxvalYX = 100;
OneButton button(bouton, true);
int page = 0;
int valeurPhotoR = 0;
int valeurvalX = 0;
int valeurY = 0;
bool systemActive = false;
int valeurSystem = 0;
int pourcentage = 0;

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  lcd.init();


  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Ouattara");  // allume rétroéclairage
  lcd.createChar(2, char33);
  lcd.setCursor(0, 1);
  lcd.write(byte(2));
  lcd.setCursor(14, 1);
  lcd.print("33");
  delay(3000);

  lcd.clear();
  button.attachClick(cliquer);
}

void photoResistance() {

   valeurPhotoR = analogRead(PhotoResPin);
   pourcentage = map(valeurPhotoR, min, max, minVal, maxvalYX);
  lcd.setCursor(0, 0);
  lcd.print("pct lum: ");
  lcd.print(pourcentage);
  lcd.print("%");

  unsigned long ct = millis();


  unsigned long tempsEcoule = ct - previousTime;
  previousTime = ct;



  if (pourcentage < seuil) {

    compteurAllumage += tempsEcoule;
  } else {
    compteurAllumage = 0;
  }
  if (compteurAllumage >= delaiChangement) {
    digitalWrite(ledPin, HIGH);

    lcd.setCursor(0, 1);

    lcd.print("phare : ON");
    systemActive = true;
  }


  if (pourcentage > seuil) {

    compteurEteindre += tempsEcoule;
  } else {
    compteurEteindre = 0;
  }
  if (compteurEteindre >= delaiChangement) {
    digitalWrite(ledPin, LOW);



    lcd.setCursor(0, 1);
    lcd.print("phare: OFF");
    systemActive = false;
  }
}
float tacheDeplacementX(int x){
  static unsigned long dernierTemps = 0;
  static float dernierPos = 0;

  float vMax = 5.0;
  float deltaT = 0.0;
  float vitesse;
  float pos;

  int taux = 20;

  unsigned long temps = millis();

  
  if (temps - dernierTemps < taux)
  {
    return dernierPos;
  }

 
  deltaT = (temps - dernierTemps) / 1000.0;

  dernierTemps = temps;

  
  vitesse = map(x, 0, 1023, -vMax * 100, vMax * 100) / 100.0;

 
  pos = dernierPos + vitesse * deltaT;
if (pos >100){
  pos=100;
}
if (pos<-100){
  pos=-100;
}
  dernierPos = pos;

  return pos;
}

float tacheDeplacementY(int y){
  static unsigned long dernierTemps = 0;
  static float dernierPos = 0;

  float vMax = 10.0;
  float deltaT = 0.0;
  float vitesse;
  float pos;

  int taux = 20; 

  unsigned long temps = millis();

  
  if (temps - dernierTemps < taux)
  {
    return dernierPos;
  }

  
  deltaT = (temps - dernierTemps) / 1000.0;

  dernierTemps = temps;

  
  vitesse = map(y, 0, 1023, -vMax * 100, vMax * 100) / 100.0;

  
  pos = dernierPos + vitesse * deltaT;
if (pos >100){
  pos=100;
}
if (pos<-100){
  pos=-100;
}
  dernierPos = pos;

  return pos;
}
void joystick() {
  int valeurX = analogRead(joystickX);
  int valeurY = analogRead(joystickY);
  int positionX = tacheDeplacementX(valeurX);
  int positionY = tacheDeplacementY(valeurY);
  
 lcd.setCursor(0, 0);
  lcd.print("X:");
  lcd.print((int)positionX);
  lcd.print("cm");
  lcd.print("    ");

  lcd.setCursor(0, 1);
  lcd.print("Y:");
  lcd.print((int)positionY);
   lcd.print("cm");
  lcd.print("    ");
}

void changementPage() {

  button.tick();

  

  if (clique) {
    if (page < 1) {
      page++;
    } else {
      page = 0;
    }
    clique = false;
  }
  if (page == 0) {

    photoResistance();
  }

  lcd.print("                      ");
  if (page == 1) {

    joystick();
    valeurPhotoR = analogRead(PhotoResPin);
    pourcentage = map(valeurPhotoR, min, max, minVal, maxvalYX);


    unsigned long ct = millis();


    unsigned long tempsEcoule = ct - previousTime;
   previousTime=ct;



    if (pourcentage < seuil) {

      compteurAllumage += tempsEcoule;
    } else {
      compteurAllumage = 0;
    }
    if (compteurAllumage >= delaiChangement) {
      digitalWrite(ledPin, HIGH);
      systemActive = true;
    }


    if (pourcentage > seuil) {

      compteurEteindre += tempsEcoule;
    } else {
      compteurEteindre = 0;
    }
    if (compteurEteindre >= delaiChangement) {
      digitalWrite(ledPin, LOW);
      systemActive = false;
    }
  }
}

int valX() {
  int valeurvalX = analogRead(joystickX);
  return valeurvalX;
}

int valY() {
  int valeurvalY = analogRead(joystickY);
  return valeurvalY;
}

int valSys() {
  if (systemActive == true) {
    return 1;

  } else {
    return 0;
  }
}

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= delaiMaj) {
    lastTime = currentTime;

    changementPage();
    Serial.print("etd:2412433,x:");
    Serial.print(valX());
    Serial.print(",");
    Serial.print("y:");
    Serial.print(valY());
    Serial.print(",");
    Serial.print("sys:");
    Serial.println(systemActive);
  }
}
void cliquer() {
  clique = true;
}