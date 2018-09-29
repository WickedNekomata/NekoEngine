#ifndef __PANEL_H__
#define __PANEL_H__

class Panel
{
public:
	Panel(char* title);
	virtual ~Panel();

	virtual bool Draw() = 0;

	virtual inline void OnOff();
	virtual inline void SetOnOff(bool set) { enabled = set; }
	virtual inline bool IsEnabled();
	virtual inline char* GetName() { return name; }

protected:
	bool enabled = false;
	char* name = nullptr;
};

#endif

