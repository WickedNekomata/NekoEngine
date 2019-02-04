#ifndef __LAYERS_H__
#define __LAYERS_H__

#include "Globals.h"

#include <string>
#include <vector>
#include <map>

#define BIT_SHIFT(x) 1 << x

constexpr uint MAX_NUM_LAYERS(32);

class Layer
{
public:

	Layer();
	Layer(uint number);
	~Layer();

	uint GetNumber() const;
	uint GetFilterGroup() const;

public:

	std::string name = "";
	bool builtin = false;

	uint filterMask = 0;

private:

	uint number = 0;
};

class Layers
{
public:

	Layers();
	~Layers();

	void SetLayerName(uint layerNumber, const char* layerName) const;

	const char* NumberToName(uint layerNumber) const;
	bool NumberToBuiltin(uint layerNumber) const;
	int NameToNumber(const char* layerName) const;
	bool NameToBuiltin(const char* layerName) const;

	int GetMask(std::vector<uint> layerNumbers) const;
	int GetMask(std::vector<const char*> layerNames) const;

	std::vector<Layer*> GetLayers() const;

private:

	std::vector<Layer*> layers;
};

#endif