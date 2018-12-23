#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;
layout (location = 3) in vec2 texCoord;

uniform mat4 model_matrix;
uniform mat4 mvp_matrix;
uniform mat3 normal_matrix;

out vec3 fPosition;
out vec3 fNormal;
out vec4 fColor;
out vec2 fTexCoord;
out vec3 startPos;

struct Wave 
{
float speed;
float amplitude;
float frequency;
};

uniform float Time;
uniform Wave wave;

void main()
{    
vec3 pos = position;
fTexCoord = texCoord;

// Wave
pos.y += sin((Time * wave.speed) + (pos.x * pos.z * wave.frequency)) * wave.amplitude;
fTexCoord.xy += sin((Time * 0.2) + (pos.x * pos.z * 0.0001)) * 0.2;
    
fPosition = vec3(model_matrix * vec4(pos, 1.0));
fNormal = normalize(normal_matrix * normal);
fColor = color;
startPos = vec3(model_matrix * vec4(position, 1.0));

gl_Position = mvp_matrix * vec4(pos, 1.0);
}