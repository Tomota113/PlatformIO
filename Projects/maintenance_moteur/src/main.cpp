#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <arduinoFFT.h>

#define DHTPIN 4
#define DHTTYPE DHT11

const uint8_t MPU_ADDR   = 0x68;
const int PIN_ENABLE     = 25;
const int PIN_IN1        = 26;
const int PIN_IN2        = 27;
const int PIN_LED        = 18;
const int PIN_BUZZER     = 19;

// Paramètres FFT
const uint16_t SAMPLES = 64;             // Puissance de 2
const double SAMPLING_FREQ = 200;        // 200 Hz
double vReal[SAMPLES];
double vImag[SAMPLES];

ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLING_FREQ);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
    Serial.begin(115200);
    delay(500);

    Wire.begin(21, 22);
    Wire.setClock(100000);

    // Réveil MPU6050
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission(true);

    pinMode(PIN_IN1, OUTPUT);
    pinMode(PIN_IN2, OUTPUT);
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);

    digitalWrite(PIN_IN1, HIGH);
    digitalWrite(PIN_IN2, LOW);

    // PWM Moteur
    ledcSetup(0, 1000, 8);
    ledcAttachPin(PIN_ENABLE, 0);
    ledcWrite(0, 200); // Vitesse de travail stable

    dht.begin();
    Serial.println("\n--- PIPELINE VIBRATION & FFT OPERATIONNEL ---");
    Serial.println("Format: [RMS_g, Peak_g, FreqDominante_Hz, Temp_C]");
}

void loop() {
    unsigned long microSecondsStep = 1000000 / SAMPLING_FREQ;
    float sumSquares = 0.0;
    float peakVal = 0.0;

    // 1. Échantillonnage haute vitesse
    for (int i = 0; i < SAMPLES; i++) {
        unsigned long nextSample = micros() + microSecondsStep;

        Wire.beginTransmission(MPU_ADDR);
        Wire.write(0x3B);
        Wire.endTransmission(true);
        Wire.requestFrom(MPU_ADDR, (uint8_t)6, (uint8_t)true);

        if (Wire.available() >= 6) {
            int16_t raw_az = Wire.read() << 8 | Wire.read();
            Wire.read(); Wire.read();
            Wire.read(); Wire.read();

            float az = (float)raw_az / 16384.0; // en g

            vReal[i] = az;
            vImag[i] = 0.0;

            sumSquares += az * az;
            if (abs(az) > peakVal) peakVal = abs(az);
        }

        while (micros() < nextSample);
    }

    // 2. Calcul du RMS
    float rms = sqrt(sumSquares / SAMPLES);

    // 3. Calcul FFT
    FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
    FFT.compute(FFTDirection::Forward);
    FFT.complexToMagnitude();
    double dominantFreq = FFT.majorPeak();

    // 4. Lecture Température
    float temp = dht.readTemperature();

    // 5. Affichage structuré
    Serial.print("RMS: "); Serial.print(rms, 3);
    Serial.print(" g | Peak: "); Serial.print(peakVal, 3);
    Serial.print(" g | Freq: "); Serial.print(dominantFreq, 1);
    Serial.print(" Hz | Temp: "); Serial.print(temp, 1);
    Serial.println(" °C");

    delay(200);
}