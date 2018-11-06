#include <assert.h>

class GameObject;

struct CurrentSelection
{
	enum class SelectedType { null, gameObject, meta };

private:
	void* cur = nullptr;
	SelectedType type = SelectedType::null;

public:
	void* Get() const
	{
		return cur;
	}

	SelectedType GetType() const
	{
		return type;
	}

	CurrentSelection& operator=(SelectedType newSelection)
	{
		assert(newSelection == SelectedType::null && "Impossible comparasion");
		type = SelectedType::null;
		cur = nullptr;
		return *this;
	}

	bool operator==(const SelectedType rhs)
	{
		return type == rhs;
	}

	CurrentSelection& operator=(GameObject* newSelection)
	{
		assert(newSelection != nullptr && "Non valid setter. Set to SelectedType::null instead");
		cur = (void*)newSelection;
		type = SelectedType::gameObject;
		return *this;
	}

	bool operator==(const GameObject* rhs)
	{
		return cur == rhs;
	}

	// Add operators in case of new kinds of selection :)
};
