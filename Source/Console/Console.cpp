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

// Returns 0 if sandbox created successfully
// Returns 1 if sandbox creation failed
int wmain()
{
	bool sandboxFail = true;

	wstring userMain, userSandbox, fileManagerPath;
	// Reads in settings information from file
	// No point in continuing without usable settings
	if (!getSettings(userMain, userSandbox, fileManagerPath))
		return sandboxFail;

	SECURITY_ATTRIBUTES defaultDACL, sandboxDACL;
//	SECURITY_ATTRIBUTES WinSta0DACL; // Code to set window station DACLs left in for potential future use
	// Creates custom DACL
	// Sets owner, denies, and allows
	// Sandbox relies on DACLs, no point in continuing if they're not usable
	if (!createDACL(userMain, &defaultDACL, userSandbox)
		|| !createDACL(userSandbox, &sandboxDACL, userMain))
//		|| !createDACL(userMain, &WinSta0DACL, userSandbox))
		return sandboxFail;

	HDESK defaultHandle = NULL, sandboxHandle = NULL;
//	HWINSTA WinSta0Handle = NULL;
	// Creates desktop with specified DACL
	// If desktop already exists, gets handle for desktop
	// Desktop handles needed to access them, no point in continuing without handles
	if (!opener(MAIN, &defaultDACL, defaultHandle)
		|| !opener(SANDBOX, &sandboxDACL, sandboxHandle))
//		|| !opener(WINDOWSTATIONMAIN, &WinSta0DACL, WinSta0Handle))
		return sandboxFail;

	// Need more power to set DACL for window station
//	SetPrivilege(SE_DEBUG_NAME, TRUE);

	// Sets DACL in case objects already existed
	// If either failed, can't be sure desktops have secure settings
	// Sets DACL denying main user, makes DACL propigate to children
	// Sets DACL for explorer.exe on main desktop denying sandbox user
	if (!setDACL(defaultDACL, defaultHandle, SE_WINDOW_OBJECT)
		|| !setDACL(sandboxDACL, sandboxHandle, SE_WINDOW_OBJECT))
//		|| !setDACL(WinSta0DACL, WinSta0Handle, SE_WINDOW_OBJECT))
		return sandboxFail;

	// Revert privilege
//	SetPrivilege(SE_DEBUG_NAME, FALSE);

	// Clean up no longer needed data
	// Use LocalFree to free the returned buffer created by
	//	ConvertStringSecurityDescriptorToSecurityDescriptor()
	//	on successful completion of createDACL()
	LocalFree(defaultDACL.lpSecurityDescriptor);
	LocalFree(sandboxDACL.lpSecurityDescriptor);
	//	LocalFree(WinSta0DACL.lpSecurityDescriptor);

	// Don't echo password back
	if (!keyboardEchoOff())
	{
		switchTo(MAIN);
		return sandboxFail;
	}

	// Launch file manager on main desktop with default DACL
	//	then jump to sandboxed desktop to launch sandboxed file manager
	if (launchFileManager(userMain, userSandbox, fileManagerPath, MAIN, true)
		&& launchFileManager(userSandbox, userMain, fileManagerPath, SANDBOX, false))
	{
		switchTo(ADMIN);
		wcout << L"Sandbox created successfully." << L"\n\n"
			<< L"Press Enter to exit: ";
		cin.ignore(numeric_limits<streamsize>::max(), L'\n');
		sandboxFail = false;
	}

	// End by going back home
	switchTo(MAIN);

	// Handles not needed
	CloseDesktop(defaultHandle);
	CloseDesktop(sandboxHandle);
//	CloseWindowStation(WinSta0Handle);

	return sandboxFail;
}
