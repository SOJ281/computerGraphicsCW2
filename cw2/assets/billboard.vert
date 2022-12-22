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
uniform mat4 world2camera;//viewMatrix
uniform mat4 projection;//projectionMatrix

uniform vec3 point;



out vec3 v2fNormal;
out vec2 v2fTexCoords;
out vec3 fragPos;


void main() {



	vec3 tPosition = iPosition;


    v2fTexCoords = iTexCoords;

    vec3 CameraRight_worldspace = vec3(world2camera[0][0], world2camera[1][0], world2camera[2][0]);
    vec3 CameraUp_worldspace = vec3(world2camera[0][1], world2camera[1][1], world2camera[2][1]);


    //gl_Position = uProjCameraWorld * vec4( tPosition, 1.0 );
    
    gl_Position =  uProjCameraWorld * vec4( vec3(0,2,0)
    + CameraRight_worldspace * tPosition.x 
    + CameraUp_worldspace * tPosition.y , 1.0 );
    //gl_Position /= gl_Position.w;
    //gl_Position.xy += tPosition.xy * vec2(0.2, 0.05);



    v2fNormal = normalize(uNormalMatrix * iNormal);
    fragPos = iPosition;

}
