#ifndef __PANEL_SHADER_EDITOR_H__
#define __PANEL_SHADER_EDITOR_H__

#include "Panel.h"

#ifndef GAMEMODE

#include <list>
#include "Globals.h"

#include "ResourceShaderProgram.h"

class ResourceShaderObject;

class PanelShaderEditor : public Panel
{
public:

	PanelShaderEditor(const char* name);
	~PanelShaderEditor();

	bool Draw();
	
	void OpenShaderInShaderEditor(uint shaderProgramUuid);
	uint GetShaderProgramUuid() const;

private:

	bool GetShaderObjects(std::list<ResourceShaderObject*>& shaderObjects) const;

	bool TryLink() const;

private:

	uint shaderProgramUuid = 0;

public:

	char shaderProgramName[INPUT_BUF_SIZE];
	ShaderProgramTypes shaderProgramType = ShaderProgramTypes::Custom;

	std::list<uint> vertexShadersUuids;
	std::list<uint> fragmentShadersUuids;
};

#endif

#endif
