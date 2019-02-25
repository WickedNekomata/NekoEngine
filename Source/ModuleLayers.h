#ifndef __LAYERS_H__
#define __LAYERS_H__

#include "Module.h"

#include "Globals.h"

#include <string>
#include <vector>
#include <map>

#define BIT_SHIFT(x) 1 << x

constexpr uint MAX_NUM_LAYERS(32);
constexpr uint DEFAULT_FILTER_MASK(	BIT_SHIFT(0) | BIT_SHIFT(1) | BIT_SHIFT(2) | BIT_SHIFT(3) | BIT_SHIFT(4) | BIT_SHIFT(5)	| 
									BIT_SHIFT(6) | BIT_SHIFT(7) | BIT_SHIFT(8) | BIT_SHIFT(9) | BIT_SHIFT(10) | BIT_SHIFT(11) |
									BIT_SHIFT(12) | BIT_SHIFT(13) | BIT_SHIFT(14) | BIT_SHIFT(15) | BIT_SHIFT(16) | BIT_SHIFT(17) |
									BIT_SHIFT(18) | BIT_SHIFT(19) | BIT_SHIFT(20) | BIT_SHIFT(21) | BIT_SHIFT(22) | BIT_SHIFT(23) |
									BIT_SHIFT(24) | BIT_SHIFT(25) | BIT_SHIFT(26) | BIT_SHIFT(27) | BIT_SHIFT(28) | BIT_SHIFT(29) |
									BIT_SHIFT(30) | BIT_SHIFT(31));

class Layer
{
public:

	Layer();
	Layer(uint number);
	~Layer();

	uint GetNumber() const;
	uint GetFilterGroup() const;

	void SetFilterMask(uint filterMask);
	uint GetFilterMask() const;

public:

	std::string name = "";
	bool builtin = false;

private:

	uint number = 0;
	uint filterMask = 0;
};

class ModuleLayers : public Module
{
public:

	ModuleLayers(bool start_enabled = true);
	~ModuleLayers();

	bool Init(JSON_Object* jObject);

	void SaveStatus(JSON_Object* jObject) const;
	void LoadStatus(const JSON_Object* jObject);

	// ----------------------------------------------------------------------------------------------------

	void SetLayerName(uint layerNumber, const char* layerName) const;

	const char* NumberToName(uint layerNumber) const;
	int NameToNumber(const char* layerName) const;

	Layer* GetLayer(uint layerNumber) const;
	std::vector<Layer*> GetLayers() const;

private:

	std::vector<Layer*> layers;
};

#endif