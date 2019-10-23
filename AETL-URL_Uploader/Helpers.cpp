#include "Helpers.h"
#include "kguithread.h"
#include "Settings.h"
#include "sqllite/SQL_Helpers.h"
#include <string>

using namespace std;

#define DEFAULT_CURL_COMMAND string("curl - H \"Authorization: " + Settings::AuthKey + "\" - F \"resolution = 720p\" - F \"type=MONTHLY\" - F \"year=2019\" - F \"month=11\" - F \"source_file=@MyFile.mp4\" https://ceeplayer.net/v1/locations/88/timelapses/upload/")
#define DEFAULT_RES "resolution = 720p"
#define DEFAULT_TYPE "type=MONTHLY"
#define DEFAULT_YEAR "year=2019"
#define DEFAULT_MONTH "month=11"
#define DEFAULT_FILENAME "source_file=@MyFile.mp4"
#define DEFAULT_URL "https://ceeplayer.net/v1/locations/88/timelapses/upload/"

struct UploadParams
{
	string Type = "type=";
	string Res = "resolution = ";
	string Month = "month=";
	string Year = "year=";
	string File = "source_file=";
};

const std::string GetAbsoluteDirectory(std::string Directory)
{
	return std::filesystem::absolute(Directory).string();
}

void UploadUsingCurl(std::filesystem::path Filepath)
{
	cout << "Uploading file - " << Filepath << endl;

	UploadParams params;

	string dir = Filepath.parent_path().string();
	SQL::FindAndReplaceAll(dir, Settings::HotFolder + "\\", "");

	string name = Filepath.filename().string();

	//Get type
	if (name.find("MONTHLY") == string::npos)
	{
		params.Type += "FULL";
		string date = SQL::CurrentDateTime();
		params.Year += date.substr(0, 3);
		params.Month += date.substr(5, 6);
	}
	else
	{
		params.Type += "MONTHLY";
		params.Year += name.substr(name.find("MONTHLY-") + 8, 4);
		params.Month += name.substr(name.find("MONTHLY-") + 13, 2);
	}
	
	params.Res = dir;

	

	string command = DEFAULT_CURL_COMMAND;
	cout << command << endl;

	//run our command on a handle
	kGUICallThread ProcessHandle;
	ProcessHandle.Start(command.c_str(), CALLTHREAD_READ);

	//get the curl output
	std::string out = *ProcessHandle.GetString();
	cout << out;
	ProcessHandle.Stop();
}
