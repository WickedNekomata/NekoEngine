#ifndef __PANEL_ASSETS_H__
#define __PANEL_ASSETS_H__



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
	
	void CreateResourcePopUp(const char* path);
	void DeleteResourcePopUp(const char* path);
	void CreateResourceConfirmationPopUp();
	void DeleteResourceConfirmationPopUp();

public:

	// Create resource
	std::string file;
	std::string extension;
	char resourceName[INPUT_BUF_SIZE];
	
	bool showCreateResourceConfirmationPopUp = false;
	bool showDeleteResourceConfirmationPopUp = false;
};

#endif

#endif // GAME
