#include <vector>
#include "rayTracer.hpp"

Ray makeRayFromMouse(double mouseX, double mouseY,
                     const glm::mat4& proj,
                     const glm::mat4& view,
                     const glm::mat4& model,
                     int width, int height){
    // Screen -> NDC
    float x = (2.0f * float(mouseX)) / float(width)  - 1.0f;
    float y = 1.0f - (2.0f * float(mouseY)) / float(height); // flip Y
    glm::vec4 rayClipNear(x, y, -1.0f, 1.0f);
    glm::vec4 rayClipFar (x, y,  1.0f, 1.0f);

    glm::mat4 inv = glm::inverse(proj * view * model);

    glm::vec4 rayWorldNear = inv * rayClipNear;
    glm::vec4 rayWorldFar  = inv * rayClipFar;
    rayWorldNear /= rayWorldNear.w;
    rayWorldFar  /= rayWorldFar.w;

    Ray r;
    r.origin = glm::vec3(rayWorldNear);
    r.dir    = glm::normalize(glm::vec3(rayWorldFar - rayWorldNear));
    return r;
}

// Moller-Trumbore trai-tri intersection
bool rayTriIntersect(const glm::vec3& orig, const glm::vec3& dir,
                     const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                     float& tOut)
{
    const float EPS = 1e-7f;
    glm::vec3 e1 = v1 - v0;
    glm::vec3 e2 = v2 - v0;

    glm::vec3 p = glm::cross(dir, e2);
    float det = glm::dot(e1, p);

    if (fabs(det) < EPS) return false; // parallel

    float invDet = 1.0f / det;
    glm::vec3 s = orig - v0;
    float u = invDet * glm::dot(s, p);
    if (u < 0.0f || u > 1.0f) return false;

    glm::vec3 q = glm::cross(s, e1);
    float v = invDet * glm::dot(dir, q);
    if (v < 0.0f || (u + v) > 1.0f) return false;

    float t = invDet * glm::dot(e2, q);
    if (t < EPS) return false; // behind origin

    tOut = t;
    return true;
}


bool pickOnMesh(const Ray& ray,
                const std::vector<glm::vec3>& vertices,
                const std::vector<unsigned int>& indices,
                glm::vec3& hitPointOut)
{
    bool hit = false;
    float bestT = std::numeric_limits<float>::max();

    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        const glm::vec3& v0 = vertices[indices[i + 0]];
        const glm::vec3& v1 = vertices[indices[i + 1]];
        const glm::vec3& v2 = vertices[indices[i + 2]];

        float t;
        if (rayTriIntersect(ray.origin, ray.dir, v0, v1, v2, t)) {
            if (t < bestT) {
                bestT = t;
                hit = true;
            }
        }
    }

    if (hit) {
        hitPointOut = ray.origin + bestT * ray.dir;
    }
    return hit;
}
