#include <Arduino.h>
#include <Servo.h>

const int trigPin = 10;
const int echoPin = 11;
const int servoPin = 9;

Servo myServo;

long duration;
int distance;

int calculateDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Timeout de 15000 µs (~250 cm) pour éviter les blocages de mesure
  duration = pulseIn(echoPin, HIGH, 15000);
  if (duration == 0) return 400; // Hors de portée ou pas d'écho
  
  int dist = duration * 0.034 / 2;
  if (dist < 2 || dist > 400) return 400; // Filtrer valeurs aberrantes
  return dist;
}

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600);
  myServo.attach(servoPin);
}

void loop() {
  // Balayage 0 -> 180°
  for (int i = 0; i <= 180; i += 2) {
    myServo.write(i);
    delay(20);
    distance = calculateDistance();
    Serial.print(i);
    Serial.print(",");
    Serial.println(distance);
  }
  
  // Balayage 180 -> 0°
  for (int i = 180; i >= 0; i -= 2) {
    myServo.write(i);
    delay(20);
    distance = calculateDistance();
    Serial.print(i);
    Serial.print(",");
    Serial.println(distance);
  }
}