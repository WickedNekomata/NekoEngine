#include "PanelAbout.h"

#include "Globals.h"

#include <windows.h>
#include "glew\include\GL\glew.h"
#include "SDL/include/SDL_cpuinfo.h"
#include "SDL/include/SDL_version.h"
#include "ImGui/imgui.h"
#include "Assimp/include/version.h"
#include "physfs/include/physfs.h"

#include "DevIL/include/il.h"
#include "DevIL/include/ilu.h"
#include "DevIL/include/ilut.h"

PanelAbout::PanelAbout(char* name) : Panel(name) {}

PanelAbout::~PanelAbout() {}

bool PanelAbout::Draw()
{
	//ImGui::SetNextWindowPos({ 100,50 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 400,400 }, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags inspectorFlags = 0;
	inspectorFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	inspectorFlags |= ImGuiWindowFlags_NoSavedSettings; // TODO: Save & Load

	if (ImGui::Begin(name, &enabled, inspectorFlags))
	{
		// Engine description
		ImGui::Text("Neko Engine");
		ImGui::Spacing();
		ImGui::TextWrapped("Neko Engine is a 3D game engine currently being developed by\ntwo students from CITM-UPC Terrassa.");
		if (ImGui::Button("NekoEngine")) { OpenInBrowser("https://github.com/WickedNekomata/NekoEngine"); }
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// Authors
		ImGui::Text("Authors");
		ImGui::Spacing();
		if (ImGui::Button("Sandra Alvarez")) { OpenInBrowser("https://github.com/Sandruski"); }
		if (ImGui::Button("Guillem Costa")) { OpenInBrowser("https://github.com/DatBeQuiet"); }
		ImGui::Spacing();
		if (ImGui::Button("WickedNekomata")) { OpenInBrowser("https://github.com/WickedNekomata"); }
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// Libraries
		ImGui::Text("Libraries");
		ImGui::Spacing();

		SDL_version version;
		SDL_GetVersion(&version);
		if (ImGui::Button("SDL")) { OpenInBrowser("https://www.libsdl.org/"); }
		ImGui::SameLine(); ImGui::TextWrapped("%i.%i.%i", version.major, version.minor, version.patch);

		if (ImGui::Button("OpenGL")) { OpenInBrowser("https://www.opengl.org/"); }
		ImGui::SameLine(); ImGui::TextWrapped("%s", glGetString(GL_VERSION));
		
		if (ImGui::Button("Glew")) { OpenInBrowser("http://glew.sourceforge.net/"); }
		ImGui::SameLine(); ImGui::TextWrapped("%s", glewGetString(GLEW_VERSION));

		if (ImGui::Button("Assimp")) { OpenInBrowser("http://cms.assimp.org/index.php"); }
		ImGui::SameLine(); ImGui::TextWrapped("%i.%i.%i", aiGetVersionMajor(), aiGetVersionMinor(), aiGetVersionRevision());
		
		int devilVersion = 0;
		if (!(ilGetInteger(IL_VERSION_NUM) < IL_VERSION ||
			iluGetInteger(ILU_VERSION_NUM) < ILU_VERSION ||
			ilutGetInteger(ILUT_VERSION_NUM) < ILUT_VERSION))
			devilVersion = IL_VERSION;

		if (ImGui::Button("DevIL")) { OpenInBrowser("http://openil.sourceforge.net/"); }
		ImGui::SameLine(); ImGui::TextWrapped("%i", devilVersion);

		if (ImGui::Button("ImGui")) { OpenInBrowser("https://github.com/ocornut/imgui"); }
		ImGui::SameLine(); ImGui::TextWrapped("%s", ImGui::GetVersion());

		if (ImGui::Button("MathGeoLib")) { OpenInBrowser("https://github.com/juj/MathGeoLib"); }
		ImGui::SameLine(); ImGui::TextWrapped("1.5");

		if (ImGui::Button("PCG Random Number Generator")) { OpenInBrowser("http://www.pcg-random.org/"); }
		ImGui::SameLine(); ImGui::TextWrapped("0.94");

		if (ImGui::Button("Parson")) { OpenInBrowser("https://github.com/kgabis/parson"); }

		if (ImGui::Button("PhysFS")) { OpenInBrowser("https://icculus.org/physfs/"); }
		ImGui::SameLine(); ImGui::TextWrapped("%i.%i.%i", PHYSFS_VER_MAJOR, PHYSFS_VER_MINOR, PHYSFS_VER_PATCH);

		if (ImGui::Button("MMGR")) { OpenInBrowser("http://www.paulnettle.com/"); }
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// License
		ImGui::TextWrapped("MIT License");
		ImGui::TextWrapped("Copyright(c) 2018 WickedNekomata (Sandra Alvarez & Guillem Costa)");
		ImGui::TextWrapped("Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the %cSoftware%c), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:", 34, 34);
		ImGui::TextWrapped("The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.");
		ImGui::TextWrapped("THE SOFTWARE IS PROVIDED %cAS IS%c, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.", 34, 34);
		ImGui::End();
	}
	
	return true;
}