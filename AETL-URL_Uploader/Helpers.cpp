#include "Helpers.h"
#include "kguithread.h"
#include "Settings.h"
#include "sqllite/SQL_Helpers.h"
#include <windows.h>
#include <string>

using namespace std;

#define DEFAULT_CURL_COMMAND string("curl -X PUT -H \"Authorization: " + Settings::AuthKey + "\" -F \"resolution=720p\" -F \"type=MONTHLY\" -F \"year=2019\" -F \"month=11\" -F \"source_file=@MyFile.mp4\" https://ceeplayer.net/v1/locations/88/timelapses/upload/")
#define DEFAULT_TEST_COMMAND string("curl -i -X PUT -H \"Authorization: NYEZAtUJMD1rZ4jAtCgDXFBIO7F4n072kJKDavd5btw\" -F \"resolution=720p\" -F \"type=MONTHLY\" -F \"year=2019\" -F \"month=11\" -F \"source_file=@MyFile.mp4\" https://ceeplayer-testing.ulam.io/v1/locations/1965/timelapses/upload/")
#define DEFAULT_RES "resolution=720p"
#define DEFAULT_TYPE "type=MONTHLY"
#define DEFAULT_YEAR "year=2019"
#define DEFAULT_MONTH "month=11"
#define DEFAULT_FILENAME "source_file=@MyFile.mp4"
#define DEFAULT_URL "https://ceeplayer.net/v1/locations/88/timelapses/upload/"
#define DEFAULT_LOCATIONID "88"
#define DEFAULT_CURL_PATH "curl"

//this covers both 200 and 201 which are successful response codes
#define CLEAN_RESPONSE_CODE "HTTP/2 20"
#define ERROR_BAD_HTTP_RESPONSE -1

struct UploadParams
{
	string Type = "type=";
	string Res = "resolution=";
	string Month = "month=";
	string Year = "year=";
	string File = "source_file=@";
	string URL = DEFAULT_URL;
};

const std::string GetAbsoluteDirectory(std::string Directory)
{
	return std::filesystem::absolute(Directory).string();
}

