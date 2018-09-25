#include "Panel.h"

Panel::Panel(char* title)
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
