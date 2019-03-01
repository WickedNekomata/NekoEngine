#include "ComponentButton.h"
#include "ComponentRectTransform.h"
#include "ComponentScript.h"

#include "ModuleUI.h"
#include "ModuleInput.h"
#include "ScriptingModule.h"

#include "GameObject.h"
#include "Application.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"
#include "imgui\imgui_stl.h"

#include <mono/metadata/attrdefs.h>

ComponentButton::ComponentButton(GameObject * parent, ComponentTypes componentType) : Component(parent, ComponentTypes::ButtonComponent)
{
	App->ui->componentsUI.push_back(this);
	state = UIState::IDLE;
	input = "z";
	button_blinded = (uint)SDL_GetScancodeFromKey(SDL_GetKeyFromName(input.c_str()));
	if (parent->cmp_image == nullptr)
		parent->AddComponent(ImageComponent);
}

ComponentButton::ComponentButton(const ComponentButton & componentButton, GameObject* parent) : Component(parent, ComponentTypes::ButtonComponent)
{
	state = componentButton.state;
	button_blinded = componentButton.button_blinded;
	input = componentButton.input;
	App->ui->componentsUI.push_back(this);
}

ComponentButton::~ComponentButton()
{
	App->ui->componentsUI.remove(this);
}

void ComponentButton::OnSystemEvent(System_Event event)
{
	Component::OnSystemEvent(event);

	switch (event.type)
	{
		case System_Event_Type::ScriptingDomainReloaded:
		{
			methodToCall = nullptr;
			scriptInstance = nullptr;

			break;
		}
	}

}

void ComponentButton::Update()
{
	const uint* rect = parent->cmp_rectTransform->GetRect();

	if (App->input->GetKey(button_blinded) == KEY_DOWN)
		KeyPressed();

	switch (state)
	{
	case IDLE:
		if (MouseInScreen(rect))
		{
			state = HOVERED;
		}
		break;
	case HOVERED:
		if (!MouseInScreen(rect))
		{
			state = IDLE;
		}
		else if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_DOWN)
		{
			state = R_CLICK;
			RightClickPressed();
		}
		else if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
		{
			state = L_CLICK;
			KeyPressed();
		}
		break;
	case R_CLICK:
		state = HOVERED;
		break;
	case L_CLICK:
		state = HOVERED;
		break;
	default:
		break;
	}
}

uint ComponentButton::GetInternalSerializationBytes()
{
	return sizeof(uint);
}

void ComponentButton::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(uint);
	memcpy(cursor, &button_blinded, bytes);
	cursor += bytes;
}

void ComponentButton::OnInternalLoad(char *& cursor)
{
	size_t bytes = sizeof(uint);
	memcpy(&button_blinded, cursor, bytes);
	cursor += bytes;

	SetNewKey(button_blinded);
}

