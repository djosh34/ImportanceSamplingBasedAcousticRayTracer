

#include "projections.h"

glm::vec3 convertCoordsToVector(const SphereCoords coords) {
    // https://stackoverflow.com/questions/1185408/converting-from-longitude-latitude-to-cartesian-coordinates

    float x = std::cos(coords.latitude) * std::cos(coords.longitude);
    float y = std::cos(coords.latitude) * std::sin(coords.longitude);
    float z = std::sin(coords.latitude);

    return {x, y, z};
}

glm::vec3 getDirectionFromZetTheta(const ProjectedCoords projectedCoords) {
    // https://angms.science/doc/RM/randUnitVec.pdf

    // [-1, 1]
    double z_3d = projectedCoords.x * 2 - 1;
    // [0, 2pi)
    double theta = 2.0f * projectedCoords.y * M_PI;

    double x_3d = std::sqrt(1 - z_3d*z_3d) * std::cos(theta);
    double y_3d = std::sqrt(1 - z_3d*z_3d) * std::sin(theta);


    return {x_3d, y_3d, z_3d};
}

glm::vec3 getDirectionsFromEquiRect(const ProjectedCoords projectedCoords) {

    // [-1, 1]
    double x_2d = projectedCoords.x * 2 - 1;

    // [-1, 1]
    double y_2d = projectedCoords.y * 2 - 1;

    // https://mathworld.wolfram.com/CylindricalEquidistantProjection.html
    double latitude = x_2d * M_PI  * 0.5;
    double longitude = y_2d * M_PI;

    return convertCoordsToVector({latitude, longitude});
}
