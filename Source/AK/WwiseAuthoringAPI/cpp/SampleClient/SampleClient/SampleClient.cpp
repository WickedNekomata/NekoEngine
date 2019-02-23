/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Version: v2017.2.6  Build: 6636
Copyright (c) 2006-2018 Audiokinetic Inc.
*******************************************************************************/

#include <iostream>

#include "TestClient.h"
#include "StressTest.h"
#include "TestErrors.h"

int main(int argc, char** argv)
{
	std::string testToRun;

	if (argc < 2)
	{
		testToRun = "HelloWorld";
	}
	else
	{
		testToRun = argv[1];
	}

	if (testToRun == "HelloWorld")
	{
		HelloWorld();
	}
	else if (testToRun == "TestWampClient")
	{
		TestWampClient();
	}
	else if (testToRun == "TestWampClientJsonString")
	{
		TestWampClientJsonString();
	}
	else if (testToRun == "TestErrors")
	{
		TestErrors();
	}
	else if (testToRun == "PerfTest")
	{
		PerfTest();
	}
	else if (testToRun == "StressTest")
	{
		StressTest();
	}
	else
	{
		std::cout << "Usage: SampleClient [HelloWorld|TestWampClient|TestWampClientJsonString]" << std::endl;
		return 1;
	}

	return 0;
}
