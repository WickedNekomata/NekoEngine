#version 330 core
layout (location = 0) in vec2 vertex; // <vec2 position, vec2 texCoords>
layout (location = 1) in vec2 texture_coords; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;

uniform vec2 topRight;
uniform vec2 topLeft;
uniform vec2 bottomLeft;
uniform vec2 bottomRight;

void main()
{
	vec2 position  = topRight;
	
	if(vertex.x > 0.0 && vertex.y > 0.0)
		position = topRight;
	else if(vertex.x > 0.0 && vertex.y < 0.0)
		position = bottomRight;
	else if(vertex.x < 0.0 && vertex.y > 0.0)
		position = topLeft;
	else if(vertex.x < 0.0 && vertex.y < 0.0)
		position = bottomLeft;
	
	TexCoords = texture_coords;
    gl_Position = vec4(position,0.0, 1.0);
}