#pragma once

void score(double * input, double * output) {
    double var0[2];

    // Exemple de structure if/else générée :
    if (input[3] <= 0.82) {
        if (input[0] <= 0.65) {
            var0[0] = 1.0;
            var0[1] = 0.0;
        } else {
            var0[0] = 0.6;
            var0[1] = 0.4;
        }
    } else {
        var0[0] = 0.0;
        var0[1] = 1.0;
    }

    output[0] = var0[0];
    output[1] = var0[1];
}