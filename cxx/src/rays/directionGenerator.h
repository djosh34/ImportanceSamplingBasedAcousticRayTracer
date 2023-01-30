#pragma once
#include <iostream>
#include <vector>
#include <glm/vec3.hpp>
#include <random>
#include "projections.h"
#include "Coords.h"


// Initial probability conditions of a given ray
struct InitNums {
    InitNums();
    InitNums(ProjectedCoords coords, double probability);

    InitNums(ProjectedCoords coords);

    ProjectedCoords projectedCoords;
    double probability;
};

struct StartingDirection {
    glm::vec3 d;
    InitNums initNums;
};

class GenerateDirections {
public:
    GenerateDirections(int seed) {
        // https://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution
        //  std::random_device rd;  //Will be used to obtain a seed for the random number engine
        //  generator = std::mt19937(rd()); //Standard mersenne_twister_engine seeded with rd()
        generator = std::mt19937(seed); //Standard mersenne_twister_engine seeded with rd()
    }

    std::vector<StartingDirection> generateDirections(int rays);
    void generateDirections(std::vector<StartingDirection> &directions, int rays);

    glm::vec3 getRandomDirection();

private:
    std::mt19937 generator;
};

std::vector<StartingDirection> generateDirectionsFromCoords(std::vector<InitNums> projectedCoords);
glm::vec3 getDirectionFrom2D(ProjectedCoords projectedCoords);


