
#include <glm/geometric.hpp>
#include "RayTracing.h"
#include "helpers/printHelper.h"
#include <boost/optional.hpp>
#include <settings.h>


bool pointInTriangle(const VertexTriangle &triangle, const glm::vec3 &hitPoint);

float intersectWithPlane(const Ray &ray, const VertexTriangle &triangle) {


    float t = (triangle.planeD - glm::dot(ray.origin, triangle.planeNormal)) / glm::dot(ray.direction, triangle.planeNormal);
    if (std::abs(t) < EPSILON) return infT; /// mega improvement
    
    return t;
}

bool intersectWithTriangle(Ray &ray, const VertexTriangle &triangle, AudioReflection *audioReflection) {

    float t = intersectWithPlane(ray, triangle);
    
    if (t >= ray.t || t < 0) {
        return false;
    }

    glm::vec3 hitPoint = ray.origin + ray.direction * t;

    if (!pointInTriangle(triangle, hitPoint)) {
        return false;
    }

    glm::vec3 normal = triangle.planeNormal;

    if (glm::dot((ray.origin - hitPoint), normal) <= 0) {
        normal = -normal;
    }

    ray.t = t;
    ray.hit = true;
    ray.hitInfo = HitInfo{normal, hitPoint};
    ray.hitInfo.incomingT = ray.total_previous_t;
    ray.hitInfo.hitAudioReflection = audioReflection;

    return true;
}

bool pointInTriangle(const VertexTriangle &triangle, const glm::vec3 &hitPoint) {

    glm::vec3 c0 = glm::cross(hitPoint - triangle.vertex_2.p, triangle.vertex_1.p - triangle.vertex_2.p);
    glm::vec3 c1 = glm::cross(hitPoint - triangle.vertex_0.p, triangle.vertex_2.p - triangle.vertex_0.p);
    glm::vec3 c2 = glm::cross(hitPoint - triangle.vertex_1.p, triangle.vertex_0.p - triangle.vertex_1.p);


    /// If the normal and the cross product point in the same direction, then the sign is positive
    /// Otherwise it is negative
    float sign0 = glm::dot(c0, triangle.vertex_0.n);
    float sign1 = glm::dot(c1, triangle.vertex_0.n);
    float sign2 = glm::dot(c2, triangle.vertex_0.n);


    if (sign0 >= 0 && sign1 >= 0 && sign2 >= 0)
        return true;
    return false;
}

std::optional<float> intersectWithSphere(Sphere &sphere, Ray &ray) {
    float dx = ray.direction.x;
    float dy = ray.direction.y;
    float dz = ray.direction.z;

    float ox = ray.origin.x - sphere.center.x;
    float oy = ray.origin.y - sphere.center.y;
    float oz = ray.origin.z - sphere.center.z;

    float r = sphere.radius;

    float A = dx * dx + dy * dy + dz * dz;
    float B = 2 * (dx * ox + dy * oy + dz * oz);
    float C = ox * ox + oy * oy + oz * oz - r * r;

    float discriminant = B * B - 4 * A * C;

    if (discriminant < EPSILON)
        return std::nullopt;


    float t0 = (-B - std::sqrt(discriminant)) / 2 * A;
    float t1 = (-B + std::sqrt(discriminant)) / 2 * A;

    float average_t = (t0 + t1) / 2.0f;

    if (average_t < 0)
        return std::nullopt;

    if (ray.t < average_t) {
        return std::nullopt;
    }

    return average_t;
}

void detectHit(Ray &ray, const RaySettings &ray_settings) {

    for (const Mesh &mesh : ray_settings.meshes) {
        for (const VertexTriangle &triangle : mesh.vertexTriangles) {
            intersectWithTriangle(ray, triangle, mesh.audioReflection);
        }
    }
}
