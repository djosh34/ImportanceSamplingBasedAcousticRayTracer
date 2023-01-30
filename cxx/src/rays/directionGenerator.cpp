#include <glm/gtx/rotate_vector.hpp>
#include <settings.h>
#include <config.h>
#include "projections.h"
#include "directionGenerator.h"


void GenerateDirections::generateDirections(std::vector<StartingDirection> &directions, int rays) {
    for (int i = 0; i < rays; i++) {
        std::uniform_real_distribution<> dis(0, 1.0);

        ProjectedCoords projectedCoords = {dis(generator), dis(generator)};

        glm::vec3 direction = getDirectionFrom2D(projectedCoords);
        StartingDirection startingDirection{
            direction,
            InitNums(projectedCoords)
        };
        directions.push_back(startingDirection);
    }
}
std::vector<StartingDirection> generateDirectionsFromCoords(std::vector<InitNums> projectedCoords) {
    std::vector<StartingDirection> directions;

    for (auto initNum : projectedCoords) {

        glm::vec3 direction = getDirectionFrom2D(initNum.projectedCoords);
        StartingDirection startingDirection{
                direction,
                initNum
        };
        directions.push_back(startingDirection);
    }

    return directions;
}


std::vector<StartingDirection> GenerateDirections::generateDirections(int rays) {
    std::vector<StartingDirection> directions;
    this->generateDirections(directions, rays);
    return directions;
}

glm::vec3 getDirectionFrom2D(ProjectedCoords projectedCoords) {

    switch (global_config->PROJECTION_METHOD) {
        case ZET_THETA:
            return getDirectionFromZetTheta(projectedCoords);
        case EQUI_RECT:
            return getDirectionsFromEquiRect(projectedCoords);
    }

    std::cerr << "Invalid PROJECTION_METHODS chosen" << std::endl;
    throw std::exception();
}

glm::vec3 GenerateDirections::getRandomDirection() {
    std::uniform_real_distribution<> dis(0, 1.0);

    double x = dis(generator);
    double y = dis(generator);

    return getDirectionFrom2D({x, y});
}



InitNums::InitNums() {
    this->probability = 1.0;
}

InitNums::InitNums(ProjectedCoords projectedCoords, double probability) {
    this->projectedCoords = projectedCoords;
    this->probability = probability;
}

InitNums::InitNums(ProjectedCoords coords) {
    this->projectedCoords = coords;
    this->probability = 1.0;
}


