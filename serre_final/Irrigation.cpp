#include "Irrigation.h"



Irrigation::Irrigation(int ledPin, int pin1, int pin2, int pin3, int pin4)
  : _stepper(MOTOR_INTERFACE_TYPE, pin1, pin3, pin2, pin4) {
  _ledPin = ledPin;
  _pin1 = pin1;
  _pin2 = pin2;
  _pin3 = pin3;
  _pin4 = pin4;
}



int Irrigation::setDistance(int &_dist) {
  _distance = &_dist;
  return *_distance;
}

void Irrigation::setBtnClickFlag(bool &clickFlag) {
  _clickFlag = &clickFlag;
}

int Irrigation::getPosition() {
  _position = _stepper.currentPosition();
  return _position;
}

void Irrigation::setClosedOpenedPos(int closed, int opened) {
  _closed = closed;
  _opened = opened;
}

int Irrigation::getPositionPct() {
  int min = 0;
  int max = 100;

  int positionBrut = getPosition();

  return map(positionBrut, _closed, _opened, min, max);
}

int Irrigation::isMoving() {
  int cible = _stepper.distanceToGo();
  if (cible != 0) {
    return true;
  } else {
    return false;
  }
}

int Irrigation::getCurrentState() {
  return _state;
}
void Irrigation::update() {
  unsigned long ct = millis();
  switch (_state) {
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

void Irrigation::ouvertureState(unsigned long ct) {
  static bool firstTime = true;
  static unsigned long previousTime = 0;
  int rate = 100;
  static bool state = LOW;

  if (firstTime) {
    firstTime = false;
    // Code à exécuter une seule fois au début
    // Initialisation des variables


    _stepper.moveTo(_opened);
  }


  // Si nécessaire


  // Code à exécuter à chaque appel de la fonction
  // Mise à jour des variables

  bool transition = false;  // Mettre à vrai si on change d'état
  if (*_clickFlag) {
    _state = ARRET;
    transition = true;
    *_clickFlag = false;
  } else if (_stepper.distanceToGo() == 0) {
    _state = OUVERT;
    transition = true;

  } else {
    _stepper.run();
  }
  if (ct - previousTime >= rate) {

    state = !state;  //clignoter
    digitalWrite(_ledPin, state);
    previousTime = ct;
  }
  if (transition) {
    // Code à exécuter une seule fois à la fin
    // Nettoyage des variables
    digitalWrite(_ledPin, LOW);

    firstTime = true;
  }
}

void Irrigation::fermetureState(unsigned long ct) {
  static unsigned long previousTime = 0;
  int rate = 100;
  static bool firstTime = true;
  static bool state = LOW;

  if (firstTime) {
    firstTime = false;
    // Code à exécuter une seule fois au début
    // Initialisation des variables


    _stepper.moveTo(_closed);
  }

  if (ct - previousTime >= rate) {

    state = !state;  //clignoter
    digitalWrite(_ledPin, state);
    previousTime = ct;
  }

  // Code à exécuter à chaque appel de la fonction
  // Mise à jour des variables
  _stepper.run();

  bool transition = _stepper.distanceToGo() == 0;  // Mettre à vrai si on change d'état
  bool sortie = false;

  if (transition) {
    _state = FERME;
    sortie = true;
  }

  if (*_clickFlag) {
    _state = ARRET;
    sortie = true;
    *_clickFlag = false;
  }

  if (sortie) {
    // Code à exécuter une seule fois à la fin
    // Nettoyage des variables

    digitalWrite(_ledPin, LOW);
    firstTime = true;
  }


  // Vous pouvez ajouter d'autres transitions ici
}

void Irrigation::fermerState() {

  static bool firstTime = true;
  int dist = *_distance;


  if (firstTime) {
    firstTime = false;
    // Code à exécuter une seule fois au début
    // Initialisation des variables
    _stepper.moveTo(_stepper.currentPosition());
  }
  _stepper.run();


  // Code à exécuter à chaque appel de la fonction
  // Mise à jour des variables

  bool transition = false;  // Mettre à vrai si on change d'état
  if (dist < 20) {
    _state = OUVERTURE;
    transition = true;
  }

  if (transition) {
    // Code à exécuter une seule fois à la fin
    // Nettoyage des variables


    firstTime = true;
  }

  // Vous pouvez ajouter d'autres transitions ici
}

void Irrigation::ouvertState() {

  static bool firstTime = true;

  int dist = *_distance;
  if (firstTime) {
    firstTime = false;
    // Code à exécuter une seule fois au début
    // Initialisation des variables
    _stepper.moveTo(_stepper.currentPosition());
  }
  _stepper.run();


  // Code à exécuter à chaque appel de la fonction
  // Mise à jour des variables

  bool transition = false;  // Mettre à vrai si on change d'état
  if (dist > 25) {
    _state = FERMETURE;
    transition = true;
  }

  if (transition) {
    // Code à exécuter une seule fois à la fin
    // Nettoyage des variables


    firstTime = true;
  }


  // Vous pouvez ajouter d'autres transitions ici
}
void Irrigation::arretState() {

  static bool firstTime = true;


  if (firstTime) {
    firstTime = false;
    // Code à exécuter une seule fois au début
    // Initialisation des variables
    // _stepper.setCurrentPosition(_stepper.currentPosition());
    // _stepper.moveTo(_stepper.currentPosition());
  }



  // Code à exécuter à chaque appel de la fonction
  // Mise à jour des variables


  bool transition = false;  // Mettre à vrai si on change d'état
  if (*_clickFlag) {
    _state = OUVERTURE;
    transition = true;
    *_clickFlag = false;
  }

  if (transition) {
    // Code à exécuter une seule fois à la fin
    // Nettoyage des variables


    firstTime = true;
  }

  // Vous pouvez ajouter d'autres transitions ici
}

void Irrigation::begin() {
  _stepper.setMaxSpeed(500);
  _stepper.setAcceleration(300);
  _stepper.setCurrentPosition(_opened);
}
