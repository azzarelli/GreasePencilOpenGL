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

    protected:
        std::vector<glm::vec3> vertices;
        std::vector<unsigned int> indices;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;

    private:
        const aiScene* scene;

        GLuint vertexbuffer;
        GLuint uvbuffer;
        GLuint normalbuffer;
        GLuint elementbuffer;

        GLuint Texture;
        GLuint TextureID;

        GLuint VertexArrayID;

};

class AABB{
    public:
        AABB(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);
        AABB(const std::vector<glm::vec3>& verts, std::vector<unsigned int> indices);
        AABB(const std::vector<glm::vec3>& points);
        
        glm::vec3 bmin, bmax;
        glm::vec3 centroid;

        int longest_axis();    
};

bool rayAABB_slab(const glm::vec3& ro, const glm::vec3& rd, const AABB& aabb);

class BVHMesh : public BaseMesh{
    public:
        void buildBVH(uint32_t leafSize);

        struct Hit { float t; uint32_t triId; };

        void recursiveHitProgram(uint32_t NodeIdx, const glm::vec3& ro, const glm::vec3& rd);

        std::vector<glm::vec3> debugLines;

        std::vector<glm::vec3> drawPoints;
        std::vector<std::array<unsigned int, 3>> drawTriIndices;

        void loadNewDrawPoint(glm::mat4 InvMVP, glm::mat4 InvModelMatrix) ;
            
    private:
        struct Node{
            AABB box;
            uint32_t ChildIndex = 0;
            std::vector<unsigned int> indices;
        };

        std::vector<uint32_t> tempNodeBuffer;


        void splitNode(uint32_t ParentIdx);

        bool rayNodeIntersection(uint32_t NodeIdx, const glm::vec3& ro, const glm::vec3& rd);

        bool rayNodeClosestHit(uint32_t NodeIdx,const glm::vec3& ro,const glm::vec3& rd,float& outT,uint32_t& outTriBase );
        void surfaceTraversal();

    protected:
        uint32_t leafMaxSize;

        std::vector<Node> nodes;
        std::vector<uint32_t> triangleOrder;



};