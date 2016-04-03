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

#include "Sandbox.h"
using namespace std;

// Returns 0 if sandbox created successfully
// Returns 1 if sandbox creation failed
int wmain()
{
	bool sandboxFail = true;

	SECURITY_ATTRIBUTES adminOnlyDACL;
	// Creates custom DACL
	// Sets owner, denies, and allows
	// Sandbox relies on DACLs, no point in continuing if they're not usable
	if (!createDACL(&adminOnlyDACL))
		return sandboxFail;

	HDESK adminOnlyHandle = NULL;
	// Creates desktop with specified DACL
	// If desktop already exists, gets handle for desktop
	// Desktop handles needed to access them, no point in continuing without handles
	if (!opener(ADMIN, &adminOnlyDACL, adminOnlyHandle))
		return sandboxFail;

	// Sets DACL in case objects already existed
	// If either failed, can't be sure desktops have secure settings
	// Sets DACL denying main user, makes DACL propigate to children
	// Sets DACL for explorer.exe on main desktop denying sandbox user
	if (!setDACL(adminOnlyDACL, adminOnlyHandle, SE_WINDOW_OBJECT))
		return sandboxFail;

	// Clean up no longer needed data
	// Use LocalFree to free the returned buffer created by
	//	ConvertStringSecurityDescriptorToSecurityDescriptor()
	//	on successful completion of createDACL()
	LocalFree(adminOnlyDACL.lpSecurityDescriptor);

	cout << "Working..." << endl;

	// Launch next part
	if (launchProcessOnDesktop(CONSOLEAPP, ADMIN, true))
		sandboxFail = false;

	// Handles not needed
	CloseDesktop(adminOnlyHandle);

	return sandboxFail;
}
