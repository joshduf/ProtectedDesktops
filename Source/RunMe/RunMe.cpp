/*
ProtectedDesktops:
A program that uses native Windows features to create a fully sandboxed environment on Windows.

This program was written as part of an undergraduate research project
presented at the Honors Transfer Council of California, 26 Mar. 2016.

Copyright 2016 Joshua DuFault


This file is part of ProtectedDesktops.

ProtectedDesktops is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

ProtectedDesktops is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ProtectedDesktops.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// Using std MIN and MAX definitions rather than windows.h ones
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Sandbox.h"
using namespace std;


// Seperate launcher is a workaround in Windows 8 & 10
// Launching console program creates conhost.exe as a child process which prevents SetThreadDesktop() from working
// Using a launcher keeps conhost.exe attached to the launcher
int wmain()
{
	PROCESS_INFORMATION processInfo = { 0 };
	STARTUPINFOW processSettings = { 0 };
	processSettings.cb = sizeof(STARTUPINFOW);

	int choice = 0;

	cout << "Protected Desktops: A Sandboxing Program" << endl;

	do
	{
		cout << '\n'
			<< "Pick an option:" << '\n'
			<< "1: Create Sandbox" << '\n'
			<< "2: Switch desktops" << '\n'
			<< "3: Close Sandbox (forcefully closes any open programs)" << '\n'
			<< "4: Exit" << '\n'
			<< "Selection: "
			<< flush;

		cin >> choice;

		if (cin.fail())
		{
			cin.clear();
			choice = 0;
		}

		cin.ignore(numeric_limits<streamsize>::max(), L'\n');

		wstring process;
		if (choice == 1)
			process = L"ProtectedDesktops.exe";
		else if (choice == 2)
			process = L"DesktopSwitcher.exe";
		else if (choice == 3)
			process = L"CloseSandbox.bat";
		else if (choice == 0 || choice == 4)
			continue;


		if (CreateProcess(process.c_str(), NULL, NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &processSettings, &processInfo))
		{
			cout << "Working..." << endl;
			WaitForSingleObject(processInfo.hProcess, 60000);
		}
		else
			cout << '\n' << "Error. Try again." << '\n';

	} while (choice != 4);

	return 0;
}
