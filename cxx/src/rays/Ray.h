
#include <glm/vec3.hpp>
#include <iostream>
#include <Mesh.h>
#include <optional>
#include <boost/optional.hpp>
#include "Energy.h"
#include "directionGenerator.h"


#pragma once
struct HitInfo {
    glm::vec3 hitNormal;
    glm::vec3 hitPoint;
    float incomingT;
    AudioReflection *hitAudioReflection;
};

struct RaySettings {
    int amount_of_rays;
    int max_hit_level;
    std::vector<Mesh> meshes;
    std::vector<glm::vec3> sourceLocations;

    void initialize_source_locations(std::vector<Mesh> &sourcePlanes);

    void generateBetweenLocations(std::vector<glm::vec3> &locations, glm::vec3 &vertex_0, glm::vec3 &vertex_1);
};

class Ray {
    public:

    Ray() = default;

    glm::vec3 origin {0.0f };
    glm::vec3 direction { 0.0f, 0.0f, -1.0f };
    InitNums initNums;
    HitInfo hitInfo;
    float closeness_to_sphere = infT;
    float t { std::numeric_limits<float>::max() };
    float total_previous_t = 0;
    bool hit {false};
    bool received {false};
    bool received_chain {false};
    int hit_level;
    int ray_start_index;
    Energy current_energy;
    float probability_ray_chosen = 1.0f;
    GenerateDirections* generateDirections;

    // diffuse and check ray creation
    Ray(glm::vec3 starting_point, glm::vec3 direction) {
        this->origin = starting_point;
        this->direction = direction;
        this->hit_level = -1;
        this->ray_start_index = -1;
    }

    // hit level zero ray creation
    Ray(glm::vec3 starting_point, glm::vec3 direction, InitNums &initNums, int ray_start_index, Energy current_energy, GenerateDirections *generator) {
        this->origin = starting_point;
        this->direction = direction;
        this->initNums = {initNums};
        this->hit_level = 0;
        this->ray_start_index = ray_start_index;
        this->current_energy = {current_energy};
        this->generateDirections = generator;
    }

    // reflection ray copy
    Ray(glm::vec3 starting_point, glm::vec3 direction, const Ray *templateRay) {
        this->origin = starting_point;
        this->direction = direction;
        this->hit_level = 0;
        this->initNums = {templateRay->initNums};
        this->total_previous_t = templateRay->total_previous_t;
        this->ray_start_index = templateRay->ray_start_index;
        this->current_energy = {templateRay->current_energy};
        this->probability_ray_chosen = templateRay->probability_ray_chosen;
        this->generateDirections = templateRay->generateDirections;

    }

    Ray getReflectionRay(int hit_level);

    void updateEnergyOfRayAfterHit(Ray &ray);
};

std::ostream& operator<<(std::ostream &s, const Ray &ray);

void initialize_meshes(RaySettings &ray_settings);
void generateRays(std::vector<Ray> &all_rays, glm::vec3 startPoint, RaySettings &ray_settings, int seed);
void generateRaysFromDirections(std::vector<Ray> &all_rays, glm::vec3 startPoint, RaySettings &ray_settings, std::vector<StartingDirection> &directions);
void castRay(std::vector<Ray> &all_rays, glm::vec3 &starting_point, StartingDirection &startingDirection,
             RaySettings &ray_settings, int rayIndex, GenerateDirections *generateDirection);
