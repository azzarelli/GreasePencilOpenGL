#ifndef CONTROLS_HPP
#define CONTROLS_HPP

#include <GLFW/glfw3.h>
void computeMatricesFromInputs();
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

extern bool flag_loadNewPoint2Draw;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void char_callback(GLFWwindow* window, unsigned int c);
#endif