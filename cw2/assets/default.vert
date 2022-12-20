#version 430

// Input data
// The layout( location = N ) syntax allows us to specify attribute indices directly in the shader. This
// avoids having to call glBindAttribLocation() when creating the shader program object. See
// https://www.khronos.org/opengl/wiki/Layout Qualifier (GLSL)5G// for more information.
// Note: the indices that we specify here must match the ones that we set up in the vertex array object.
layout( location = 0 ) in vec3 iPosition;
layout( location = 1 ) in vec3 iNormal;
layout( location = 2 ) in vec2 iTexCoords;

uniform mat4 uProjCameraWorld;
uniform mat3 uNormalMatrix;
uniform mat4 rotation;

uniform mat4 rotateDoor;
uniform vec3 point;

//layout( location = 5 ) uniform vec3 uLightPos;

out vec3 v2fNormal;
out vec2 v2fTexCoords;
//out float lightDistance;
out vec3 fragPos;


vec3 translate(vec3 transV, vec3 vertex);
vec3 rotateByPoint(mat4 rotV, vec3 pointBy, vec3 vertex);
vec3 rotate(mat4 rotV, vec3 vertex);
vec3 scaling(mat4 scalV, vec3 vertex);

void main() {

    //vec4 t = rotation * vec4( iPosition - point, 1.0 );
	//vec3 tPosition = vec3( t.x, t.y, t.z ) + point;
	vec3 tPosition = rotateByPoint(rotateDoor, point, iPosition);

    vec3 tNormal = rotate(rotateDoor, iNormal);


    v2fTexCoords = iTexCoords;


    gl_Position = uProjCameraWorld * vec4( tPosition, 1.0 );
    v2fNormal = normalize(uNormalMatrix * iNormal);


    fragPos = iPosition;

    //lightDistance = length(uLightPos - iPosition);
}


//Mapping simple transformations so I don't have to use brain
vec3 translate(vec3 transV, vec3 vertex) {
	return transV + vertex;
}

vec3 rotateByPoint(mat4 rotV, vec3 pointBy, vec3 vertex) {
    vec4 t = rotation * vec4( iPosition - pointBy, 1.0 );
	return vec3( t.x, t.y, t.z ) + pointBy;
}

vec3 rotate(mat4 rotV, vec3 vertex) {
    vec4 t = rotation * vec4( iPosition, 1.0 );
	return vec3( t.x, t.y, t.z );
}

vec3 scaling(mat4 scalV, vec3 vertex) {
    vec4 t = rotation * vec4( iPosition, 1.0 );
    return vec3( t.x, t.y, t.z );
}