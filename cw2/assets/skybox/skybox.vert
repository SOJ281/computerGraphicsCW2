#version 430
layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec3 iTexCoords;


out vec3 v2fTexCoords;

uniform mat4 uProjCameraWorld;

void main()
{
	v2fTexCoords = iTexCoords;
	vec4 pos = uProjCameraWorld * vec4(iPosition, 1.0);
	gl_Position = vec4(pos.xy, pos.ww);
}