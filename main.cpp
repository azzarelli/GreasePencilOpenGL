#include <stdio.h>
#include <vector>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
using namespace glm;
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "common/shader.hpp"
#include "common/controls.hpp"
#include "common/mesh.hpp"


GLFWwindow* window = nullptr;

int main(){

    // Init GLFW
    glewExperimental = true;
    if (!glfwInit()){
        fprintf(stderr, "Faild to init GLFW\n");
        return -1;
    }
    
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 

    int width = 1024;
    int height = 768;

    window = glfwCreateWindow(width, height, "BVH", NULL, NULL);

    if (window == NULL){
        fprintf(stderr, "Failed to open GLFW ...");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental=true;
    if (glewInit() != GLEW_OK){
        fprintf(stderr, "Failer to init GLEW");
        return -1;
    }

    glfwSetScrollCallback(window, scroll_callback);

    // To stop window from autoclosing - lol
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);


    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // Accept closer fragments
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable cursor and register mousebutton callback
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Compile Shaders
    GLuint programID = LoadShaders( "shaders/vertexShader.glsl", "shaders/fragmentShader.glsl" );


    mat4 ModelMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f)); // At the origin w/ identity
    vec3 lightPos = vec3(-4.0f, 4.0f, 4.0f);

    // Hand over to shaders
    GLuint MatrixID = glGetUniformLocation(programID, "MVP"); // Init full cam transform
    GLuint ViewID = glGetUniformLocation(programID, "V"); // Init view transform 
    GLuint ModelID = glGetUniformLocation(programID, "M"); // Init model transform 
    GLuint ModelViewID = glGetUniformLocation(programID, "MV"); // Init model transform 
    GLuint LightPosID = glGetUniformLocation(programID, "LightPosition_worldspace"); // Init model transform 


    BVHMesh basemesh;
    basemesh.load("../assets/Person.obj","../assets/uvmap.DDS", programID);
    basemesh.buildBVH((uint32_t) 4);


    GLuint lineVAO=0, lineVBO=0;
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);

    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);


    // Define light position
    do{
        // Clear scene to avoid flickering
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.1f, 1.0f);

        computeMatricesFromInputs();
        mat4 ProjectionMatrix = getProjectionMatrix();
        mat4 ViewMatrix = getViewMatrix();

        mat4 mvp = ProjectionMatrix * ViewMatrix * ModelMatrix;
        mat4 mv = ViewMatrix * ModelMatrix;

        glUseProgram(programID);
        // send transform to the shader, doesnt mater where its at so long as it is instantiated before the end of the loop
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
        glUniformMatrix4fv(ViewID, 1, GL_FALSE, &ViewMatrix[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
        glUniformMatrix4fv(ModelViewID, 1, GL_FALSE, &mv[0][0]);
        glUniform3fv(LightPosID, 1, &lightPos[0]);
        
        basemesh.step();
        
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
    }
    while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) ==0);

	// Cleanup VBO
	glDeleteProgram(programID);
    basemesh.del();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;

}