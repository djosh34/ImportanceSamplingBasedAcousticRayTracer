#pragma once
#include "disable_all_warnings.h"
#include "config.h"
// Suppress warnings in third-party code.
DISABLE_WARNINGS_PUSH()
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <boost/filesystem.hpp>
#include <vector>
#include <GLFW/glfw3.h>
#include <glm/vec4.hpp>
#include <helpers/printHelper.h>
#include <settings.h>
#include <rays/Energy.h>

struct Vertex {
    glm::vec3 p; // Position.
    glm::vec3 n; // Normal.
};

struct Material {
    glm::vec3 kd; // Diffuse color.
};

struct AudioReflection {
    Energy scattering_coefficient;
    Energy absorption_coefficient;
    AudioReflection(Energy &SCATTERING_COEFFICIENT, Energy &ABSORPTION_COEFFICIENT);
};

struct VertexTriangle {
    Vertex vertex_0;
    Vertex vertex_1;
    Vertex vertex_2;
    glm::vec3 planeNormal;
    float planeD;
};

struct Sphere {
    glm::vec3 center;
    float radius;
};

std::ostream &operator<<(std::ostream &out, const VertexTriangle &vertexTriangle);

using Triangle = glm::uvec3;

struct Mesh {
    // Vertices contain the vertex positions and normals of the mesh.
    std::vector<Vertex> vertices;
    // Triangles are the indices of the vertices involved in a triangle.
    // A triangle, thus, contains a triplet of values corresponding to the 3 vertices of a triangle.
    std::vector<Triangle> triangles;

    std::vector<VertexTriangle> vertexTriangles;

    Material material;
    AudioReflection *audioReflection;
};

[[nodiscard]] std::vector<Mesh> loadMesh(const std::string& file);

