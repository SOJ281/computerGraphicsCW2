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

uniform vec3 uLightDir;
uniform vec3 uLightDiffuse;
uniform vec3 uSceneAmbient;
uniform vec3 uSpecular;



struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;   
    vec3 emissive;
    float shininess;
    float opacity;
}; 
uniform Material material;

struct Textures {
    sampler2D diffuse;
    sampler2D specular;
}; 
uniform Textures textures;
uniform int textureCount;


//vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
    vec3 normal = normalize(v2fNormal);
    vec3 viewDir = normalize(viewPos - fragPos);

    float nDotL = max( 0.0, dot(normal, uLightDir) );


    vec3 reflectDir = reflect(-uLightDir, normal); 
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 100);
    //vec3 specular = uSpecular * (spec * material.specular); 


    //vec3 result = (uSceneAmbient*material.ambient + nDotL * uLightDiffuse * material.diffuse) * v2fColor;
    vec3 result = uSceneAmbient * (material.ambient + vec3(texture(textures.diffuse, v2fTexCoords)));//Ambient
    result += nDotL * uLightDiffuse * (material.diffuse + vec3(texture(textures.diffuse, v2fTexCoords)));//diffuse
    //result += specular * spec * ( material.specular + vec3(texture(textures.specular, v2fTexCoords)));//specular
    result += uSpecular * spec * ( material.specular + vec3(texture(textures.specular, v2fTexCoords)));//specular
    result += material.emissive;//emissive
 

    oColor = vec4(result, material.opacity);
}
