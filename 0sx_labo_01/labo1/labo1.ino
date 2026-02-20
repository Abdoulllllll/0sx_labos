
int min = 0;
int max = 255;
int delaiEteint = 250;
int delaiClignotement = 250;
int delaiAllumer = 1000;
const int LED_PIN = 13;
int tempsTotal = 2048;
int nbPas = tempsTotal / (max + 1);
const char messageClignotement[30] = "Clignotement – 2412433";
const char messageVariation[30] = "Variation – 2412433";
const char messageAe[30] = "Allume – 2412433";
void clignoter() {
  Serial.println(messageClignotement);
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_PIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(delaiClignotement);     // wait for a second
    digitalWrite(LED_PIN, LOW);   // turn the LED off by making the voltage LOW
    delay(delaiClignotement);
  }
}
void variation() {
  Serial.println(messageVariation);
  for (int i = max; i >= min; i--) {
    analogWrite(LED_PIN, i);
    delay(nbPas);
  }
}
void allumerEteint() {
  Serial.println(messageAe);
  digitalWrite(LED_PIN, LOW);
  delay(delaiEteint);
  digitalWrite(LED_PIN, HIGH);
  delay(delaiAllumer);
  digitalWrite(LED_PIN, LOW);
  delay(delaiAllumer);
}
void setup() {
  // put your setup code here, to run once:
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  clignoter();
  variation();
  allumerEteint();
}
