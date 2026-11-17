#include "led.h"

// Définition de la fonction
void clignoterLed(int broche, int delaiMs) {
    digitalWrite(broche, HIGH);
    delay(delaiMs);
    digitalWrite(broche, LOW);
    delay(delaiMs);
}