const int ledPin1 = 8;
const int ledPin2 = 9;
const int ledPin3 = 10;
const int ledPin4 = 11;
const int bouton = 2;
unsigned long previous_time = 0;
const int delai = 20;
const int delai_bouton = 20;
int etat_stable = HIGH;
int dernier_etat = HIGH;
bool bouton_appuyer = false;

int led[4] = { ledPin1, ledPin2, ledPin3, ledPin4 };
int sensorValue = 0;
int pointTotal = 20;
int minPot = 0;
int maxPot = 1023;
int min = 0;
int max = 20;
int maxBouton = 100;

void setup() {
  // initialize serial communication at 9600 bits per second:

  Serial.begin(9600);
  pinMode(bouton, INPUT_PULLUP);
  for (int i = 0; i < 4; i++) {
    pinMode(led[i], OUTPUT);
  }
}

void allumerled() {
  unsigned long ct = millis();
  if (ct - previous_time >= delai) {
    previous_time = ct;
    sensorValue = analogRead(A1);
  }

  if (sensorValue <= 255) {
    int valeurPot = map(sensorValue, 0, 255, 0, 255);
    analogWrite(ledPin1, valeurPot);
    for (int i = 1; i < 4; i++) {
      analogWrite(led[i], 0);
    }
  } else if (sensorValue <= 511) {
    int valeurPot = map(sensorValue, 256, 511, 0, 255);

    analogWrite(ledPin2, valeurPot);
    for (int i = 2; i < 4; i++) {
      analogWrite(led[i], 0);
    }
  } else if (sensorValue <= 767) {
    int valeurPot = map(sensorValue, 512, 767, 0, 255);

    analogWrite(ledPin3, valeurPot);

    for (int i = 3; i < 4; i++) {
      analogWrite(led[i], 0);
    }

  } else if (sensorValue >= 768 && sensorValue <= 1023) {
    int valeurPot = map(sensorValue, 768, 1023, 0, 255);

    analogWrite(ledPin4, valeurPot);
  }
}

void progression() {
  int etat_bouton = digitalRead(bouton);
 unsigned long ct = millis();
  if (etat_bouton != dernier_etat) {
    dernier_etat = etat_bouton;
    previous_time = millis();
  }

  if (ct - previous_time >= delai_bouton) {

    if (etat_bouton != etat_stable) {
      etat_stable = etat_bouton;
      if (etat_stable == LOW) {
        bouton_appuyer = !bouton_appuyer;
      }
    }

    int niveau = map(sensorValue, minPot, maxPot, min, max);
    int progression = map(sensorValue, minPot, maxPot, min, maxBouton);

    if (bouton_appuyer) {
      Serial.print(progression);
      Serial.print("% [");
      for (int i = 0; i < pointTotal; i++) {
        if (i < niveau) {
          Serial.print(">");
        } else {
          Serial.print(".");
        }
      }
      Serial.println("]");
      bouton_appuyer = false;
    }
  }
}
// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:

  allumerled();
  progression();
}
