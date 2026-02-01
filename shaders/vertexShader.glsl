#version 330 core
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
out vec3 Position_worldspace;
out vec3 Normal_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 LightDirection_cameraspace;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform vec3 LightPosition_worldspace;

void main(){
    // Output positions in "clip space"
    gl_Position =  MVP * vec4(vertexPosition_modelspace,1);

    // Vertex worl position
    Position_worldspace = (M * vec4(vertexPosition_modelspace,1)).xyz;

    // Vertex in cameraspace: transform the model space to cam space
    vec3 vertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelspace,1)).xyz;
    // Unnormalized direction
    EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace; // camera is at the origin

    // Get the light position from world space
    vec3 LightPosition_cameraspace = ( V * vec4(LightPosition_worldspace, 1)).xyz;
    LightDirection_cameraspace = LightPosition_cameraspace - vertexPosition_cameraspace;
    mat3 normalMatrix = mat3(V * M); //transpose(inverse(mat3(V * M)));
    Normal_cameraspace = normalMatrix* vec4(vertexNormal_modelspace,0).xyz; // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.

    
    // Pass UV vertices to the fragment shader
    UV = vertexUV;
}