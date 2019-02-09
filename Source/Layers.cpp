#include "Layers.h"

#include "Application.h"
#include "EventSystem.h"

#include <assert.h>

Layer::Layer() {}

Layer::Layer(uint number) :number(number) {}

Layer::~Layer() {}

uint Layer::GetNumber() const
{
	return number;
}

uint Layer::GetFilterGroup() const
{
	return BIT_SHIFT(number);
}

void Layer::SetFilterMask(uint filterMask)
{
	this->filterMask = filterMask;

	if (App != nullptr)
	{
		System_Event newEvent;
		newEvent.type = System_Event_Type::LayerFilterMaskChanged;
		newEvent.layerEvent.layer = number;
		App->PushSystemEvent(newEvent);
	}
}

uint Layer::GetFilterMask() const
{
	return filterMask;
}

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

Layers::Layers() 
{
	layers.reserve(MAX_NUM_LAYERS);
	layers.shrink_to_fit();

	for (uint i = 0; i < MAX_NUM_LAYERS; ++i)
	{
		layers.push_back(new Layer(i));
		layers[i]->SetFilterMask(DEFAULT_FILTER_MASK);
	}

	layers[0]->name = "Default";
	layers[0]->builtin = true;
}

Layers::~Layers() {}

bool Layers::Init(JSON_Object * jObject)
{
	return true;
}

bool Layers::Start()
{
	return true;
}

bool Layers::CleanUp()
{
	return true;
}

// ----------------------------------------------------------------------------------------------------

void Layers::SetLayerName(uint layerNumber, const char* layerName) const
{
	assert(layerName != nullptr
		&& layerNumber >= 0 && layerNumber < MAX_NUM_LAYERS);

	if (strcmp(layerName, "") == 0)
	{
		System_Event newEvent;
		newEvent.type = System_Event_Type::LayerNameReset;
		newEvent.layerEvent.layer = layerNumber;
		App->PushSystemEvent(newEvent);
	}
	else if (NameToNumber(layerName) > -1)
		CONSOLE_LOG(LogTypes::Warning, "A layer with the name '%s' is already registered", layerName);

	layers[layerNumber]->name = layerName;
}

// Returns the name of the layer. "" means no name
const char* Layers::NumberToName(uint layerNumber) const
{
	assert(layerNumber >= 0 && layerNumber < MAX_NUM_LAYERS);
	return layers[layerNumber]->name.data();
}

// Returns the number of the layer. -1 if error
int Layers::NameToNumber(const char* layerName) const
{
	assert(layerName != nullptr);
	for (uint i = 0; i < layers.size(); ++i)
	{
		if (strcmp(layers[i]->name.data(), layerName) == 0)
			return i;
	}

	return -1;
}

// ----------------------------------------------------------------------------------------------------

Layer* Layers::GetLayer(uint layerNumber) const
{
	assert(layerNumber >= 0 && layerNumber < MAX_NUM_LAYERS);
	return layers[layerNumber];
}

std::vector<Layer*> Layers::GetLayers() const
{
	return layers;
}
