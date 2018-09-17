
#pragma once

struct Color
{
	float r, g, b, a;
	
	Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f)
	{}

	Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a)
	{}

	void Set(float r, float g, float b, float a = 1.0f)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	float* operator & ()
	{
		return (float*)this;
	}
};

extern Color Red;
extern Color Green;
extern Color Blue;
extern Color Black;
extern Color White;