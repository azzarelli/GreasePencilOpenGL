#version 330 core

in vec2 UV;
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;

out vec4 color;

// Values that stay constant for the whole mesh
uniform sampler2D myTextureSampler;
uniform mat4 MV;
uniform vec3 LightPosition_worldspace;

void main(){
  vec3 LightColor = vec3(1.0f,1.0f,1.0f);
  float LightPower = 50.0f;

  vec3 MaterialColor = texture(myTextureSampler, UV).rgb;
	vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);

  // distance from lightposition and world space position
  float distance = length( LightPosition_worldspace - Position_worldspace );

  // get normalized normal direction (I would have assumed most models pre-normalized these params but I guess this is safe)
  vec3 n = normalize( Normal_cameraspace );
	vec3 l = normalize( LightDirection_cameraspace );


  float cosTheta = clamp(dot(n, l), 0, 1);
  vec3 MaterialDiffuseColor = 
		MaterialColor*LightPower*LightColor * cosTheta / (distance*distance);


  // Ambient Light, Small increase in material color so its not purely black
  vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * MaterialColor;

  // Specular component increases based on parallelity between ray direction and view
  vec3 EyeN = normalize(EyeDirection_cameraspace);
  vec3 Refl = reflect(-l, n); // reflect -l around normal n. -l gives us a direction from light to surface


  float cosSpec = clamp (dot(EyeN, Refl), 0, 1);
  MaterialSpecularColor = MaterialSpecularColor * LightPower * pow(cosSpec, 2) / (distance * distance);

  vec3 DebugColor = vec3(1.0f,0.0f, 0.0f);


	color.rgb = 
  //DebugColor ;
    MaterialAmbientColor + MaterialDiffuseColor + MaterialSpecularColor;
  
  color.a = 1.0;
}