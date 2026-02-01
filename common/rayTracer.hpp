#ifndef RT_HPP
#define RT_HPP

#include <glm/glm.hpp>
#include <vector>

struct Ray {
    glm::vec3 origin;
    glm::vec3 dir; // normalized
};

Ray makeRayFromMouse(double mouseX, double mouseY,
                     const glm::mat4& proj,
                     const glm::mat4& view,
                     const glm::mat4& model,
                     int width, int height);

bool rayTriIntersect(const glm::vec3& orig, const glm::vec3& dir,
                     const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                     float& tOut);

                     
bool pickOnMesh(const Ray& ray,
                const std::vector<glm::vec3>& vertices,
                const std::vector<unsigned int>& indices,
                glm::vec3& hitPointOut);

#endif