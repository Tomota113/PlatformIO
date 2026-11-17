#include <Arduino.h>
#include <Servo.h>
#include <LiquidCrystal.h>

// Initialisation du LCD 16 broches (pins A0 à A5)
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

// Configuration des pins
const uint8_t PIN_TRIG   = 7;
const uint8_t PIN_ECHO   = 6;
const uint8_t PIN_SERVO  = 9;
const uint8_t PIN_BUZZER = 8;

Servo monServo;

int currentAngle = 0;
int step = 5; // Pas d'avancement de l'angle

float mesurerDistance() {
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);

    unsigned long duree = pulseIn(PIN_ECHO, HIGH, 30000UL);
    if (duree == 0) return -1.0;

    return (duree * 0.0343) / 2.0;
}

void setup() {
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    pinMode(PIN_BUZZER, OUTPUT);

    monServo.attach(PIN_SERVO);

    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.print("   RADAR INTRUSION   ");
    lcd.setCursor(0, 1);
    lcd.print("   INITIALISATION    ");
    delay(2000);
    lcd.clear();
}

void loop() {
    // 1. Prendre la mesure de distance
    float distance = mesurerDistance();

    // 2. Vérifier si un obstacle est trop près (moins de 15 cm)
    bool obstacleDetecte = (distance > 0 && distance < 15.0);

    // 3. Affichage sur l'écran LCD
    lcd.setCursor(0, 0);
    lcd.print("Angle: ");
    lcd.print(currentAngle);
    lcd.print((char)223);
    lcd.print("   ");

    lcd.setCursor(0, 1);
    if (obstacleDetecte) {
        lcd.print("STOP! INTRUSION ");
        tone(PIN_BUZZER, 2000, 80); // Alarme sonore
    } else {
        lcd.print("Zone Libree    ");
        noTone(PIN_BUZZER);
    }

    // 4. Gestion du Mouvement : On ne fait tourner le servo QUE SI la zone est libre
    if (!obstacleDetecte) {
        currentAngle += step;

        // Inverser le sens du balayage aux extrémités
        if (currentAngle >= 180) {
            currentAngle = 180;
            step = -5;
        } else if (currentAngle <= 0) {
            currentAngle = 0;
            step = 5;
        }

        monServo.write(currentAngle);
    }

    delay(50); // Pause entre chaque pas
}