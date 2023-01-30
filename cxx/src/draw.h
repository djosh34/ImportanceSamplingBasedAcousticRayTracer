#pragma once
#include <rays/Ray.h>
#include <set>
#include "mesh.h"
#include "settings.h"


enum partyType {
    SENDER,
    RECEIVER
};

void drawShape(partyType sphereType, glm::vec3 center);
void drawScene(std::vector<Mesh> scene);
void drawSourcePlanes(std::vector<Mesh> &sourcePlanes);
void drawSources(std::vector<glm::vec3> sourceLocations);

class Draw {
public:
    Draw(int rays_cast) {
        this->rays_cast = rays_cast;
        this->step = rays_cast / MAX_AMOUNT_RAYS;
        if (rays_cast < MAX_AMOUNT_RAYS) {
            this->step = 1;
        }
    }

    void drawRays(std::vector<Ray> &vector);

    void drawRaysAnimation(std::vector<Ray> all_rays, float current_t);

private:
    int rays_cast;
    int step;

    void drawRay(Ray &ray);
    void drawRay(Ray &ray, float max_t);
    void drawRay(Ray &ray, glm::vec4 &ray_color);
    void drawRay(Ray &ray, float max_t, glm::vec4 &rayColor);

    glm::vec4 getRayColor(int ray_start_index);

    void drawHit(Ray &ray, glm::vec3 center);
    bool isLegalIndex(int index);

    void drawHits(Ray &ray);

    void drawOutgoing(Ray &ray);


};



