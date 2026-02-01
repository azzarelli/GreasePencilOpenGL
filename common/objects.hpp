#ifndef OBJECTS_HPP
#define OBJECTS_HPP

#include <glm/glm.hpp>
#include <stdio.h>
#include <assimp/scene.h>   // <-- REQUIRED

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};
struct aiNode;
struct aiScene;


// ---- function declarations ----
void processNode(aiNode* node, const aiScene* scene);
void processMesh(aiMesh* mesh,
                 std::vector<Vertex>& vertices,
                 std::vector<unsigned int>& indices);


#endif