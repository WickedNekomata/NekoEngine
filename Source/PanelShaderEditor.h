#ifndef __PANEL_SHADER_EDITOR_H__
#define __PANEL_SHADER_EDITOR_H__

#include "Panel.h"

#ifndef GAMEMODE

#include <list>
#include "Globals.h"

class ResourceShaderObject;
class ResourceShaderProgram;

class PanelShaderEditor : public Panel
{
public:

	PanelShaderEditor(char* name);
	~PanelShaderEditor();

	bool Draw();
	
	void OpenShaderInShaderEditor(uint shaderProgramUUID);
	uint GetShaderProgramUUID() const;

private:

	bool GetShaderObjects(std::list<ResourceShaderObject*>& shaderObjects) const;
	bool GetShaderObjectsIDs(std::list<uint>& shaderObjectsIDs) const;

	bool TryLink() const;

private:

	uint shaderProgramUUID = 0;

public:

	char shaderProgramName[INPUT_BUF_SIZE];

	std::list<uint> vertexShadersUUIDs;
	std::list<uint> fragmentShadersUUIDs;
};

#endif

#endif
