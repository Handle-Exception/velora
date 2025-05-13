#version 450

// Lights
#define LIGHT_TYPE_DIRECTIONAL 1
#define LIGHT_TYPE_POINT       2
#define LIGHT_TYPE_SPOT        3

struct GPULight {
    vec4 position;      // xyz: position (or direction for directional), w: unused
    vec4 direction;     // xyz: direction, w: type
    vec4 color;         // rgb: color, w: intensity
    vec4 attenuation;   // x: constant, y: linear, z: quadratic
    vec2 cutoff;        // x: inner cos, y: outer cos
    vec2 castShadows;   // unused here
};

//struct ShadowCaster
//{
//    mat4 lightSpaceMatrix;
//};
//sampler2DShadow shadowMap[254];


layout(std430, binding = 2) buffer LightBuffer {
    GPULight lights[];
};
uniform int lightCount;

// Camera
uniform vec3 viewPos;

// G-buffer inputs
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

// Shadow Map
//uniform sampler2DShadow shadowMap;
//uniform mat4 lightSpaceMatrix;

in vec2 TexCoord;
out vec4 FragColor;

//float calculateShadow(vec3 fragPosWorld)
//{
//    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragPosWorld, 1.0);
//    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
//    projCoords = projCoords * 0.5 + 0.5;
//   return texture(shadowMap, projCoords); // 0 = in shadow, 1 = lit
//}


vec3 calculateLight(GPULight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir;
    float attenuation = 1.0;
    float diff = 0.0;

    int type = int(light.direction.w);
    if (type == LIGHT_TYPE_DIRECTIONAL) {
        lightDir = normalize(-light.direction.xyz);
    } else {
        vec3 delta = light.position.xyz - fragPos;
        lightDir = normalize(delta);
        float dist = length(delta);
        attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * dist + light.attenuation.z * dist * dist);

        if (type == LIGHT_TYPE_SPOT) {
            float theta = dot(lightDir, normalize(-light.direction.xyz));
            float epsilon = light.cutoff.x - light.cutoff.y;
            float intensity = clamp((theta - light.cutoff.y) / epsilon, 0.0, 1.0);
            attenuation *= intensity;
        }
    }

    diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.color.rgb * light.color.w * diff * attenuation;
    return diffuse;
}

void main()
{
    vec3 FragPos = texture(gPosition, TexCoord).rgb;
    vec3 Normal  = normalize(texture(gNormal, TexCoord).rgb);
    vec4 Albedo  = texture(gAlbedoSpec, TexCoord);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 lighting = vec3(0.0);
    for (int i = 0; i < lightCount; ++i)
        lighting += calculateLight(lights[i], Normal, FragPos, viewDir);

    //float shadow = calculateShadow(fragPos);
    //lighting *= shadow;

    FragColor = vec4(lighting * Albedo.rgb, Albedo.a);
}
