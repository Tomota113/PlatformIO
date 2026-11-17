#include <Arduino.h>
#include <math.h>
#include <arduinoFFT.h>
#include "model.h"

// Paramètres d'échantillonnage
const uint16_t SAMPLES = 256;          // Puissance de 2 obligatoire pour la FFT
const double SAMPLING_FREQ = 1000.0;   // 1000 Hz

// Buffers pour la FFT (Partie réelle et imaginaire)
double vReal[SAMPLES];
double vImag[SAMPLES];

// Création de l'objet FFT
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLING_FREQ);

// Fonction pour extraire l'amplitude à 50 Hz via la FFT
double extraire_amp_50hz(double signal[]) {
    // 1. Remplir les buffers
    for (int i = 0; i < SAMPLES; i++) {
        vReal[i] = signal[i];
        vImag[i] = 0.0;
    }

    // 2. Fenêtrage (Hamming) & Calcul FFT
    FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);
    FFT.compute(FFTDirection::Forward);
    FFT.complexToMagnitude();

    // 3. Calcul de la résolution fréquentielle (Hz par case/bin)
    double resolution = SAMPLING_FREQ / SAMPLES; // 1000 / 256 = 3.906 Hz
    int bin_50hz = (int)round(50.0 / resolution);

    // 4. Normalisation de l'amplitude
    double amp_50hz = (vReal[bin_50hz] * 2.0) / SAMPLES;
    return amp_50hz;
}
// Fonctions de calcul (DSP)
double calculer_rms(double signal[], int n) {
    double somme = 0.0;
    for (int i = 0; i < n; i++) {
        somme += signal[i] * signal[i];
    }
    return sqrt(somme / n);
}

double calculer_peak(double signal[], int n) {
    double max_val = 0.0;
    for (int i = 0; i < n; i++) {
        double val = fabs(signal[i]);
        if (val > max_val) max_val = val;
    }
    return max_val;
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("--- TEST INFERENCE TINYML SUR ESP32 ---");

    // Données d'entrée : [RMS, Peak, Crest_Factor, Amp_50Hz]
    double faux_signal_sain[4]  = {0.55, 1.10, 2.0, 0.45};
    double faux_signal_panne[4] = {1.80, 3.40, 1.88, 1.25};

    // Tableaux pour recevoir les probabilités [proba_sain, proba_panne]
    double output_sain[2]  = {0.0, 0.0};
    double output_panne[2] = {0.0, 0.0};

    // Appel de la fonction score avec entrée ET sortie
    score(faux_signal_sain, output_sain);
    score(faux_signal_panne, output_panne);

    Serial.println(">> Resultat Test 1 (Signal Sain attendu) :");
    Serial.print("  Proba Sain  : "); Serial.println(output_sain[0]);
    Serial.print("  Proba Panne : "); Serial.println(output_sain[1]);

    Serial.println(">> Resultat Test 2 (Signal Panne attendu) :");
    Serial.print("  Proba Sain  : "); Serial.println(output_panne[0]);
    Serial.print("  Proba Panne : "); Serial.println(output_panne[1]);
}


void loop() {
    // 1. Simulation d'un signal acquis (ex: 256 points)
    // Plus tard, ces valeurs viendront de la lecture I2C du MPU6050
    double signal_mesure[SAMPLES];
    for (int i = 0; i < SAMPLES; i++) {
        double t = (double)i / SAMPLING_FREQ;
        // Signal simulé sain : amplitude 0.5 à 50 Hz
        signal_mesure[i] = 0.5 * sin(2 * PI * 50.0 * t);
    }

    // 2. Extraction des 4 features (DSP)
    double rms          = calculer_rms(signal_mesure, SAMPLES);
    double peak         = calculer_peak(signal_mesure, SAMPLES);
    double crest_factor = peak / rms;
    double amp_50hz     = extraire_amp_50hz(signal_mesure);

    // 3. Vecteur d'entrée pour l'arbre TinyML
    double input[4]  = {rms, peak, crest_factor, amp_50hz};
    double output[2] = {0.0, 0.0};

    // 4. Inférence
    score(input, output);

    // 5. Affichage & Diagnostic
    Serial.print("RMS: "); Serial.print(rms, 2);
    Serial.print(" | 50Hz: "); Serial.print(amp_50hz, 2);
    Serial.print(" ==> ");

    if (output[1] > 0.5) {
        Serial.println("ETAT: [ ANOMALIE DETECTEE ]");
    } else {
        Serial.println("ETAT: [ MOTEUR SAIN ]");
    }

    delay(1000);
}