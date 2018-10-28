#ifndef __PANEL_SETTINGS_H__
#define __PANEL_SETTINGS_H__

#include "Panel.h"

#include "ImGui/imgui.h"

#define SCREEN_MIN_WIDTH 640
#define SCREEN_MIN_HEIGHT 480

class Module;

class PanelSettings : public Panel
{
public:

	PanelSettings(char* name);
	~PanelSettings();

	bool Draw();

	void AddInput(const char* input);

private:

	void ApplicationNode() const;

	bool IsActiveNode(Module* module) const;

	void WindowNode() const;
	void RendererNode() const;
	void FileSystemNode() const;
	void InputNode() const;
	void HardwareNode() const;
	void SceneNode() const;

	void RecursiveDrawQuadtree() const;

private:

	ImGuiTextBuffer buf;
	ImVector<int> lineOffsets;
};

#endif

