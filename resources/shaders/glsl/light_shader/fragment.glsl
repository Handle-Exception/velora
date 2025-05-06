#version 450

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

struct GPULight {
    vec4 position;      // xyz: position, w: unused
    vec4 direction;     // xyz: direction, w: type
    vec4 color;         // rgb: color, w: intensity
    vec4 attenuation;   // x: constant, y: linear, z: quadratic, w: unused
    vec2 cutoff;        // x: inner cos, y: outer cos
    uint castShadows;
    uint pad0;
    uint pad1;
    uint pad2;
};

layout(std430, binding = 2) buffer LightBuffer {
    GPULight lights[];
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec4 uColor;
uniform sampler2D uTexture;
uniform bool useTexture;

void main()
{
    vec4 baseColor = useTexture ? texture(uTexture, TexCoord) : uColor;
    FragColor = baseColor;
}