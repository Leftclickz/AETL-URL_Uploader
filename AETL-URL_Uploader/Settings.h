#pragma once
#include <string>

using namespace std;

enum class RunVersion
{
	NORMAL,
	TESTING
};

enum class RunLength
{
	NORMAL,
	RUN_ONCE
};

struct Settings
{
	static string HotFolder;
	static string AuthKey;
	static RunVersion Version;
	static RunLength Length;
};

