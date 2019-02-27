#include "ComponentLabel.h"

#include "ModuleUI.h"

#include "GameObject.h"
#include "Application.h"


ComponentLabel::ComponentLabel(GameObject * parent, ComponentTypes componentType) : Component(parent, ComponentTypes::LabelComponent)
{
	App->ui->componentsUI.push_back(this);
}

ComponentLabel::ComponentLabel(const ComponentLabel & componentLabel) : Component(parent, ComponentTypes::LabelComponent)
{
	App->ui->componentsUI.push_back(this);

}

ComponentLabel::~ComponentLabel()
{
	App->ui->componentsUI.remove(this);
}

void ComponentLabel::Update()
{
}

uint ComponentLabel::GetInternalSerializationBytes()
{
	return 0u;
}

void ComponentLabel::OnInternalSave(char *& cursor)
{
}

void ComponentLabel::OnInternalLoad(char *& cursor)
{
}

void ComponentLabel::OnUniqueEditor()
{

}
