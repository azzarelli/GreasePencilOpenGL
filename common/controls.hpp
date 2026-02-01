#ifndef CONTROLS_HPP
#define CONTROLS_HPP

#include <GLFW/glfw3.h>
void computeMatricesFromInputs();
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

#endif