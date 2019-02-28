#include "ComponentButton.h"
#include "ComponentRectTransform.h"

#include "ModuleUI.h"
#include "ModuleInput.h"

#include "GameObject.h"
#include "Application.h"

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

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
		}
		else if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
		{
			state = L_CLICK;
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
}

void ComponentButton::OnUniqueEditor()
{
#ifndef GAMEMODE

	static char inputBlind[2] = { input.at(0) };
	if (ImGui::InputText("Blind key", inputBlind, IM_ARRAYSIZE(inputBlind), ImGuiInputTextFlags_EnterReturnsTrue))
		SetNewKey(inputBlind);

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
	CONSOLE_LOG(LogTypes::Normal, "GG");
}

UIState ComponentButton::GetState() const
{
	return state;
}
