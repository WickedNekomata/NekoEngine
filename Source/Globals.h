#pragma once

// Warning disabled ---
#pragma warning( disable : 4577 ) // Warning that exceptions are disabled
#pragma warning( disable : 4530 ) // Warning that exceptions are disabled

#include <windows.h>
#include <stdio.h>

#define CONSOLE_LOG(format, ...) Log(__FILE__, __LINE__, format, __VA_ARGS__);

void Log(const char file[], int line, const char* format, ...);

void OpenInBrowser(const char* url);

void OpenInExplorer();

#define CAP(n) ((n <= 0.0f) ? n=0.0f : (n >= 1.0f) ? n=1.0f : n=n)

#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f

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

enum update_status
{
	UPDATE_CONTINUE = 1,
	UPDATE_STOP,
	UPDATE_ERROR
};

#define BUF_SIZE 4096
#define STR_INPUT_SIZE 128