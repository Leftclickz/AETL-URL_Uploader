#include <conio.h>
#include <filesystem>
#include <iostream>

#include "LogFile.h"
#include "Helpers.h"
#include "sqllite/sqlite3.h"
#include "Settings.h"
//#include "curl.h"

using namespace std;
namespace fs = std::filesystem;

volatile bool IsRunning = true;
static sqlite3* OUR_DATABASE;
//static CURL* OUR_CURL_HANDLE;

//Listen for a character input to exit
void ListenForExit();
void Update();

int main(int argc, char* argv[])
{
	LogFile::BeginLogging();

	for (int i = 0; i < argc; i++)
	{
		string val = string(argv[i]);

		if (val == "-folder")
		{
			if (i + 1 >= argc)
			{
				cout << "Invalid folder setting. -folder requires a string input." << endl;
				return 0;
			}

			Settings::HotFolder = fs::absolute(string(argv[i + 1])).string();
		}
	}



#if (_DEBUG)
	std::thread InputListener(ListenForExit);
#elif (!_DEBUG)
	std::thread* InputListener = new std::thread(ListenForExit);
	InputListener->detach();
#endif

	//OUR_CURL_HANDLE = curl_easy_init();

	while (IsRunning)
	{		
		Update();
		SLEEP(SECOND * 60 * 60);
	}


#if (!_DEBUG)
	delete InputListener;
#endif

	return 0;
}

void ListenForExit()
{
	LogFile::WriteToLog("ListenForExit thread initiated. Now listening for ESC char.");


	while (true)
	{
		int val = _getch();

		if (val == 0x1B)
		{
			cout << endl << "Exit has been requested. Finishing up last directive then shutting down." << endl;
			LogFile::WriteToLog("ListenForExit: Exit has been requested. Finishing up last directive then shutting down.");
			IsRunning = false;
			break;
		}
	}
}


void Update()
{
	fs::directory_iterator hotFolderIterator = fs::directory_iterator(Settings::HotFolder);

	for (auto i : hotFolderIterator)
	{
		cout << "Searching path - " << i.path().string() << endl;

		fs::directory_iterator subFolder = fs::directory_iterator(i.path());

		for (auto entry : subFolder)
		{
			UploadUsingCurl(entry.path());
		}


		cout << endl;
	}
}