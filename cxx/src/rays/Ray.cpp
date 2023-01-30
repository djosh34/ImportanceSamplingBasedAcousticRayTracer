

#include <glm/geometric.hpp>
#include "Ray.h"
#include "RayTracing.h"
#include "directionGenerator.h"
#include <vector>
#include <boost/range/irange.hpp>
#include <thread>
#include <random>


void calculatePlaneNormal(VertexTriangle &triangle) {
    triangle.planeNormal = glm::normalize(glm::cross(triangle.vertex_0.p - triangle.vertex_2.p, triangle.vertex_1.p - triangle.vertex_2.p));
    triangle.planeD = glm::dot(triangle.planeNormal, triangle.vertex_0.p);
}

void initialize_meshes(RaySettings &ray_settings) {
    // create all vertexTriangles
    for (Mesh &mesh : ray_settings.meshes) {
        for (Triangle triangle: mesh.triangles) {
            VertexTriangle currentVertexTriangle{
                    mesh.vertices.at(triangle.x),
                    mesh.vertices.at(triangle.y),
                    mesh.vertices.at(triangle.z)
            };

            calculatePlaneNormal(currentVertexTriangle);


            mesh.vertexTriangles.push_back(currentVertexTriangle);
        }
    }
}

void castRayIteration(int i, std::vector<Ray> &all_rays, glm::vec3 &startPoint, std::vector<StartingDirection> &directions,
                      RaySettings &ray_settings, int block, GenerateDirections *generateDirections, int* ptotal_rays_done) {
    int s = i * block;
    int e = s + block;
    for (int ray_i = s; ray_i < e; ray_i++) {
        if (ray_i % 10000 == 0) {
            std::cout << "\rCasting rays: ";
            std::cout << *ptotal_rays_done << "/" << ray_settings.amount_of_rays;
            *ptotal_rays_done += 10000;
        }
        castRay(all_rays, startPoint, directions.at(ray_i), ray_settings, ray_i, generateDirections);
    }
}


void generateRaysFromDirections(std::vector<Ray> &all_rays, glm::vec3 startPoint, RaySettings &ray_settings,
                                std::vector<StartingDirection> &directions) {
    int block = ray_settings.amount_of_rays / NUM_THREADS;
    std::vector<std::thread> threads = std::vector<std::thread>();

    int total_rays_done = 0;
    int* ptotal_rays_done = &total_rays_done;
    for (int i = 0; i < NUM_THREADS; i++) {
        GenerateDirections generateDirections{i};
        std::thread t(castRayIteration, i, std::ref(all_rays), std::ref(startPoint), std::ref(directions), std::ref(ray_settings), block, &generateDirections, ptotal_rays_done);
        threads.push_back(std::move(t));
    }

    std::cout << "\rCasting rays: ";
    std::cout << *ptotal_rays_done << "/" << ray_settings.amount_of_rays;

    for (auto& th : threads) {
        th.join();
    }
    std::cout << std::endl;
}



void castRay(std::vector<Ray> &all_rays, glm::vec3 &starting_point, StartingDirection &startingDirection,
             RaySettings &ray_settings, int rayIndex, GenerateDirections *generateDirection) {



    for (int hit_level = 0; hit_level < ray_settings.max_hit_level; hit_level++) {
        int index = rayIndex + ray_settings.amount_of_rays * hit_level;

        if (hit_level == 0) {
            Energy oneEnergy OneEnergyTemplate;
            Ray newRay{starting_point, startingDirection.d, startingDirection.initNums, rayIndex, oneEnergy, generateDirection};
            detectHit(newRay, ray_settings);

            all_rays.at(index) = newRay;
        } else {
            Ray &prevRay = all_rays.at(index - (ray_settings.amount_of_rays));

            if (!prevRay.hit) {
                break;
            }

            Ray reflectedRay = prevRay.getReflectionRay(hit_level);
            reflectedRay.total_previous_t = prevRay.total_previous_t + prevRay.t;

            detectHit(reflectedRay, ray_settings);

            reflectedRay.updateEnergyOfRayAfterHit(prevRay);
            all_rays.at(index) = reflectedRay;
        }

    }
}

std::ostream &operator<<(std::ostream &s, const Ray &ray) {
    return s << "(o: " << ray.origin << ", d: " << ray.direction << ", hp:" << ray.t * ray.direction + ray.origin << " index: " << ray.ray_start_index << ")";
}


void Ray::updateEnergyOfRayAfterHit(Ray &ray) {
    // https://reuk.github.io/wayverb/theory.html equation 21
    this->current_energy.multiply(ray.hitInfo.hitAudioReflection->absorption_coefficient.complement());
    this->current_energy.multiply(ray.hitInfo.hitAudioReflection->scattering_coefficient.complement());
}

Ray Ray::getReflectionRay(int hit_level) {
    glm::vec3 normal = hitInfo.hitNormal;

    float average_scattering = hitInfo.hitAudioReflection->scattering_coefficient.get_average();

    if (!RANDOM_REFLECTION_RAYS) {
        average_scattering = 0.0f;
    }

    glm::vec3 randomUnitVector = generateDirections->getRandomDirection();
    glm::vec3 reflectionVector = glm::reflect(direction, normal);

    reflectionVector = randomUnitVector * average_scattering + reflectionVector * (1 - average_scattering);


    glm::vec3 rayHitPoint = hitInfo.hitPoint;

    Ray reflectionRay{
            rayHitPoint,
            reflectionVector,
            this
    };

    reflectionRay.hit_level = hit_level;

    return reflectionRay;
}


void RaySettings::initialize_source_locations(std::vector<Mesh> &sourcePlanes) {
    std::vector<glm::vec3> sourceLocationsA;
    std::vector<glm::vec3> sourceLocationsB;


    for (Mesh &mesh : sourcePlanes) {

        glm::vec3 vertexA0 = mesh.vertices[0].p;
        glm::vec3 vertexA1 = mesh.vertices[1].p;
        glm::vec3 vertexB0 = mesh.vertices[3].p;
        glm::vec3 vertexB1 = mesh.vertices[2].p;

        generateBetweenLocations(sourceLocationsA, vertexA0, vertexA1);
        generateBetweenLocations(sourceLocationsB, vertexB0, vertexB1);

        if (sourceLocationsA.size() == 0) {
            sourceLocations.push_back(vertexA0);
            sourceLocations.push_back(vertexA1);
            sourceLocations.push_back(vertexB0);
            sourceLocations.push_back(vertexB1);
            continue;
        }

        for (int i = 0; i < sourceLocationsA.size(); i++) {
            generateBetweenLocations(sourceLocations, sourceLocationsA.at(i), sourceLocationsB.at(i));
        }



    }

}

void RaySettings::generateBetweenLocations(std::vector<glm::vec3> &locations, glm::vec3 &vertex_0, glm::vec3 &vertex_1) {
    float dist = glm::length(vertex_0 - vertex_1);
    int number_of_locations_in_row = std::ceil(dist / (2.0 * global_config->MIN_SOURCE_RADIUS));
    int locations_in_row_denomator = number_of_locations_in_row - 1;

    if (locations_in_row_denomator == 0) {
        return;
    }

    for (int i = 0; i < number_of_locations_in_row; i++) {

        float in_between_factor = (float) i / (float) locations_in_row_denomator;
        glm::vec3 location = vertex_0 * in_between_factor + vertex_1 * (1 - in_between_factor);
        locations.push_back(location);
    }
}
