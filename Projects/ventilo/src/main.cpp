#include <Arduino.h>

// Pins de contrôle
const uint8_t PIN_IN1 = 3;
const uint8_t PIN_IN2 = 4;

void stopper() {
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, LOW);
}

void tournerSensHoraire() {
    digitalWrite(PIN_IN1, HIGH);
    digitalWrite(PIN_IN2, LOW);
}

void tournerSensAntiHoraire() {
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, HIGH);
}

void setup() {
    pinMode(PIN_IN1, OUTPUT);
    pinMode(PIN_IN2, OUTPUT);
    stopper();
}

void loop() {
    // 1. Tourne dans un sens pendant 2 secondes
    tournerSensHoraire();
    delay(20000);

    // 2. Pause 1 seconde
    stopper();
    delay(1000);

    // 3. Tourne dans l'autre sens pendant 2 secondes
    tournerSensAntiHoraire();
    delay(2000);

    // 4. Pause 1 seconde
    stopper();
    delay(1000);
}