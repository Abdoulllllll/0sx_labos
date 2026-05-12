#include "Convoyeur.h"

// Initialisation de l'instance static du Convoyeur
// Pour l'instant null
Convoyeur *Convoyeur::instance = nullptr;

// TODO : Compléter le constructeur
Convoyeur::Convoyeur(byte motPin1, byte motPin2, byte manette, byte btnPin, byte aff_CLK, byte aff_DIN, byte aff_CS)
  : _button(btnPin), _u8g2(U8G2_R0, aff_CLK, aff_DIN, aff_CS, U8X8_PIN_NONE)

{
  _button.attachClick(buttonClick, this);
  _button.attachLongPressStart(longPress, this);


  _moteur_pin_1 = motPin1;
  _moteur_pin_2 = motPin2;
  _manette = manette;
  _btn_pin = btnPin;
  _aff_CLK = aff_CLK;
  _aff_DIN = aff_DIN;
  _aff_CS = aff_CS;
}


void Convoyeur::buttonClick(void *context) {
  Convoyeur *self = static_cast<Convoyeur *>(context);
  self->_buttonPressed = true;
}

void Convoyeur::longPress(void *context) {
  Convoyeur *self = static_cast<Convoyeur *>(context);
  self->_longPress = true;
}

void Convoyeur::update() {
  _currentTime = millis();
  _button.tick();
  switch (_state) {
    case ConvState::ACTIF:
      actifState();
      break;
    case ConvState::INACTIF:
      inactifState();
      break;
    case ConvState::MANUEL:
      manuelState();
      break;
    case ConvState::CONSTANT:
      constantState();
      break;
  }
}
void Convoyeur::setMoteurVitesse(int vitesse) {

  int min = 0;
  int max = 255;
  int minVitesse = 0;
  int maxVitesse = 100;


  _valeurM = map(abs(vitesse), minVitesse, maxVitesse, min, max);
  if (vitesse > 0) {

    analogWrite(_moteur_pin_1, _valeurM);
    analogWrite(_moteur_pin_2, 0);


  }

  else if (vitesse < 0) {

    analogWrite(_moteur_pin_2, _valeurM);
    analogWrite(_moteur_pin_1, 0);

  }

  else {
    analogWrite(_moteur_pin_2, 0);
    analogWrite(_moteur_pin_1, 0);
  }
}

int Convoyeur::getVitesseMoteur() {
  return _valY;
}

void Convoyeur::setAffichage(int id_symbole) {

  switch (id_symbole) {
    case 0:
      _u8g2.clearBuffer();
      _u8g2.drawBitmap(0, 0, 1, 8, bitmapMoins);
      _u8g2.sendBuffer();
      break;
    case 1:
      _u8g2.clearBuffer();
      _u8g2.drawBitmap(0, 0, 1, 8, bitmapPointExclamation);
      _u8g2.sendBuffer();
      break;
    case 2:
      _u8g2.clearBuffer();
      _u8g2.drawBitmap(0, 0, 1, 8, bitmapHaut);
      _u8g2.sendBuffer();
      break;
    case 3:
      _u8g2.clearBuffer();
      _u8g2.drawBitmap(0, 0, 1, 8, bitmapBas);
      _u8g2.sendBuffer();
      break;
  }
}

void Convoyeur::begin() {
  _u8g2.begin();
  pinMode(_moteur_pin_1, OUTPUT);
  pinMode(_moteur_pin_2, OUTPUT);
  setMoteurVitesse(0);
  setAffichage(0);
}

void Convoyeur::actifState() {

  setMoteurVitesse(0);
  setAffichage(1);
  int min = -100;
  int max = 100;
  int minJoystick = 0;
  int maxJoystick = 1023;

  int joystickY = analogRead(_manette);
  _valY = map(joystickY, minJoystick, maxJoystick, min, max);

  if (abs(_valY) > _joystickDeadZone) {
    _state = ConvState::MANUEL;
  }
  if (_buttonPressed) {
    _buttonPressed = false;
  }
  if (_longPress) {
    _state = ConvState::INACTIF;
    _longPress = false;
    return;
  }
}


void Convoyeur::inactifState() {
  setMoteurVitesse(0);
  setAffichage(0);

  if (_longPress) {
    _state = ConvState::ACTIF;
    _longPress = false;
    return;
  }
}

void Convoyeur::manuelState() {

  int min = -100;
  int max = 100;
  int minJoystick = 0;
  int maxJoystick = 1023;

  int joystickY = analogRead(_manette);
  int valY = map(joystickY, minJoystick, maxJoystick, min, max);

  setMoteurVitesse(valY);
  if (abs(valY) <= _joystickDeadZone) {
    setMoteurVitesse(0);
    _state = ConvState::ACTIF;
    return;
  } else if (valY < 0) {
    setAffichage(3);
  }

  else {
    setAffichage(2);
  }

  if (_longPress) {
    _state = ConvState::INACTIF;
    _longPress = false;
    return;
  }
  if (_buttonPressed) {
    _state = ConvState::CONSTANT;
    _vitesse_constante = valY;
    _buttonPressed = false;
    return;
  }
}


void Convoyeur::constantState() {

  _valY = _vitesse_constante;
  setMoteurVitesse(_vitesse_constante);

  if (_vitesse_constante < 0) {
    setAffichage(3);
  } else if (_vitesse_constante > 0) {
    setAffichage(2);
  } else {
    setAffichage(1);
  }

  if (_longPress) {
    _state = ConvState::INACTIF;
    _longPress = false;

    return;
  }

  if (_buttonPressed) {
    _buttonPressed = false;


    _state = ConvState::MANUEL;

    return;
  }
}
void Convoyeur::setVitesseMoteur(int vitesse) {

  _vitesse_constante = vitesse;
  _valY = _vitesse_constante;

  _state = ConvState::CONSTANT;

  setMoteurVitesse(_vitesse_constante);
}
