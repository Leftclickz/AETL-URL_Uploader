#include <conio.h>
#include <iostream>

#include "LogFile.h"
#include "Helpers.h"
#include "sqllite/sqlite3.h"

using namespace std;


volatile bool IsRunning = true;
static sqlite3* OUR_DATABASE;

//Listen for a character input to exit
void ListenForExit();

int main()
{
	return 0;

	LogFile::BeginLogging();

#if (_DEBUG)
	std::thread InputListener(ListenForExit);
#elif (!_DEBUG)
	std::thread* InputListener = new std::thread(ListenForExit);
	InputListener->detach();
#endif


	while (IsRunning)
	{
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