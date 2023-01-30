
#include <settings.h>

#pragma once
struct Energy {
    float values [N_BANDS] {-1};
    float average = -1;

    Energy complement();

    void multiply(const Energy &energy);
    void multiply(const float energy);

    float get_average();
};

std::ostream &operator<<(std::ostream &out, Energy &energy);