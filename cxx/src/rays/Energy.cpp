#include "Energy.h"

Energy Energy::complement() {
    Energy complement OneEnergyTemplate;

    for (int i = 0; i < N_BANDS; i++) {
        complement.values[i] -= this->values[i];
    }

    return complement;
}

void Energy::multiply(const Energy &energy) {
    for (int i = 0; i < N_BANDS; i++) {
        this->values[i] *= energy.values[i];
    }
}

void Energy::multiply(const float energy) {
    for (int i = 0; i < N_BANDS; i++) {
        this->values[i] *= energy;
    }
}

float Energy::get_average() {
    if (average > -1) {
        return average;
    }

    average = 0;
    for (int band = 0; band < N_BANDS; band++) {
        average += values[band];
    }

    return average;
}

std::ostream &operator<<(std::ostream &out, Energy &energy) {
    out << "[";

    for (int i = 0; i < N_BANDS; i++) {
        out << energy.values[i];
        if (i < N_BANDS - 1) {
            out << ",";
        }
    }
    out << "]";

    return out;
}

