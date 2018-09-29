#include "PanelAbout.h"
#include "Globals.h"

#include <windows.h>
#include <gl/GL.h>
#include "SDL/include/SDL_cpuinfo.h"
#include "SDL/include/SDL_version.h"
#include "ImGui/imgui.h"

PanelAbout::PanelAbout(char* name) : Panel(name)
{
}


PanelAbout::~PanelAbout()
{
}

bool PanelAbout::Draw()
{
	ImGuiWindowFlags inspectorFlags = 0;
	inspectorFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	ImGui::Begin(name, &enabled, inspectorFlags);
	ImGui::Text("Neko Engine");
	ImGui::Spacing();
	ImGui::Text("This is a 3D game engine being developed by\ntwo students from CITM-UPC Terrassa.");
	ImGui::Separator();
	ImGui::Text("Authors:");
	if (ImGui::Button("Sandra Alvarez")) { OpenInBrowser("https://github.com/Sandruski"); }
	if (ImGui::Button("Guillem Costa")) { OpenInBrowser("https://github.com/DatBeQuiet"); }
	ImGui::Separator();
	ImGui::Text("Libraries:");

	SDL_version version;
	SDL_GetVersion(&version);

	ImGui::Text("ImGuI, OpenGL %s, \nSDL %i.%i.%i, MathGeoLib, PCG, Parson, MMGR", glGetString(GL_VERSION), version.major, version.minor, version.patch);
	ImGui::Separator();

	// TODO: only one imgui::text for MIT
	ImGui::Text("MIT License Copyright(c) 2018 WickedNekomata");
	ImGui::Text("Permission is hereby granted, free of charge, to any person obtaining a copy");
	ImGui::Text("of this software and associated documentation files(the %cSoftware%c), to deal",34,34);
	ImGui::Text("in the Software without restriction, including without limitation the rights");
	ImGui::Text("to use, copy, modify, merge, publish, distribute, sublicense, and/or sell");
	ImGui::Text("copies of the Software, and to permit persons to whom the Software is");
	ImGui::Text("furnished to do so, subject to the following conditions :");
	ImGui::Text("The above copyright notice and this permission notice shall be included in all");
	ImGui::Text("copies or substantial portions of the Software.");
	ImGui::Text("THE SOFTWARE IS PROVIDED %cAS IS%c, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR",34,34);
	ImGui::Text("IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,");
	ImGui::Text("FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE");
	ImGui::Text("AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER");
	ImGui::Text("LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,");
	ImGui::Text("OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE");
	ImGui::Text("SOFTWARE.");
	ImGui::End();

	return true;
}