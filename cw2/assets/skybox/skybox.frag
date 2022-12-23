#version 430
out vec4 oColor;

in vec3 v2fTexCoords;

uniform samplerCube skybox;

void main()
{
	oColor = texture(skybox, v2fTexCoords);
}