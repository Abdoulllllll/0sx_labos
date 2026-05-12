#pragma once


typedef enum { FERME,
               OUVERTURE,
               OUVERT,
               FERMETURE,
               ARRET } IrrigationState;

#include <AccelStepper.h>
#include <HCSR04.h>
#define MOTOR_INTERFACE_TYPE 4

// Constructeur
class Irrigation {
public:
  Irrigation(int ledPin, int pin1, int pin2, int pin3, int pin4);



  // Fonction inline qui retourne la position de la vanne

  int getPosition();



  // Fonction inline qui retourne la position de la vanne en pourcentage

  int getPositionPct();



  // Fonction inline qui configure les position fermée et ouverte de la vanne

  void setClosedOpenedPos(int closed, int opened);



  // Fonction inline qui configure la référence à la distance

  int setDistance(int& dist);

  // Fonction inline qui configure la référence à l’état du click du bouton

  void setBtnClickFlag(bool& clickFlag);



  // Fonction inline qui retourne vrai si le système est ouverture ou fermeture

  int isMoving();



  // Fonction inline qui retourne l’état du système

  int getCurrentState();



  // Fonction appelée dans le loop pour mettre à jour la machine à états

  void update();
  void begin();

private:
  int _ledPin = 0;
  int _pin1 = 0;
  int _pin2 = 0;
  int _pin3 = 0;
  int _pin4 = 0;
  int* _distance = nullptr;
  bool* _clickFlag = nullptr;
  int _position;
  int _closed;
  int _opened;
  int _pourcentage;

  AccelStepper _stepper;
  IrrigationState _state = OUVERT;
  void arretState();
  void ouvertState();
  void fermerState();
  void fermetureState(unsigned long ct);
  void ouvertureState(unsigned long ct);
};