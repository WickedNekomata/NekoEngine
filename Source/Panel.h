#ifndef __PANEL_H__
#define __PANEL_H__

#include "GameMode.h"

#ifndef GAMEMODE

#define BLUE { 0.06f, 0.53f, 0.98f, 1.00f }

class Panel
{
public:

	Panel(char* title);
	virtual ~Panel();

	virtual bool Draw() = 0;

	virtual inline void OnOff();
	virtual inline void SetOnOff(bool set) { enabled = set; }
	virtual inline bool IsEnabled() const;
	virtual inline char* GetName() const { return name; }

protected:

	bool enabled = false;
	char* name = nullptr;
};

#endif

#endif // GAME