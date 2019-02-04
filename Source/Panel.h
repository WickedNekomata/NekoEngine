#ifndef __PANEL_H__
#define __PANEL_H__

#include "GameMode.h"

#ifndef GAMEMODE

#define BLUE { 0.06f, 0.53f, 0.98f, 1.00f }

class Panel
{
public:

	Panel(const char* name);
	virtual ~Panel();

	virtual bool Draw() = 0;

	virtual inline void OnOff();
	virtual inline void SetOnOff(bool set) { enabled = set; }
	virtual inline bool IsEnabled() const;
	virtual inline const char* GetName() const { return name; }

protected:

	bool enabled = false;
	const char* name = nullptr;
};

#endif

#endif // GAME