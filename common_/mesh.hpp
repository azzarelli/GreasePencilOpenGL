#pragma once
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <GL/glew.h>
#include <glm/glm.hpp>

class BaseMesh{
    public:
        BaseMesh();
        void load(std::string filePath, const char* filePathDSS, GLuint programID);
        void step();
        void del();

    private:
    const aiScene* scene;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
    
    GLuint vertexbuffer;
    GLuint uvbuffer;
    GLuint normalbuffer;
    GLuint elementbuffer;

    GLuint Texture;
    GLuint TextureID;

    GLuint VertexArrayID;

};