#include "PanelConsole.h"

#ifndef GAMEMODE

#include "Application.h"
#include "ModuleWindow.h"

#include "Globals.h"

PanelConsole::PanelConsole(const char* name) : Panel(name) {}

PanelConsole::~PanelConsole()
{
	Clear();
}

bool PanelConsole::Draw()
{
	ImGuiWindowFlags consoleFlags = 0;
	consoleFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, consoleFlags))
	{
		if (ImGui::SmallButton("Clear"))
		{
			Clear();
		}

		static bool showLogs = true, showWarnings = true, showErrors = true;

		ImGui::SameLine();
		ImGui::Checkbox("Auto Scroll", &scrollToBottom);
		ImGui::SameLine();
		ImGui::Checkbox("Logs", &showLogs);
		ImGui::SameLine();
		ImGui::Checkbox("Warnings", &showWarnings);
		ImGui::SameLine();
		ImGui::Checkbox("Errors", &showErrors);
		ImGui::SameLine();

		filter.Draw("Filter", -100.0f);

		ImGui::Separator();

		ImGuiWindowFlags scrollFlags = 0;
		scrollFlags |= ImGuiWindowFlags_HorizontalScrollbar;
		scrollFlags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;

		if (ImGui::BeginChild("scroll", ImVec2(0, 0), false, scrollFlags))
		{
			// Display logs
			if (filter.IsActive())
			{
				const char* line = buf.begin();

				for (int lineIndex = 0; line != nullptr; ++lineIndex)
				{
					const char* lineEnd;

					if (lineIndex < lineOffsets.Size)
						lineEnd = buf.begin() + lineOffsets[lineIndex];
					else
						lineEnd = nullptr;

					// Print filtered text
					if (filter.PassFilter(line, lineEnd))
					{		
						char lineToPrint[DEFAULT_BUF_SIZE];
						uint toPrintLength = strlen(line) - strlen(lineEnd);
						strncpy(lineToPrint, line, toPrintLength);
						lineToPrint[toPrintLength] = '\0';

						if (strstr(lineToPrint, "Error: ") != nullptr)
						{
							if (showErrors)
								ImGui::TextColored({ 1,0,0,1 }, lineToPrint);
						}
						else if (strstr(lineToPrint, "Warning: ") != nullptr)
						{
							if (showWarnings)
								ImGui::TextColored({ .95f,.81f,0.24f,1.0f }, lineToPrint);
						}
						else if (showLogs)
							ImGui::TextUnformatted(lineToPrint);
					}
						
					if (lineEnd != nullptr && lineEnd[1] != '\0')
						line = lineEnd + 1;
					else
						line = nullptr;
				}
			}
			else
			{
				// Print text
				uint charsRead = 0;
				while (charsRead < buf.size())
				{
					char line[DEFAULT_BUF_SIZE];
					char character[2];
					strcpy(line, "");

					while (buf[charsRead] != '\n')
					{				
						character[0] = buf[charsRead];
						character[1] = '\0';
						strcat(line, character);
						charsRead++;
					}

					strcat(line, "\n");
					charsRead++;

					if (strstr(line, "Error: ") != nullptr)
					{
						if (showErrors)
							ImGui::TextColored({ 1.0f,0.0f,0.0f,1.0f }, line);
					}
					else if (strstr(line, "Warning: ") != nullptr)
					{
						if (showWarnings)
							ImGui::TextColored({ .95f,.81f,0.24f,1.0f }, line);
					}
					else if (showLogs)
						ImGui::TextUnformatted(line);
				}			
			}			
		}

		if (scrollToBottom)
			ImGui::SetScrollHereY(1.0f);

		ImGui::EndChild();
	}
	ImGui::End();

	return true;
}

void PanelConsole::AddLog(const char* log) 
{
	int oldSize = buf.size();

	buf.appendf(log);

	for (int newSize = buf.size(); oldSize < newSize; ++oldSize)
	{
		if (buf[oldSize] == '\n')
			lineOffsets.push_back(oldSize);
	}
}

void PanelConsole::Clear() 
{
	buf.clear();
	lineOffsets.clear();
}

#endif // GAME