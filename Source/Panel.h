#ifndef __PANEL_H__
#define __PANEL_H__

#define YELLOW { 239, 201, 0, 255 }
#define WHITE { 255, 255, 255, 255 }

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

