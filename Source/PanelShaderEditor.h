#ifndef __PANEL_SHADER_EDITOR_H__
#define __PANEL_SHADER_EDITOR_H__

#include "Panel.h"

#ifndef GAMEMODE

#include <list>
#include "Globals.h"

#define vShaderTemplate \
"#version 330 core\n" \
"\n" \
"layout (location = 0) in vec3 position;\n" \
"layout (location = 1) in vec4 normals;\n" \
"layout (location = 2) in vec4 color;\n" \
"layout (location = 3) in vec2 texCoord;\n" \
"\n" \
"uniform mat4 model_matrix;\n" \
"uniform mat4 view_matrix;\n" \
"uniform mat4 proj_matrix;\n" \
"\n" \
"out vec4 ourColor;\n" \
"out vec2 ourTexCoord;\n" \
"\n" \
"void main()\n" \
"{\n" \
"    ourTexCoord = texCoord;\n" \
"    ourColor = color;\n" \
"    gl_Position = proj_matrix * view_matrix * model_matrix * vec4(position, 1.0f);\n" \
"}\n"

#define fShaderTemplate \
"#version 330 core\n" \
"\n" \
"in vec4 ourColor;\n" \
"in vec2 ourTexCoord;\n" \
"out vec4 FragColor;\n" \
"\n" \
"uniform sampler2D ourTexture_0;\n" \
"\n" \
"void main()\n" \
"{\n" \
"     FragColor = texture(ourTexture_0, ourTexCoord);\n" \
"}\n" \

class PanelShaderEditor : public Panel
{
public:

	PanelShaderEditor(char* name);
	~PanelShaderEditor();

	bool Draw();

private:

	std::list<uint> vertexShaders;
	std::list<uint> fragmentShaders;
};

#endif

#endif
