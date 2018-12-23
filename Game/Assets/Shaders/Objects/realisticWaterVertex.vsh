#version 330 core

#define PI 3.14285714286

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;
layout (location = 3) in vec2 texCoord;
layout (location = 4) in vec3 tangent;
layout (location = 5) in vec3 bitangent;

uniform mat4 model_matrix;
uniform mat4 mvp_matrix;

out vec3 fPosition;
out vec3 fNormal;
out vec4 fColor;
out vec2 fTexCoord;
out mat3 TBN;
out vec3 startPos;

struct Ripple 
{
float speed;
float amplitude;
float frequency;
};

uniform float Time;
uniform Ripple ripple;

void main()
{            
vec3 T = normalize(vec3(model_matrix * vec4(tangent,   0.0)));
vec3 B = normalize(vec3(model_matrix * vec4(bitangent, 0.0)));
vec3 N = normalize(vec3(model_matrix * vec4(normal,    0.0)));
TBN = mat3(T, B, N);

vec3 pos = position;

// Ripple
float distance = length(position);
pos.y += sin((Time * ripple.speed) + (2.0 * PI * distance * ripple.frequency)) * ripple.amplitude;
    
fPosition = vec3(model_matrix * vec4(pos, 1.0));
fNormal = normal;
fColor = color;
fTexCoord = texCoord;
startPos = vec3(model_matrix * vec4(position, 1.0));

gl_Position = mvp_matrix * vec4(pos, 1.0);
}