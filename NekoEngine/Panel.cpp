#include "Panel.h"

Panel::Panel(char* name) : name(name)
{
}

Panel::~Panel()
{
}

void Panel::OnOff()
{
	enabled = !enabled;
}

bool Panel::IsEnabled()
{
	return enabled;
}
