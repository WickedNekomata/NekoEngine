#include "Layers.h"

#include <assert.h>

#define BIT_SHIFT(x) 1 << x

Layer::Layer() {}

Layer::Layer(uint number) :number(number) {}

Layer::~Layer() {}

uint Layer::GetNumber() const
{
	return number;
}

uint Layer::GetValue() const
{
	return BIT_SHIFT(number);
}

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

Layers::Layers() 
{
	layers.reserve(MAX_NUM_LAYERS);
	layers.shrink_to_fit();

	for (uint i = 0; i < MAX_NUM_LAYERS; ++i)
		layers.push_back(new Layer(i));

	layers[0]->name = "Default";
	layers[0]->builtin = true;
}

Layers::~Layers() {}

void Layers::SetLayerName(uint layerNumber, const char* layerName) const
{
	if (layers[layerNumber]->name.data() != nullptr)
		CONSOLE_LOG("Warning! A layer with the name '%s' is already registered", layerName);

	layers[layerNumber]->name = layerName;
}

// Returns the name of the layer
const char* Layers::NumberToName(uint layerNumber) const
{
	assert(layerNumber >= 0 && layerNumber < MAX_NUM_LAYERS);
	return layers[layerNumber]->name.data();
}

// Returns whether the layer is builtin
bool Layers::NumberToBuiltin(uint layerNumber) const
{
	assert(layerNumber >= 0 && layerNumber < MAX_NUM_LAYERS);
	return layers[layerNumber]->builtin;
}

// Returns the number of the layer. -1 if error
int Layers::NameToNumber(const char* layerName) const
{
	assert(layerName != nullptr);
	for (uint i = 0; i < layers.size(); ++i)
	{
		if (layers[i]->name.data() == layerName)
			return i;
	}

	return -1;
}

// Returns whether the layer is builtin. -1 if error
int Layers::NameToBuiltin(const char* layerName) const
{
	assert(layerName != nullptr);
	for (uint i = 0; i < layers.size(); ++i)
	{
		if (layers[i]->name.data() == layerName)
			return layers[i]->builtin;
	}

	return -1;
}

// Returns the mask
uint Layers::GetMask(std::vector<uint> layerNumbers) const
{
	uint mask = 0;

	for (uint i = 0; i < layerNumbers.size(); ++i)
	{
		assert(layerNumbers[i] >= 0 && layerNumbers[i] < MAX_NUM_LAYERS);
		mask |= BIT_SHIFT(layerNumbers[i]);
	}

	return mask;
}

// Returns the mask
uint Layers::GetMask(std::vector<const char*> layerNames) const
{
	uint mask = 0;

	for (uint i = 0; i < layerNames.size(); ++i)
	{
		int layerNumber = NameToNumber(layerNames[i]);
		assert(layerNumber >= 0 && layerNumber < MAX_NUM_LAYERS);
		mask |= BIT_SHIFT(layerNumber);
	}

	return mask;
}