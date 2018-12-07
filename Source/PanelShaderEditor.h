#ifndef __PANEL_SHADER_EDITOR_H__
#define __PANEL_SHADER_EDITOR_H__

#include "Panel.h"

#ifndef GAMEMODE

#include <list>
#include "Globals.h"

class ResourceShaderObject;

class PanelShaderEditor : public Panel
{
public:

	PanelShaderEditor(char* name);
	~PanelShaderEditor();

	bool Draw();

private:

	std::list<ResourceShaderObject*> vertexShaders;
	std::list<ResourceShaderObject*> fragmentShaders;
};

#endif

#endif
