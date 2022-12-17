#version 430
// Input attributes
// These should match the outputs from the vertex shader.

in vec3 v2fColor;
in vec3 v2fNormal;
//in float lightDistance;
in vec3 fragPos;
in vec2 v2fTexCoords;



layout( location = 0 ) out vec3 oColor;

//uniform Light light;
uniform mat4 uProjCameraWorld;
uniform mat3 uNormalMatrix;
uniform vec3 viewPos;

uniform vec3 uLightDir;
uniform vec3 uLightDiffuse;
uniform vec3 uSceneAmbient;
uniform vec3 uSpecular;



struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform PointLight pointLights[6];


struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;    
    float shininess;
}; 
uniform Material material;

struct Textures {
    sampler2D diffuse;
    sampler2D specular;
}; 
uniform Textures textures;


vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
    vec3 normal = normalize(v2fNormal);
    vec3 viewDir = normalize(viewPos - fragPos);

    float nDotL = max( 0.0, dot( normal, uLightDir ) );


    vec3 reflectDir = reflect(-uLightDir, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = uSpecular * (spec * material.specular); 


    //vec3 result = (uSceneAmbient*material.ambient + nDotL * uLightDiffuse * material.diffuse) * v2fColor;
    vec3 result = uSceneAmbient * (material.ambient + vec3(texture(textures.diffuse, v2fTexCoords)));//Ambient
    result += nDotL * uLightDiffuse * (material.diffuse + vec3(texture(textures.diffuse, v2fTexCoords)));//diffuse
    result += specular * (spec * material.specular + vec3(texture(textures.specular, v2fTexCoords)));//specular
    result *= v2fColor;

    // phase 2: point lights
    for(int i = 0; i < 6; i++)
        result += CalcPointLight(pointLights[i], normal, fragPos, viewDir);    

    oColor = result;
}



// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = max(dot(viewDir, reflectDir), 0.0);

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // combine results
    vec3 ambient = light.ambient*material.ambient * vec3(texture(textures.diffuse, v2fTexCoords));
    vec3 diffuse = light.diffuse * diff * material.diffuse * vec3(texture(textures.diffuse, v2fTexCoords));
    vec3 specular = light.specular * spec * material.specular * vec3(texture(textures.specular, v2fTexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    spec *= attenuation;
    return (ambient + diffuse + spec);
}

/*
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
    vec3 normal = normalize(v2fNormal);
    oColor = normal;
    vec3 viewDir = normalize(viewPos - fragPos);

    float nDotL = max( 0.0, dot( normal, uLightDir ) );


    vec3 reflectDir = reflect(-uLightDir, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = uSpecular * (spec * material.specular); 

    //vec3 result = (uSceneAmbient*material.ambient + nDotL * uLightDiffuse * material.diffuse) * v2fColor;
    vec3 result = (uSceneAmbient*material.ambient + nDotL * uLightDiffuse * material.diffuse + specular) * v2fColor;

    // phase 2: point lights
    for(int i = 0; i < 6; i++)
        result += CalcPointLight(pointLights[i], normal, fragPos, viewDir);    

    oColor = result;
}



// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = max(dot(viewDir, reflectDir), 0.0);

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // combine results
    vec3 ambient = light.ambient*material.ambient;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    ambient *= attenuation;
    diffuse *= attenuation;
    spec *= attenuation;
    return (ambient + diffuse + spec);
}*/