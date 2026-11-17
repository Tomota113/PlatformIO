#include <Arduino.h>
#include <Servo.h>

const int servoPin = 9;
const int ledPin = 13; // LED intégrée sur la broche 13

Servo myServo;
int currentAngle = 90; // Position centrale au démarrage

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  myServo.attach(servoPin);
  myServo.write(currentAngle);
  digitalWrite(ledPin, LOW);
}

void loop() {
  if (Serial.available() > 0) {
    // Lit la commande texte jusqu'au saut de ligne '\n'
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.length() > 0) {
      int targetAngle = input.toInt();

      // Sécurité : restreindre l'angle entre 0° et 180°
      if (targetAngle >= 0 && targetAngle <= 180) {
        myServo.write(targetAngle);
        currentAngle = targetAngle;

        // Allume la LED quand le suivi est actif
        digitalWrite(ledPin, HIGH);
      }
    }
  }
}