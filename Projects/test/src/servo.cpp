#include "servo.h"
#include "Servo.h"
#include "Arduino.h"

void tourner_moteur(Servo monmoteur, int broche,int delai){
    monmoteur.write(0);
    delay(1000);
    monmoteur.write(90);
    delay(100);
    monmoteur.write(180);
    delay(10000);
}