void UploadUsingCurl(std::filesystem::path Filepath)
{
	
	string fullFilepath = GetAbsoluteDirectory(Filepath.string());

	//Our parameters for upload
	UploadParams params;
	params.File += GetAbsoluteDirectory(Filepath.string());

	//get resolution from folder name
	string resolution = Filepath.parent_path().string();
	SQL::FindAndReplaceAll(resolution, Settings::HotFolder + "\\", "");
	params.Res += resolution;

	//get our filename and a copy stripped of the project ID
	Filepath.replace_extension("");
	string name = Filepath.filename().string();

	//This is simply safety in case a Thumbs.db sneaks its way in here.
	if (name == "Thumbs")
	{
		return;
	}

	string nameWithoutProjectID = name.substr(name.find("-") + 1);

	
	string locationID = "";
	
	//Get params based on filename being monthly or not
	if (name.find("MONTHLY") == string::npos)
		//FULL files have the structure {PROJECT_ID}-{LOCATION_ID}.mp4
	{
		params.Type += "FULL";
		string date = SQL::CurrentDateTime();//since it's a full timelapse we'll just send it to the current month it is and year
		params.Year += date.substr(0, 4);
		params.Month += date.substr(5, 2);
		SQL::FindAndReplaceAll(params.URL, DEFAULT_LOCATIONID, nameWithoutProjectID);
	}
	else
		//MONTHLY files have the structure {PROJECT_ID}-{LOCATION_ID}-MONTHLY-{YEAR}-{MONTH}.mp4
	{
		params.Type += "MONTHLY";
		params.Year += name.substr(name.find("MONTHLY-") + 8, 4);
		params.Month += name.substr(name.find("MONTHLY-") + 13, 2);
		SQL::FindAndReplaceAll(params.URL, DEFAULT_LOCATIONID, nameWithoutProjectID.substr(0, nameWithoutProjectID.find("-")));//copy the entirety of the location ID into the URL
	}
	
	//the default command and path to the curl exe
	string command;
	switch (Settings::Version)
	{
	case RunVersion::NORMAL:
		command = DEFAULT_CURL_COMMAND;
		break;
	case RunVersion::TESTING:
		command = DEFAULT_TEST_COMMAND;
		break;
	default:
		cout << "Massive error. Unknown version." << endl;
		return;
	}

	//Replace the default vals with the one of our file to upload
	string absCurlPath = "\"" + GetAbsoluteDirectory(CURL_EXE) + "\"";
	SQL::FindAndReplaceAll(command, DEFAULT_CURL_PATH, absCurlPath);
	SQL::FindAndReplaceAll(command, DEFAULT_RES, params.Res);
	SQL::FindAndReplaceAll(command, DEFAULT_FILENAME, params.File);
	SQL::FindAndReplaceAll(command, DEFAULT_MONTH, params.Month);
	SQL::FindAndReplaceAll(command, DEFAULT_YEAR, params.Year);
	SQL::FindAndReplaceAll(command, DEFAULT_TYPE, params.Type);

	//We only change the URL if we're in production mode
	if (Settings::Version != RunVersion::TESTING)
	{
		SQL::FindAndReplaceAll(command, DEFAULT_URL, params.URL);
	}

	cout << "Uploading file - " << Filepath << endl;

	//------------- EXECUTION BEGIN ---------------

	//convert to wide format and execute the command
	wstring executeString(command.begin(), command.end());
	CStringA res = ExecCmd(executeString.c_str());

	//Check if return code was a success or not
	if (res.Find(CLEAN_RESPONSE_CODE) == ERROR_BAD_HTTP_RESPONSE)
	{
		cout << "Upload failed. Full response: "  + res << endl;
	}
	else
	{
		cout << "Upload successful." << endl;

		//remove the file since we successfully uploaded it
		if (remove(fullFilepath.c_str()) == 0)
		{
			cout << params.File + ": successfully deleted from system.";
		}
		else
		{
			cout << params.File + ": failed to delete from system.";
		}
	}
	//------------- EXECUTION END --------------------
}


CStringA ExecCmd(const wchar_t* cmd)
{
	CStringA strResult;
	HANDLE hPipeRead, hPipeWrite;

	SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
	saAttr.bInheritHandle = TRUE; // Pipe handles are inherited by child process.
	saAttr.lpSecurityDescriptor = NULL;

	// Create a pipe to get results from child's stdout.
	if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0))
		return strResult;

	STARTUPINFOW si = { sizeof(STARTUPINFOW) };
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.hStdOutput = hPipeWrite;
	si.hStdError = hPipeWrite;
	si.wShowWindow = SW_HIDE; // Prevents cmd window from flashing.
							  // Requires STARTF_USESHOWWINDOW in dwFlags.

	PROCESS_INFORMATION pi = { 0 };

	BOOL fSuccess = CreateProcessW(NULL, (LPWSTR)cmd, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
	if (!fSuccess)
	{
		CloseHandle(hPipeWrite);
		CloseHandle(hPipeRead);
		return strResult;
	}

	bool bProcessEnded = false;
	for (; !bProcessEnded;)
	{
		// Give some timeslice (50 ms), so we won't waste 100% CPU.
		bProcessEnded = WaitForSingleObject(pi.hProcess, 50) == WAIT_OBJECT_0;

		// Even if process exited - we continue reading, if
		// there is some data available over pipe.
		for (;;)
		{
			char buf[1024];
			DWORD dwRead = 0;
			DWORD dwAvail = 0;

			if (!::PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwAvail, NULL))
				break;

			if (!dwAvail) // No data available, return
				break;

			if (!::ReadFile(hPipeRead, buf, min(sizeof(buf) - 1, dwAvail), &dwRead, NULL) || !dwRead)
				// Error, the child process might ended
				break;

			buf[dwRead] = 0;
			strResult += buf;
		}
	} //for

	CloseHandle(hPipeWrite);
	CloseHandle(hPipeRead);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return strResult;
}
