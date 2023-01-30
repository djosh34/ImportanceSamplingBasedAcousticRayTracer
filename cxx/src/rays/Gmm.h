

#pragma once
#include "ray.h"


class Gmm {

public:
    bool initialized = false;

    void generateRays(std::vector<Ray> &all_rays, glm::vec3 startPoint, RaySettings &ray_settings, int seed);

    std::vector<InitNums> findHitProjectionCoords(std::vector<Ray> &all_rays);

    std::vector<InitNums> pythonNewDirectionCoords(const std::vector<InitNums> &coords, int num_directions);
};

void generateRays(std::vector<Ray> &all_rays, glm::vec3 startPoint, RaySettings &ray_settings, int seed);

