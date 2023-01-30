

#include "Gmm.h"
#include "RayTracing.h"
#include <fstream>
#include <boost/tokenizer.hpp>


std::vector<InitNums> Gmm::pythonNewDirectionCoords(const std::vector<InitNums> &coords, int num_directions) {
    // Create a temporary CSV file
    std::ofstream tempFile("./Postprocessing/io/hitCoords.csv");
    tempFile << "x,y" << std::endl;
    for (const auto& coord : coords) {
        tempFile << coord.projectedCoords.x << "," << coord.projectedCoords.y << std::endl;
    }
    tempFile.close();

    // Execute the Python script and pass the temporary CSV file as an argument
    std::cout << "Loading python......" << std::flush;
    std::string command = "/usr/bin/python3 ./Postprocessing/hitCoords.py ./Postprocessing/io/hitCoords.csv ./Postprocessing/io/newDirectionsCoords.csv " + std::to_string(num_directions);
    int result = system(command.c_str());
    std::cout << "\rPython loaded" << std::endl;


    // Read the output of the Python script
    std::vector<InitNums> newCoords;
    std::string line;
    std::ifstream outputFile("./Postprocessing/io/newDirectionsCoords.csv");

    while (std::getline(outputFile, line)) {
        boost::tokenizer<boost::escaped_list_separator<char>> tokens(line);
        std::vector<std::string> vec(tokens.begin(), tokens.end());
        newCoords.emplace_back(InitNums{ProjectedCoords{std::stod(vec[0]), std::stod(vec[1])}, std::stod(vec[2])});
    }

    outputFile.close();
    return newCoords;

}

std::vector<InitNums> Gmm::findHitProjectionCoords(std::vector<Ray> &all_rays) {
    Sphere receiverSphere {global_config->RECEIVER_LOCATION, global_config->RECEIVER_RADIUS};
    std::vector<InitNums> coords;

    for (Ray &ray : all_rays) {
        if (!intersectWithSphere(receiverSphere, ray).has_value()) {
            continue;
        }

        // this ray has gone through the receiver
        coords.push_back(ray.initNums);
    }
    return coords;
}

void Gmm::generateRays(std::vector <Ray> &all_rays, glm::vec3 startPoint, RaySettings &ray_settings, int seed) {
    if (!initialized) {
        ::generateRays(all_rays, startPoint, ray_settings, seed);
        initialized = true;
        return;
    }

    std::vector<InitNums> hitProjectionCoords = findHitProjectionCoords(all_rays);
    std::vector<InitNums> newDirectionProjectionCoords = pythonNewDirectionCoords(hitProjectionCoords, ray_settings.amount_of_rays);

    std::vector<StartingDirection> directions = generateDirectionsFromCoords(newDirectionProjectionCoords);
    generateRaysFromDirections(all_rays, startPoint, ray_settings, directions);
    return;
}


void generateRays(std::vector<Ray> &all_rays, glm::vec3 startPoint, RaySettings &ray_settings, int seed) {
    GenerateDirections generateDirections {seed};
    std::vector<StartingDirection> directions = generateDirections.generateDirections(ray_settings.amount_of_rays);
    generateRaysFromDirections(all_rays, startPoint, ray_settings, directions);
}
