#ifndef __PANEL_H__
#define __PANEL_H__

class Panel
{
public:
	Panel(char* title);
	virtual ~Panel();

	virtual bool Draw() = 0;

	virtual inline void OnOff();
	virtual inline bool IsEnabled();

protected:
	bool enabled = false;
	char* name = nullptr;
};

#endif

