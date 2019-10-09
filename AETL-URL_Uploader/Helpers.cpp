#include "Helpers.h"
#include <string>
#include <filesystem>

const std::string GetAbsoluteDirectory(std::string Directory)
{
	return std::filesystem::absolute(Directory).string();
}