void ComponentButton::OnUniqueEditor()
{
#ifndef GAMEMODE


	if (ImGui::InputText("Blind key", &input), ImGuiInputTextFlags_EnterReturnsTrue)
	{
		input[1] = '\0';
		button_blinded = (uint)SDL_GetScancodeFromKey(SDL_GetKeyFromName(input.data()));
	}

	ImGui::Text(SDL_GetKeyName(SDL_GetKeyFromScancode((SDL_Scancode)button_blinded)));

	switch (state)
	{
	case IDLE:
		ImGui::Text("This button is IDLE.");
		break;
	case HOVERED:
		ImGui::Text("This button is Hovered.");
		break;
	case R_CLICK:
		ImGui::Text("This button is Right Clicked.");
		break;
	case L_CLICK:
		ImGui::Text("This button is Left Clicked.");
		break;
	}

	ImVec2 cursorPos = ImGui::GetCursorScreenPos();

	ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 4 });

	ImGui::Text("OnClick: "); ImGui::SameLine();
	
	cursorPos = ImGui::GetCursorScreenPos();

	ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y - 4 });

	uint buttonWidth = 0.65 * ImGui::GetWindowWidth();
	cursorPos = ImGui::GetCursorScreenPos();
	ImGui::ButtonEx(("##" + std::to_string(UUID)).data(), { (float)buttonWidth, 20 }, ImGuiButtonFlags_::ImGuiButtonFlags_Disabled);

	bool dragged = false;

	//Dragging GameObjects
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECTS_HIERARCHY", ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
		if (payload)
		{
			GameObject* go = *(GameObject**)payload->Data;

			if (ImGui::IsMouseReleased(0))
			{
				draggedGO = go;
				dragged = true;
			}
		}
		ImGui::EndDragDropTarget();
	}

	if(dragged)
		ImGui::OpenPopup("OnClick Method");

	if (ImGui::BeginPopup("OnClick Method", 0))
	{
		std::vector<Component*> scripts = draggedGO->GetComponents(ComponentTypes::ScriptComponent);
		for (int i = 0; i < scripts.size(); ++i)
		{
			ComponentScript* script = (ComponentScript*)scripts[i];
			if (ImGui::BeginMenu(script->scriptName.data()))
			{
				MonoObject* compInstance = script->classInstance;
				MonoClass* compClass = mono_object_get_class(compInstance);

				const char* className = mono_class_get_name(compClass);

				bool somethingClicked = false;

				void* iterator = 0;
				MonoMethod* method = mono_class_get_methods(compClass, &iterator);
				while (method)
				{					
					uint32_t flags = 0;
					uint32_t another = mono_method_get_flags(method, &flags);
					if (another & MONO_METHOD_ATTR_PUBLIC && !(another & MONO_METHOD_ATTR_STATIC))
					{
						std::string name = mono_method_get_name(method);
						if (name != ".ctor")
						{
							if (ImGui::MenuItem((name + "()").data()))
							{
								//Set this method and instance to be called
								methodToCall = method;
								scriptInstance = compInstance;
								somethingClicked = true;
								break;
							}
						}
					}
					method = mono_class_get_methods(compClass, &iterator);
				}

				ImGui::EndMenu();

				if (somethingClicked)
					break;
			}
		}

		ImGui::EndPopup();
	}

	if (ImGui::IsItemClicked(0))
	{
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(cursorPos, { cursorPos.x + buttonWidth, cursorPos.y + 20 }, ImGui::GetColorU32(ImGuiCol_::ImGuiCol_ButtonActive));
	}
	else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
	{
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(cursorPos, { cursorPos.x + buttonWidth, cursorPos.y + 20 }, ImGui::GetColorU32(ImGuiCol_::ImGuiCol_ButtonHovered));

		if (App->input->GetKey(SDL_SCANCODE_BACKSPACE) == KEY_DOWN)
		{
			

			
		}
	}

	//Button text

	std::string text;

	if (methodToCall)
	{
		text = mono_method_get_name(methodToCall) + std::string("()");

		ImGui::SetCursorScreenPos({ cursorPos.x + 7, cursorPos.y + 3 });

		ImGui::Text(text.data());

		cursorPos = ImGui::GetCursorScreenPos();
		ImGui::SetCursorScreenPos({ cursorPos.x, cursorPos.y + 4 });
		
	}


#endif
}

bool ComponentButton::MouseInScreen(const uint* rect) const
{
	uint mouseX = App->input->GetMouseX();
	uint mouseY = App->input->GetMouseY();

	return mouseX > rect[X_RECT] && mouseX < rect[X_RECT] + rect[XDIST_RECT]
		&& mouseY > rect[Y_RECT] && mouseY < rect[Y_RECT] + rect[YDIST_RECT];
}

void ComponentButton::SetNewKey(const char * key)
{
	input = key;
	button_blinded = (uint)SDL_GetScancodeFromKey(SDL_GetKeyFromName(key));
}

void ComponentButton::SetNewKey(uint key)
{
	input = SDL_GetKeyName(SDL_GetKeyFromScancode((SDL_Scancode)key));
	button_blinded = key;
}

void ComponentButton::KeyPressed()
{
	if (methodToCall && scriptInstance)
	{
		MonoObject* exc = nullptr;
		if (IsTreeActive())
		{
			mono_runtime_invoke(methodToCall, scriptInstance, NULL, &exc);
			if (exc)
			{
				System_Event event;
				event.type = System_Event_Type::Pause;
				App->PushSystemEvent(event);
				App->Pause();

				MonoString* exceptionMessage = mono_object_to_string(exc, NULL);
				char* toLogMessage = mono_string_to_utf8(exceptionMessage);
				CONSOLE_LOG(LogTypes::Error, toLogMessage);
				mono_free(toLogMessage);
			}
		}	
	}
}

void ComponentButton::RightClickPressed()
{
}

UIState ComponentButton::GetState() const
{
	return state;
}
