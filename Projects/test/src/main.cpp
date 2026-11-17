#include <Arduino.h>
#include "led.h" // On importe notre fichier personnalisé 
#include <Servo.h>
#include "servo.h"

const int LED_PIN = 8;
Servo monmoteur;

void setup() {
    pinMode(LED_PIN, OUTPUT);
    monmoteur.attach(9);
}

void loop() {
    // On appelle la fonction créée dans l'autre fichier
    clignoterLed(LED_PIN, 50000); 
    tourner_moteur(monmoteur,9,100);
    
}
