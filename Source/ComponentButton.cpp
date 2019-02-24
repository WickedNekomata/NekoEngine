#include "ComponentButton.h"
#include "ComponentRectTransform.h"

#include "ModuleUI.h"
#include "ModuleInput.h"

#include "GameObject.h"
#include "Application.h"
ComponentButton::ComponentButton(GameObject * parent, ComponentTypes componentType) : Component(parent, ComponentTypes::ButtonComponent)
{
	App->ui->componentsUI.push_back(this);
	state = UIState::IDLE;
	input = "z";
	scancode = (uint)SDL_GetScancodeFromKey(SDL_GetKeyFromName(input.c_str()));
}

ComponentButton::ComponentButton(const ComponentButton & componentButton) : Component(parent, ComponentTypes::ButtonComponent)
{
	state = componentButton.state;
}

ComponentButton::~ComponentButton()
{
	App->ui->componentsUI.remove(this);
}

void ComponentButton::Update()
{
	const uint* rect = parent->cmp_rectTransform->GetRect();

	if(App->input->GetKey(scancode) == KEY_DOWN)
		CONSOLE_LOG(LogTypes::Normal, "GG");

	switch (state)
	{
	case IDLE:
		if (MouseInScreen(rect))
			state = HOVERED;
		break;
	case HOVERED:
		if (!MouseInScreen(rect))
			state = IDLE;
		else if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_DOWN)
			state = R_CLICK;
		else if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
			state = L_CLICK;
		CONSOLE_LOG(LogTypes::Normal, "HOVER");
		break;
	case R_CLICK:
		CONSOLE_LOG(LogTypes::Normal, "CLICKRRRR");
		state = HOVERED;
		break;
	case L_CLICK:
		CONSOLE_LOG(LogTypes::Normal, "CLICKLLL");
		state = HOVERED;
		break;
	default:
		break;
	}
}

uint ComponentButton::GetInternalSerializationBytes()
{
	return sizeof(UIState);
}

void ComponentButton::OnInternalSave(char *& cursor)
{
	size_t bytes = sizeof(UIState);
	memcpy(cursor, &state, bytes);
	cursor += bytes;
}

void ComponentButton::OnInternalLoad(char *& cursor)
{
	size_t bytes = sizeof(UIState);
	memcpy(&state, cursor, bytes);
	cursor += bytes;
}

void ComponentButton::OnUniqueEditor()
{
}

bool ComponentButton::MouseInScreen(const uint* rect) const
{
	uint mouseX = App->input->GetMouseX();
	uint mouseY = App->input->GetMouseY();

	return mouseX > rect[X_RECT] && mouseX < rect[X_RECT] + rect[XDIST_RECT]
		&& mouseY > rect[Y_RECT] && mouseY < rect[Y_RECT] + rect[YDIST_RECT];
}
