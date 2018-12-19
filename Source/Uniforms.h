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
	uint32_t	type;
	char		name[DEFAULT_BUF_SIZE];
	uint32_t	location;
	uint32_t	value;
};

struct FloatU
{
	uint32_t	type;
	char		name[DEFAULT_BUF_SIZE];
	uint32_t	location;
	float		value;
};

struct Vec2FU
{
	struct VecF2
	{
		float x;
		float y;
	};

	uint32_t	type;
	char		name[DEFAULT_BUF_SIZE];
	uint32_t	location;
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

	uint32_t	type;
	char		name[DEFAULT_BUF_SIZE];
	uint32_t	location;
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

	uint32_t	type;
	char		name[DEFAULT_BUF_SIZE];
	uint32_t	location;
	VecF4		value;
};

struct Vec2IU
{
	struct VecI2
	{
		uint32_t x;
		uint32_t y;
	};

	uint32_t	type;
	char		name[DEFAULT_BUF_SIZE];
	uint32_t	location;
	VecI2		value;
};

struct Vec3IU
{
	struct VecI3
	{
		uint32_t x;
		uint32_t y;
		uint32_t z;
	};

	uint32_t	type;
	char		name[DEFAULT_BUF_SIZE];
	uint32_t	location;
	VecI3		value;
};

struct Vec4IU
{
	struct VecI4
	{
		uint32_t x;
		uint32_t y;
		uint32_t z;
		uint32_t w;
	};

	uint32_t	type;
	char		name[DEFAULT_BUF_SIZE];
	uint32_t	location;
	VecI4		value;
};

/////////////////////////////////////////////////////////

struct CommonData
{
	uint32_t	type;
	char		name[DEFAULT_BUF_SIZE];
	uint32_t	location;
};

union Uniform
{
	CommonData	common;

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