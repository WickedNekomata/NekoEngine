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

private:
	bool enabled = false;
};

#endif

