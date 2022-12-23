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


uniform vec3 center;
uniform vec3 viewPos;
uniform vec3 direction;



out vec3 v2fNormal;
out vec2 v2fTexCoords;
out vec3 fragPos;

mat4 rotationMatrix(vec3 axis, float angle);

void main() {



	vec3 tPosition = iPosition;


    v2fTexCoords = iTexCoords;



    //vec3 from_vector = top - bottom;
    vec3 from_vector = normalize(vec3(center.x+1, center.y, center.z));
    //vec3 from_vector = normalize(center - viewPos);
    vec3 to_vector = normalize(viewPos - center);

    float cosa = dot(from_vector,to_vector);
    clamp(cosa, -1.f, 1.f);
    vec3 axis = cross(from_vector,to_vector); 
    float angle = acos(cosa);
    //mat4 rotate_matrix = rotate(mat4(1),angle,axis);
    //mat4 rotate_matrix = make_rotation_x( angle ) * vec4( axis, 1.0 );
    mat4 rotate_matrix = rotationMatrix(axis, angle);

    tPosition = vec3(vec4( tPosition - center, 1.0 ) * rotate_matrix) + center;

    gl_Position = uProjCameraWorld * (vec4( tPosition, 1.0 ));


    v2fNormal = iNormal;
    fragPos = iPosition;

}

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}