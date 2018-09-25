#include "PanelPreferences.h"

#include "ImGui/imgui.h"

#include "SDL/include/SDL_cpuinfo.h"
#include "SDL/include/SDL_version.h"

#include <windows.h>
#include <gl/GL.h>

#define GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX			0x9047
#define GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX		0x9048
#define GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX    0x9049
#define GPU_MEMORY_INFO_EVICTION_COUNT_NVX				0x904A
#define GPU_MEMORY_INFO_EVICTED_MEMORY_NVX				0x904B

PanelPreferences::PanelPreferences(char* name) : Panel(name)
{
}

PanelPreferences::~PanelPreferences()
{
}

bool PanelPreferences::Draw()
{
	ImGui::SetNextWindowSize({ 325, 300 });
	ImGui::Begin(name, &enabled);
	if (ImGui::TreeNode("Application"))
	{
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Window"))
	{
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Hardware"))
	{
		HardwareNode();	
		ImGui::TreePop();
	}
	ImGui::End();

	return true;
}

void PanelPreferences::HardwareNode()
{
	SDL_version version;
	SDL_GetVersion(&version);

	ImGui::Text("OpenGL version: %s", glGetString(GL_VERSION));
	ImGui::Text("SDL version: %i.%i.%i", version.major, version.minor, version.patch);

	ImGui::Separator();

	// CPU
	ImGui::Text("CPUs:"); ImGui::SameLine();
	ImGui::TextColored({ 239,201,0,255 }, "%i (Cache: %ikb)", SDL_GetCPUCount(), SDL_GetCPUCacheLineSize());

	// RAM
	ImGui::Text("System Ram:"); ImGui::SameLine();
	ImGui::TextColored({ 239,201,0,255 }, "%iMb", SDL_GetSystemRAM());

	// Capabilites
	ImGui::Text("Caps:"); ImGui::SameLine();
	ImGui::TextColored({ 239,201,0,255 }, "%s%s%s%s%s%s%s%s%s%s%s", (SDL_HasAVX()) ? "AVX " : "", (SDL_HasAVX2()) ? "AVX2 " : "", (SDL_HasAltiVec()) ? "AltiVec " : "",
		(SDL_Has3DNow()) ? "3DNow " : "", (SDL_HasMMX()) ? "MMX " : "", (SDL_HasRDTSC()) ? "RDTSC " : "", (SDL_HasSSE()) ? "SEE " : "",
		(SDL_HasSSE2()) ? "SSE2 " : "", (SDL_HasSSE3()) ? "SSE3 " : "", (SDL_HasSSE41()) ? "SSE41 " : "",
		(SDL_HasSSE42()) ? "SSE42 " : "");

	ImGui::Separator();

	//Gpu
	ImGui::Text("Gpu:"); ImGui::SameLine();
	ImGui::TextColored({ 239,201,0,255 }, "%s", glGetString(GL_RENDERER));
	ImGui::Text("Brand:"); ImGui::SameLine();
	ImGui::TextColored({ 239,201,0,255 }, "%s", glGetString(GL_VENDOR));

	GLint totalMemory;
	glGetIntegerv(GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalMemory);
	GLint memoryUsage = 0;
	GLint reservedMemory = 0;
	GLint availableMemory = 0;
	glGetIntegerv(GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &availableMemory);
	glGetIntegerv(GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &reservedMemory);
	memoryUsage = totalMemory - availableMemory;

	ImGui::Text("VRAM Budget:"); ImGui::SameLine();
	ImGui::TextColored({ 239,201,0,255 }, "%.2fMb", totalMemory * 0.001);
	ImGui::Text("VRAM Usage:"); ImGui::SameLine();
	ImGui::TextColored({ 239,201,0,255 }, "%.2fMb", memoryUsage * 0.001);
	ImGui::Text("VRAM Available:");	ImGui::SameLine();
	ImGui::TextColored({ 239,201,0,255 }, "%.2fMb", availableMemory * 0.001);
	ImGui::Text("VRAM Reserved:");	ImGui::SameLine();
	ImGui::TextColored({ 239,201,0,255 }, "%.2fMb", reservedMemory * 0.001);

}
