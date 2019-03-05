#ifndef __PANEL_SHADER_EDITOR_H__
#define __PANEL_SHADER_EDITOR_H__

#ifndef GAMEMODE

#include "Panel.h"
#include "Globals.h"

#include <vector>

class PanelShaderEditor : public Panel
{
public:

	PanelShaderEditor(const char* name);
	~PanelShaderEditor();

	bool Draw();
	
	void OpenShaderInShaderEditor(uint shaderProgramUuid);

private:

	bool TryLink() const;
	uint GetShaderObjects(std::vector<uint>& shaderObjectsUuids) const;

private:

	uint shaderProgramUuid = 0;

public:

	char shaderProgramName[INPUT_BUF_SIZE];

	std::vector<uint> vertexShadersUuids;
	std::vector<uint> fragmentShadersUuids;
};

#endif

#endif
