#include "Panel.h"

#ifndef GAMEMODE

Panel::Panel(const char* name) : name(name) {}

Panel::~Panel() {}

void Panel::OnOff()
{
	enabled = !enabled;
}

bool Panel::IsEnabled() const
{
	return enabled;
}

#endif