// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"


glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}

// Initial position : on +Z
glm::vec3 position = glm::vec3( 0, 1, 3 ); 
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 60.0f;

float speed = 0.05f; // 3 units / second
float mouseSpeed = 0.0005f;

float FoV = initialFoV; 

// Before starting we need a way to define the scroll wheel inputs
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // yoffset = vertical scroll amount
    FoV -= 5.0f * (float)yoffset;    
}


void computeMatricesFromInputs(){

    // Get the current window size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    glfwSetCursorPos(window, width/2, height/2);
    
    horizontalAngle += mouseSpeed  * float(width/2 - xpos );
    verticalAngle   += mouseSpeed  * float( height/2 - ypos );

    vec3 direction(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle)
    );

    vec3 right = vec3(
        sin(horizontalAngle - 3.14f/2.0f),
        0,
        cos(horizontalAngle - 3.14f/2.0f)
    );

    vec3 up = cross(right, direction );

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        position += direction * speed;
    }
    // Move backward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        position -= direction * speed;
    }
    // Strafe right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        position += right *  speed;
    }
    // Strafe left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        position -= right * speed;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        position += up * speed;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        position -= up * speed;
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        position = glm::vec3(0, 1, 3);
        horizontalAngle = 3.14f;
        verticalAngle = 0.0f;
        initialFoV = 60.0f;
    }


    ProjectionMatrix = perspective(glm::radians(FoV), (float)width / (float)height, 0.1f, 100.0f);
    // Camera matrix
    ViewMatrix       = lookAt(
        position,           // Camera is here
        position+direction, // and looks here : at the same position, plus "direction"
        up                  // Head is up (set to 0,-1,0 to look upside-down)
    );

}
