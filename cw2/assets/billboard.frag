#version 430
// Input attributes
// These should match the outputs from the vertex shader.

in vec3 v2fNormal;
//in float lightDistance;
in vec3 fragPos;
in vec2 v2fTexCoords;



layout( location = 0 ) out vec4 oColor;

//uniform Light light;
uniform mat4 uProjCameraWorld;
uniform mat3 uNormalMatrix;
uniform vec3 viewPos;


uniform sampler2D sprite;

void main() {


    oColor = texture(sprite, v2fTexCoords);
}
