#ifndef APP_HPP
#define APP_HPP

#include <glm/glm.hpp>
#include <vector>

struct AppContext {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model;

    int width;
    int height;

    std::vector<glm::vec3>* vertices;
    std::vector<unsigned int>* indices;

    std::vector<glm::vec3> linePoints;
};

#endif