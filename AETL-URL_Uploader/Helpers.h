#pragma once

#include <thread>
#include <iostream>
#include <filesystem>
#include <string>
#include <atlstr.h>

#define SECOND 1000
#define SLEEP(_VAL) std::this_thread::sleep_for(std::chrono::milliseconds(_VAL))

#define CURL_EXE std::string("curl\\curl.exe")

const std::string GetAbsoluteDirectory(std::string Directory);

//process for uploading a file to the endpoint
void UploadUsingCurl(std::filesystem::path Filepath);

//Run a console command explicitly with a pipeline to handle return output in wchar format
CStringA ExecCmd(const wchar_t* cmd);

