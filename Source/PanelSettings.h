#ifndef __PANEL_SETTINGS_H__
#define __PANEL_SETTINGS_H__

#include "Panel.h"

#include "ImGui/imgui.h"

#define SCREEN_MIN_WIDTH 640
#define SCREEN_MIN_HEIGHT 480

class Module;
class QuadtreeNode;

class PanelSettings : public Panel
{
public:

	PanelSettings(char* name);
	~PanelSettings();

	bool Draw();

	void AddInput(const char* input);

private:

	void ApplicationNode() const;
	void WindowNode() const;
	void RendererNode() const;
	void FileSystemNode() const;
	void InputNode() const;
	void HardwareNode() const;
	void SceneNode() const;
	void TimeManagerNode() const;

	void RecursiveDrawQuadtreeHierarchy(QuadtreeNode* node) const;

private:

	ImGuiTextBuffer buf;
	ImVector<int> lineOffsets;
};

#endif

