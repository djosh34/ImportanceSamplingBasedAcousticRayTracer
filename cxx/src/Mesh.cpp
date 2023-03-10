#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/mat4x4.hpp>
#include <iostream>
#include <stack>

// Delft University of technology


static glm::mat4 assimpMatrix(const aiMatrix4x4& m)
{
    //float values[3][4] = {};
    glm::mat4 matrix;
    matrix[0][0] = m.a1;
    matrix[0][1] = m.b1;
    matrix[0][2] = m.c1;
    matrix[0][3] = m.d1;
    matrix[1][0] = m.a2;
    matrix[1][1] = m.b2;
    matrix[1][2] = m.c2;
    matrix[1][3] = m.d2;
    matrix[2][0] = m.a3;
    matrix[2][1] = m.b3;
    matrix[2][2] = m.c3;
    matrix[2][3] = m.d3;
    matrix[3][0] = m.a4;
    matrix[3][1] = m.b4;
    matrix[3][2] = m.c4;
    matrix[3][3] = m.d4;
    return matrix;
}

static glm::vec3 assimpVec(const aiVector3D& v)
{
    return glm::vec3(v.x, v.y, v.z);
}

static glm::vec3 assimpVec(const aiColor3D& c)
{
    return glm::vec3(c.r, c.g, c.b);
}

std::vector<Mesh> loadMesh(const std::string& file)
{

    Assimp::Importer importer;
    const aiScene* pAssimpScene = importer.ReadFile(file.c_str(), aiProcess_GenNormals | aiProcess_Triangulate);

    if (pAssimpScene == nullptr || pAssimpScene->mRootNode == nullptr || pAssimpScene->mFlags == AI_SCENE_FLAGS_INCOMPLETE) {
        std::cerr << "Assimp failed to load mesh file " << file << std::endl;
        throw std::exception();
    }

    std::vector<Mesh> out;

    std::stack<std::tuple<aiNode*, glm::mat4>> stack;
    stack.push({ pAssimpScene->mRootNode, assimpMatrix(pAssimpScene->mRootNode->mTransformation) });
    while (!stack.empty()) {
        auto [node, matrix] = stack.top();
        stack.pop();

        matrix *= assimpMatrix(node->mTransformation);
        const glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(matrix));

        for (unsigned i = 0; i < node->mNumMeshes; i++) {
            // Process sub mesh.
            const aiMesh* pAssimpMesh = pAssimpScene->mMeshes[node->mMeshes[i]];

            if (pAssimpMesh->mNumVertices == 0 || pAssimpMesh->mNumFaces == 0)
                std::cerr << "Empty mesh encountered" << std::endl;

            // Process triangles in sub mesh.
            Mesh mesh;
            for (unsigned j = 0; j < pAssimpMesh->mNumFaces; j++) {
                const aiFace& face = pAssimpMesh->mFaces[j];
                if (face.mNumIndices != 3) {
                    std::cerr << "Found a face which is not a triangle, discarding!" << std::endl;
                    continue;
                }

                const auto aiIndices = face.mIndices;
                mesh.triangles.emplace_back(aiIndices[0], aiIndices[1], aiIndices[2]);
            }

            // Process vertices in sub mesh.
            for (unsigned j = 0; j < pAssimpMesh->mNumVertices; j++) {
                const glm::vec3 pos = matrix * glm::vec4(assimpVec(pAssimpMesh->mVertices[j]), 1.0f);
                const glm::vec3 normal = normalMatrix * assimpVec(pAssimpMesh->mNormals[j]);
                mesh.vertices.push_back(Vertex { pos, normal });
            }

            const aiMaterial* pAssimpMaterial = pAssimpScene->mMaterials[pAssimpMesh->mMaterialIndex];
            auto getMaterialColor = [&](const char* pKey, unsigned type, unsigned idx) {
                aiColor3D color(0.f, 0.f, 0.f);
                pAssimpMaterial->Get(pKey, type, idx, color);
                return assimpVec(color);
            };

            mesh.material.kd = getMaterialColor(AI_MATKEY_COLOR_DIFFUSE);
            out.emplace_back(std::move(mesh));
        }

        for (unsigned i = 0; i < node->mNumChildren; i++) {
            stack.push({ node->mChildren[i], matrix });
        }
    }
    importer.FreeScene();

    return out;
}

std::ostream &operator<<(std::ostream &out, const VertexTriangle &vertexTriangle) {
    out << "["
        << vertexTriangle.vertex_0.p << " "
        << vertexTriangle.vertex_1.p << " "
        << vertexTriangle.vertex_2.p
        << "]";

    return out;
}

AudioReflection::AudioReflection(Energy &SCATTERING_COEFFICIENT, Energy &ABSORPTION_COEFFICIENT) {
    for (int band = 0; band < N_BANDS; band++) {
        this->scattering_coefficient.values[band] = SCATTERING_COEFFICIENT.values[band];
        this->absorption_coefficient.values[band] = ABSORPTION_COEFFICIENT.values[band];
    }
}
