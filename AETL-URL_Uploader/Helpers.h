#pragma once

#include <thread>
#include <iostream>

#define SECOND 1000
#define SLEEP(_VAL) std::this_thread::sleep_for(std::chrono::milliseconds(_VAL))

const std::string GetAbsoluteDirectory(std::string Directory);

