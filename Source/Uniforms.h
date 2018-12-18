#ifndef __UNIFORMS_H__
#define __UNIFORMS_H__

#include <stdint.h>
#include "globals.h"

enum Uniforms_Values { FloatU_value = 0x1406,	  Vec2FU_value = 0x8B50,	 Vec3FU_value = 0x8B51,		Vec4FU_value = 0x8B52,
					   FloatMAT2U_value = 0x8B5A, FloatMAT3U_value = 0x8B5B, FloatMAT4U_value = 0x8B5C,
					   IntU_value = 0x1404,		  Vec2IU_value = 0x8B53,	 Vec3IU_value = 0x8B54,		Vec4IU_value = 0x8B55,
					   BoolU_value = 0x8B56,	  Vec2BU_value = 0x8B58,	 Vec3BU_value = 0x8B58,		Vec4BU_value = 0x8B59,
					   SamplerU_value = 0x8B5D,	  Sampler2U_value = 0x8B5E,  Sampler3U_value = 0x8B5F,  SamplerCU_value = 0x8B60,
};

struct IntU
{
	char		name[DEFAULT_BUF_SIZE];
	int			type;
	uint32_t	value;
};

struct FloatU
{
	char		name[DEFAULT_BUF_SIZE];
	int			type;
	float		value;
};

struct Vec2FU
{
	struct VecF2
	{
		float x;
		float y;
	};

	char		name[DEFAULT_BUF_SIZE];
	int			type;
	VecF2		value;
};

struct Vec3FU
{
	struct VecF3
	{
		float x;
		float y;
		float z;
	};

	char		name[DEFAULT_BUF_SIZE];
	int			type;
	VecF3		value;
};

struct Vec4FU
{
	struct VecF4
	{
		float x;
		float y;
		float z;
		float w;
	};

	char		name[DEFAULT_BUF_SIZE];
	int			type;
	VecF4		value;
};

struct Vec2IU
{
	struct VecI2
	{
		int x;
		int y;
	};

	char		name[DEFAULT_BUF_SIZE];
	int			type;
	VecI2		value;
};

struct Vec3IU
{
	struct VecI3
	{
		int x;
		int y;
		int z;
	};

	char		name[DEFAULT_BUF_SIZE];
	int			type;
	VecI3		value;
};

struct Vec4IU
{
	struct VecI4
	{
		int x;
		int y;
		int z;
		int w;
	};

	char		name[DEFAULT_BUF_SIZE];
	int			type;
	VecI4		value;
};

/////////////////////////////////////////////////////////

union Uniform
{
	char		name[DEFAULT_BUF_SIZE];
	uint32_t	type;

	IntU		intU;
	FloatU		floatU;
	Vec2FU		vec2FU;
	Vec3FU		vec3FU;
	Vec4FU		vec4FU;
	Vec2IU		vec2IU;
	Vec3IU		vec3IU;
	Vec4IU		vec4IU;
};

#endif