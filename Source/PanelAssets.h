#ifndef __PANEL_ASSETS_H__
#define __PANEL_ASSETS_H__

#include "GameMode.h"

#ifndef GAMEMODE

#include "Panel.h"

#include "ResourceTypes.h"
#include "Globals.h"
#include "ResourceShaderObject.h"

struct Directory;
struct Resource;

class PanelAssets : public Panel
{
public:

	PanelAssets(const char* name);
	~PanelAssets();

	bool Draw();

private:

	void RecursiveDrawAssetsDir(const Directory& assetsFile);
	
	void CreateShaderPopUp(const char* path);
	void DeleteShaderPopUp(const char* path);
	void CreateShaderConfirmationPopUp();
	void DeleteShaderConfirmationPopUp();

public:

	ShaderTypes shaderType = ShaderTypes::NoShaderType;
	std::string shaderFile;
	char shaderName[INPUT_BUF_SIZE];

	bool showCreateShaderConfirmationPopUp = false;
	bool showDeleteShaderConfirmationPopUp = false;
};

#endif

#endif // GAME
