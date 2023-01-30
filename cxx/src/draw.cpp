#include "draw.h"
#include "disable_all_warnings.h"
#ifdef __APPLE__
/* Defined before OpenGL and GLUT includes to avoid deprecation messages */
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif
#include <glm/gtc/type_ptr.hpp>



static void setMaterial(const Material& material)
{
    glColor4fv(glm::value_ptr(glm::vec4(material.kd, STANDARD_TRANSPARENCY)));
}

void drawShape(const Mesh& mesh)
{


    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glEnable(GL_LINE_SMOOTH);

    for (const auto& triangleIndex : mesh.triangles) {
        setMaterial(mesh.material);
        glBegin(GL_TRIANGLES);

        for (int i = 0; i < 3; i++) {
            const auto& vertex = mesh.vertices[triangleIndex[i]];
            glNormal3fv(glm::value_ptr(vertex.n)); // Normal.
            glVertex3fv(glm::value_ptr(vertex.p)); // Position.
        }
        glEnd();

        glBegin(GL_LINE_LOOP);
        glLineWidth(2.0); // 3-pixel line width
        glColor3f(0.0, 0.0, 0.0);

        for (int i = 0; i < 3; i++) {
            const auto& vertex = mesh.vertices[triangleIndex[i]];
            glNormal3fv(glm::value_ptr(vertex.n)); // Normal.
            glVertex3fv(glm::value_ptr(vertex.p)); // Position.
        }
        glEnd();

    }
}

void drawSphere(const glm::vec3 &center, float radius, const glm::vec3 &color, float opacity, bool light) {


    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    const glm::mat4 transform = glm::translate(glm::identity<glm::mat4>(), center);
    glMultMatrixf(glm::value_ptr(transform));
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glColor4fv(glm::value_ptr(glm::vec4(color, opacity)));

    auto quadric = gluNewQuadric();
    gluSphere(quadric, radius, 50, 20);
    gluDeleteQuadric(quadric);

    glPopMatrix();
}

void drawShape(partyType sphereType, glm::vec3 center)
{

    switch (sphereType) {
        case SENDER:
            drawSphere(center, SOURCE_RADIUS, SENDER_COLOR, PARTY_OPACITY, false);
            break;
        case RECEIVER:
            drawSphere(center, global_config->RECEIVER_RADIUS, RECEIVER_COLOR, PARTY_OPACITY, false);
            break;
    }
}

void Draw::drawHit(Ray &ray, glm::vec3 center) {
    drawSphere(center, RAY_RADIUS, getRayColor(ray.ray_start_index), 1.0f, false);
}

void drawScene(std::vector<Mesh> scene) {
    for (const auto &mesh: scene)
        drawShape(mesh);
}

glm::vec4 Draw::getRayColor(int ray_start_index) {
    // https://www.codespeedy.com/hsv-to-rgb-in-cpp/

    float H = ((float) ray_start_index / (float) rays_cast) * 360.0f;
    float s = 1.0f;
    float v = 1.0f;

    float C = s*v;
    float X = C*(1-abs(fmod(H/60.0, 2)-1));
    float m = v-C;
    float r,g,b;

    if(H >= 0 && H < 60){
        r = C,g = X,b = 0;
    }
    else if(H >= 60 && H < 120){
        r = X,g = C,b = 0;
    }
    else if(H >= 120 && H < 180){
        r = 0,g = C,b = X;
    }
    else if(H >= 180 && H < 240){
        r = 0,g = X,b = C;
    }
    else if(H >= 240 && H < 300){
        r = X,g = 0,b = C;
    }
    else{
        r = C,g = 0,b = X;
    }


    return {r+m, g+m, b+m, RAY_TRANSPARENCY};
}

void Draw::drawRay(Ray &ray, float max_t, glm::vec4 &rayColor) {
    if (!DRAW_RAY_LINES && !DRAW_RAY_POINTS) {
        return;
    }

    const glm::vec3 hitPoint = ray.origin + std::clamp(ray.t, 0.0f, max_t) * ray.direction;

    glLineWidth(RAY_LINE_WIDTH);

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glBegin(GL_LINES);

    if (DRAW_RAY_LINES) {
        glColor4fv(glm::value_ptr(rayColor));
        glVertex3fv(glm::value_ptr(ray.origin));
        glColor4fv(glm::value_ptr(rayColor));
        glVertex3fv(glm::value_ptr(hitPoint));
    }
    glEnd();

    if (ray.hit && DRAW_RAY_POINTS)
        drawHit(ray, hitPoint);

    glPopAttrib();
}

void Draw::drawRay(Ray &ray, float max_t) {
    glm::vec4 ray_color = getRayColor(ray.ray_start_index);
    drawRay(ray, max_t, ray_color);
}

void Draw::drawRay(Ray &ray, glm::vec4 &ray_color) {
    drawRay(ray, 100.0f, ray_color);
}

void Draw::drawRay(Ray &ray) {
    drawRay(ray, 100.0f);
}


void Draw::drawRays(std::vector<Ray> &vector) {

    for (Ray &ray : vector) {

        if (DRAW_ONLY_INTERSECTIONS) {
            if (ray.received_chain) {
                drawRay(ray);
            }
            continue;
        }


        if (!isLegalIndex(ray.ray_start_index)) {
            continue;
        }




        drawRay(ray);
    }
}

bool Draw::isLegalIndex(int index) {
    if (!ENABLE_VIEW_FILTERING) {
        return true;
    }

    // for debugging purposes, doesn't effect simulation
    if (index == 1 && DRAW_INDEX_1) {
        return true;
    }

    return index % step == 0;
}

void Draw::drawHits(Ray &ray) {
    glm::vec4 ray_color = ray.received ? RAY_HIT_COLOR : RAY_MISS_COLOR;
    drawRay(ray, ray_color);

}

void Draw::drawOutgoing(Ray &ray) {
    if (!isLegalIndex(ray.ray_start_index)) {
        return;
    }

    if (ray.hit_level == 0) {
        drawRay(ray);
    }

}

void Draw::drawRaysAnimation(std::vector<Ray> all_rays, float current_t) {
    for (Ray &ray : all_rays) {
        if (!isLegalIndex(ray.ray_start_index)) {
            continue;
        }

        float rayPrevT = ray.total_previous_t;
        float rayT = ray.t;
        float totalT = rayPrevT + rayT;
        float diffT = current_t - rayPrevT;

        if (current_t > totalT) {
            drawRay(ray);
        }

        if (diffT > 0) {
            drawRay(ray, diffT);
        }

    }
}


void drawSources(std::vector<glm::vec3> sourceLocations) {
    for (auto sourceLocation : sourceLocations) {
        drawSphere(sourceLocation, SOURCE_RADIUS, SENDER_COLOR, PARTY_OPACITY, false);
    }
}

void drawSourcePlanes(std::vector<Mesh> &sourcePlanes) {
    for (auto sourcePlane : sourcePlanes) {
        for (const auto& triangleIndex : sourcePlane.triangles) {

            glColor4f(SOURCE_PLANE_COLOR);
            glBegin(GL_TRIANGLES);

            for (int i = 0; i < 3; i++) {
                const auto &vertex = sourcePlane.vertices[triangleIndex[i]];
                glNormal3fv(glm::value_ptr(vertex.n)); // Normal.
                glVertex3fv(glm::value_ptr(vertex.p)); // Position.
            }
            glEnd();
        }
    }
}




