#include "ModuleLayers.h"

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

ModuleLayers::ModuleLayers(bool start_enabled) : Module(start_enabled)
{
	name = "Layers";

	layers.reserve(MAX_NUM_LAYERS);
	layers.shrink_to_fit();

	for (uint i = 0; i < MAX_NUM_LAYERS; ++i)
	{
		layers.push_back(new Layer(i));
		layers[i]->SetFilterMask(DEFAULT_FILTER_MASK);
	}

	layers[0]->name = "Default";
	layers[0]->builtin = true;

	layers[UILAYER]->name = "UI";
	layers[UILAYER]->builtin = true;
}

ModuleLayers::~ModuleLayers() {}

bool ModuleLayers::Init(JSON_Object* jObject)
{
	LoadStatus(jObject);

	return true;
}

void ModuleLayers::SaveStatus(JSON_Object* jObject) const
{
	char layerName[DEFAULT_BUF_SIZE];
	for (uint i = 0; i < MAX_NUM_LAYERS; ++i)
	{
		if (layers[i]->builtin || strcmp(layers[i]->name.data(), "") == 0)
			continue;

		JSON_Value* layerValue = json_value_init_object();
		JSON_Object* layerObject = json_value_get_object(layerValue);
		sprintf_s(layerName, DEFAULT_BUF_SIZE, "Layer %i", layers[i]->GetNumber());
		json_object_set_value(jObject, layerName, layerValue);

		json_object_set_string(layerObject, "Name", layers[i]->name.data());
		json_object_set_number(layerObject, "Filter mask", layers[i]->GetFilterMask());
	}
}

void ModuleLayers::LoadStatus(const JSON_Object* jObject)
{
	if (jObject == nullptr)
		return;

	char layerName[DEFAULT_BUF_SIZE];
	for (uint i = 0; i < MAX_NUM_LAYERS; ++i)
	{
		if (layers[i]->builtin)
			continue;

		sprintf_s(layerName, DEFAULT_BUF_SIZE, "Layer %i", layers[i]->GetNumber());
		JSON_Object* layerObject = json_object_get_object(jObject, layerName);
		if (layerObject == nullptr)
			continue;

		const char* name = json_object_get_string(layerObject, "Name");
		if (name == nullptr || strcmp(name, "") == 0)
			continue;
		layers[i]->name = name;

		layers[i]->SetFilterMask(json_object_get_number(layerObject, "Filter mask"));
	}
}

// ----------------------------------------------------------------------------------------------------

void ModuleLayers::SetLayerName(uint layerNumber, const char* layerName) const
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
const char* ModuleLayers::NumberToName(uint layerNumber) const
{
	assert(layerNumber >= 0 && layerNumber < MAX_NUM_LAYERS);
	return layers[layerNumber]->name.data();
}

// Returns the number of the layer. -1 if error
int ModuleLayers::NameToNumber(const char* layerName) const
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

Layer* ModuleLayers::GetLayer(uint layerNumber) const
{
	assert(layerNumber >= 0 && layerNumber < MAX_NUM_LAYERS);
	return layers[layerNumber];
}

std::vector<Layer*> ModuleLayers::GetLayers() const
{
	return layers;
}