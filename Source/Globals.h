#pragma once

// Warning disabled ---
#pragma warning( disable : 4577 ) // Warning that exceptions are disabled
#pragma warning( disable : 4530 ) // Warning that exceptions are disabled

#include <windows.h>
#include <stdio.h>
#include "parson/parson.h"

enum class LogTypes
{
	Normal,
	Warning,
	Error
};

#define DEPRECATED_LOG(format, ...) Log(__FILE__, __LINE__, LogTypes::Normal, format, __VA_ARGS__);
#define CONSOLE_LOG(mode, format, ...) Log(__FILE__, __LINE__, mode, format, __VA_ARGS__);

void Log(const char file[], int line, LogTypes mode, const char* format, ...);

void OpenInBrowser(char* url);

void OpenInExplorer();

void SaveNumberArray(JSON_Object * parent, char * name, float * numbers, int size);

#define CAP(n) ((n <= 0.0f) ? n=0.0f : (n >= 1.0f) ? n=1.0f : n=n)

#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f
#define PI 3.14159265358979323846264338327950288

#define MSTOSECONDS 0.001

#define RELEASE(x) \
    { \
    if (x != nullptr) \
      delete x; \
	x = nullptr; \
	} \

#define RELEASE_ARRAY(x) \
    { \
    if (x != nullptr) \
      delete[] x; \
	x = nullptr; \
	} \

typedef unsigned int uint;
typedef unsigned char uchar;

enum update_status
{
	UPDATE_CONTINUE = 1,
	UPDATE_STOP,
	UPDATE_ERROR
};

#define MAX_BUF_SIZE 4096
#define OPEN_GL_BUF_SIZE 512
#define DEFAULT_BUF_SIZE 256
#define INPUT_BUF_SIZE 128

#define EXTENSION_MESH ".nekoMesh"
#define EXTENSION_TEXTURE ".nekoDDS"
#define EXTENSION_BONE ".nekoBone"
#define EXTENSION_ANIMATION ".nekoAnimation"
#define EXTENSION_SCENE ".nekoScene"
#define EXTENSION_VERTEX_SHADER_OBJECT ".vsh"
#define EXTENSION_FRAGMENT_SHADER_OBJECT ".fsh"
#define EXTENSION_SHADER_PROGRAM ".psh"
#define EXTENSION_META ".meta"
#define EXTENSION_SCRIPT ".dll"