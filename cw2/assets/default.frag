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


struct SpotLight {
    vec3 position;
    vec3 direction;
    float phi;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform SpotLight spotLights[1];


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


vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

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
    //result += uSpecular * spec * ( material.specular + vec3(texture(textures.specular, v2fTexCoords)));//specular
    result += uSpecular * spec * ( material.specular + vec3(texture(textures.specular, v2fTexCoords)));//specular
    result += material.emissive;//emissive

    // phase 2: point lights
    for(int i = 0; i < 6; i++)
        result += calcPointLight(pointLights[i], normal, fragPos, viewDir); 
    result += calcSpotLight(spotLights[0], normal, fragPos, viewDir);  

    oColor = vec4(result, material.opacity);
}



// calculates the color when using a point light.
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    //float spec = max(dot(viewDir, reflectDir), 0.0);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 100);
    //float spec = max(dot(viewDir, reflectDir), 0.0) * material.shininess;

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // combine results
    vec3 ambient = light.ambient * (material.ambient + vec3(texture(textures.diffuse, v2fTexCoords)));
    vec3 diffuse = light.diffuse * diff * (material.diffuse + vec3(texture(textures.diffuse, v2fTexCoords)));
    vec3 specular = light.specular * spec * (material.specular + vec3(texture(textures.specular, v2fTexCoords)));
    ambient *= attenuation;
    diffuse *= attenuation;
    spec *= attenuation;
    return (ambient + diffuse + spec);
}


// calculates the color when using a spot light.
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    float theta = dot(lightDir, normalize(-light.direction));
    if (theta <= light.phi) 
        return vec3(0,0,0);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    //float spec = max(dot(viewDir, reflectDir), 0.0);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 100);
    //float spec = max(dot(viewDir, reflectDir), 0.0) * material.shininess;

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // combine results
    vec3 ambient = light.ambient * (material.ambient + vec3(texture(textures.diffuse, v2fTexCoords)));
    vec3 diffuse = light.diffuse * diff * (material.diffuse + vec3(texture(textures.diffuse, v2fTexCoords)));
    vec3 specular = light.specular * spec * (material.specular + vec3(texture(textures.specular, v2fTexCoords)));
    ambient *= attenuation;
    diffuse *= attenuation;
    spec *= attenuation;
    return (ambient + diffuse + spec);
}
