#ifndef __LAYERS_H__
#define __LAYERS_H__

#include "Globals.h"

#include <string>
#include <vector>

constexpr uint MAX_NUM_LAYERS(32);

class Layer
{
public:

	Layer();
	Layer(uint value, std::string name);
	~Layer();

	uint GetNumber() const;
	uint GetValue() const;

public:

	std::string name;

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
	int NameToNumber(const char* layerName) const;

	uint GetMask(std::vector<uint> layerNumbers) const;
	uint GetMask(std::vector<const char*> layerNames) const;

private:

	std::vector<Layer*> layers;
};

#endif