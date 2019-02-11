#pragma once

#include "Globals.h"
#include "Application.h"

void Log(const char file[], int line, LogTypes mode, const char* format, ...)
{
	static char tmp_string[MAX_BUF_SIZE];
	static char tmp_string2[MAX_BUF_SIZE];
	static va_list  ap;

	// Construct the string from variable arguments
	va_start(ap, format);
	vsprintf_s(tmp_string, MAX_BUF_SIZE, format, ap);
	va_end(ap);
	sprintf_s(tmp_string2, MAX_BUF_SIZE, "\n%s(%d) : %s", file, line, tmp_string);
	OutputDebugString(tmp_string2);

	// Send the string to the console
	if (App != nullptr)
	{
		sprintf_s(tmp_string2, MAX_BUF_SIZE, "%s\n", tmp_string);

		switch ((LogTypes)mode)
		{
			case LogTypes::Warning:
			{
				if (strstr(tmp_string2, "Warning: ") == nullptr)
				{
					strcpy(tmp_string, tmp_string2);
					strcpy(tmp_string2, "");
					strcat(tmp_string2, "Warning: ");
					strcat(tmp_string2, tmp_string);
				}
				break;
			}
			case LogTypes::Error:
			{
				if (strstr(tmp_string2, "Error: ") == nullptr)
				{
					strcpy(tmp_string, tmp_string2);
					strcpy(tmp_string2, "");
					strcat(tmp_string2, "Error: ");
					strcat(tmp_string2, tmp_string);
				}
				break;
			}
		}

		App->LogGui(tmp_string2);
	}
}

void OpenInBrowser(char* url)
{
	ShellExecute(0, 0, url, 0, 0, SW_SHOW);
}

void OpenInExplorer()
{
	TCHAR pwd[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, pwd);
	ShellExecute(NULL, "open", pwd, NULL, NULL, SW_SHOWDEFAULT);
}

void SaveNumberArray(JSON_Object* parent, char* name, float* numbers, int size)
{
	JSON_Value* value = json_value_init_array();
	JSON_Array* arrray = json_value_get_array(value);

	json_object_set_value(parent, name, value);

	for (int i = 0; i < size; ++i)
	{
		json_array_append_number(arrray, numbers[i]);
	}
}