#version 430

// Input data
// The layout( location = N ) syntax allows us to specify attribute indices directly in the shader. This
// avoids having to call glBindAttribLocation() when creating the shader program object. See
// https://www.khronos.org/opengl/wiki/Layout Qualifier (GLSL)5G// for more information.
// Note: the indices that we specify here must match the ones that we set up in the vertex array object.
layout( location = 0 ) in vec3 iPosition;
layout( location = 1 ) in vec3 iColor;
layout( location = 2 ) in vec3 iNormal;

uniform mat4 uProjCameraWorld;
uniform mat3 uNormalMatrix;
uniform mat4 transformation;

//layout( location = 5 ) uniform vec3 uLightPos;

out vec3 v2fColor; // v2f = vertex to fragment
out vec3 v2fNormal;
//out float lightDistance;
out vec3 fragPos;
void main() {
	vec4 t = transformation * vec4( iPosition, 1.0 );
	t /= t.w;
	vec3 tPosition = vec3( t.x, t.y, t.z );
    //vec3 tPosition = iPosition * transformation;

    v2fColor = iColor;

    gl_Position = uProjCameraWorld * vec4( iPosition, 1.0 );
    v2fNormal = normalize(uNormalMatrix * iNormal);


    fragPos = iPosition;

    //lightDistance = length(uLightPos - iPosition);
}
