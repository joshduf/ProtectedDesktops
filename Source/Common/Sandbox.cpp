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

// Opens settings file
// gets primary user, user account & file manager to use for sandbox
// Returns true 
bool getSettings(wstring &mainUser, wstring &sandboxUser, wstring &fileManager)
{
	bool getSettingsWorked = false;

	wifstream fin;
	fin.open(SETTINGSFILE.c_str());

	if (fin)
		getline(fin, mainUser);

	if (fin)
		getline(fin, sandboxUser);

	if (fin)
		getline(fin, fileManager);

	if (fin && mainUser != L"" && sandboxUser != L"" && fileManager != L"")
		getSettingsWorked = true;
	else
		fin.clear();

	fin.close();

	return getSettingsWorked;
}

bool createDACL(SECURITY_ATTRIBUTES *newDACL)
{
	bool createDACLWorked = false;

	wstring userDenySIDString, userallowSIDString;

	// Sets owner to Builtin/Administrators
	// Denies access to userToDeny
	// Gives full control to Builtin/Administrators
	// Gives read & execute access to userToAllow
	wstring newACE;
	newACE = L"D:(A;OICI;GA;;;SY)(A;OICI;GA;;;BA)";

	// createDACLWorked is true iff denyDACL is successfully set
	if (ConvertStringSecurityDescriptorToSecurityDescriptor(newACE.c_str(), SDDL_REVISION_1, &(newDACL->lpSecurityDescriptor), NULL))
		createDACLWorked = true;

	return createDACLWorked;
}

// Adds deny ACE entry for specified user
// Returns true on successfull creation of security descriptor
// Returns false otherwise
bool createDACL(const wstring &userToAllow, SECURITY_ATTRIBUTES *newDACL, const wstring &userToDeny, bool isDesktop)
{
	bool createDACLWorked = false;

	wstring userDenySIDString, userallowSIDString;

	// Must have a successful creation of user to allow or only admin will have access
	// Must either have a successful creation of user to deny or have no user to deny
	// No point in continuing otherwise
	if (!usernameToSIDString(userToAllow, userallowSIDString)
		|| !usernameToSIDString(userToDeny, userDenySIDString))
		return createDACLWorked;

	// Sets owner to Builtin/Administrators
	// Denies access to userToDeny
	// Gives full control to Builtin/Administrators
	// Gives read & execute access to userToAllow
	wstring newACE;
	newACE = L"D:(D;OICI;GA;;;" + userDenySIDString + L")(A;OICI;GA;;;SY)(A;OICI;GA;;;BA)(A;OICI;GA;;;" + userallowSIDString + L")";

	// Gives desktop access to Modern apps
	if (isDesktop)
		newACE += L"(A;OICI;GA;;;S-1-15-2-1)";

	// createDACLWorked is true iff denyDACL is successfully set
	if (ConvertStringSecurityDescriptorToSecurityDescriptor(newACE.c_str(), SDDL_REVISION_1, &(newDACL->lpSecurityDescriptor), NULL))
		createDACLWorked = true;

	return createDACLWorked;
}

// Sets DACL and owner on object with specified handle
// Returns true on successful setting of security info
// Returns false otherwise
bool setDACL(SECURITY_ATTRIBUTES newDACL, HANDLE objectHandle, SE_OBJECT_TYPE objectType)
{
	bool setDACLWorked = false;

	BOOL existsACL = TRUE, isDefaultACL = TRUE;
	PACL DACLtoSet;
	PSID newOwner;
	ConvertStringSidToSid(L"BA", &newOwner); // SID for Builtin/Administrators

	// Sets the bit flag mask
	SECURITY_INFORMATION securityToSet;
	if (objectType == SE_KERNEL_OBJECT)
		securityToSet = OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION | LABEL_SECURITY_INFORMATION;
	else
		securityToSet = OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION;

	// Pulls the DACL out of the newDACL structure
	// Sets the DACL of the object with specified handle
	// Function is not bool, this return value means success
	if (GetSecurityDescriptorDacl(newDACL.lpSecurityDescriptor, &existsACL, &DACLtoSet, &isDefaultACL)
		&& SetSecurityInfo(objectHandle, objectType, securityToSet, newOwner, NULL, DACLtoSet, NULL) == ERROR_SUCCESS)
		setDACLWorked = true;

	return setDACLWorked;
}

bool setDefaultDACL(SECURITY_ATTRIBUTES newDACL, HANDLE &tokenHandle)
{
	bool setDefaultDACLworked = false;

	PACL sandboxACLtoSet;
	BOOL existsACL = TRUE, isDefaultACL = TRUE;

	if (GetSecurityDescriptorDacl(newDACL.lpSecurityDescriptor, &existsACL, &sandboxACLtoSet, &isDefaultACL))
	{
		TOKEN_DEFAULT_DACL defaultDACL;
		defaultDACL.DefaultDacl = sandboxACLtoSet;

		if (SetTokenInformation(tokenHandle, TokenDefaultDacl, &defaultDACL, sizeof(defaultDACL)))
			setDefaultDACLworked = true;
	}

	return setDefaultDACLworked;
}

// Creates DACL that only gives access to admin and system
// Sets userSIDString to SID wstring version of username
// Returns true on successful completion of ConvertSidToStringSid(), false otherwise
bool usernameToSIDString(const wstring &username, wstring &userSIDString)
{
	bool usernameToSIDStringWorked = false;

	SID userSID[SECURITY_MAX_SID_SIZE];
	DWORD userSIDSize = SECURITY_MAX_SID_SIZE;
	DWORD domainNameSize = 0;
	SID_NAME_USE userType;
	LPTSTR LPuserSIDString = nullptr;

	// Get size of buffer needed for domainName
	LookupAccountName(NULL, username.c_str(), &userSID, &userSIDSize, NULL, &domainNameSize, &userType);
	TCHAR *domainName = new TCHAR[domainNameSize];

	// Create wstring version of SIDS for userToDenySID and userToAllowSID
	if (LookupAccountName(NULL, username.c_str(), &userSID, &userSIDSize, domainName, &domainNameSize, &userType)
		&& ConvertSidToStringSid(&userSID, &LPuserSIDString))
	{
		userSIDString = LPuserSIDString;
		usernameToSIDStringWorked = true;
		// Needed after ConvertSidToStringSid()
		LocalFree(LPuserSIDString);
	}

	delete[] domainName;

	return usernameToSIDStringWorked;
}

// Don't want to echo back password
bool keyboardEchoOff()
{
	bool keyboardEchoOff = false;

	HANDLE consoleInputHandle = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;

	if (GetConsoleMode(consoleInputHandle, &mode) && SetConsoleMode(consoleInputHandle, mode - ENABLE_ECHO_INPUT))
		keyboardEchoOff = true;

	return keyboardEchoOff;
}

bool getCredentials(const wstring &user, wstring &userCreds)
{
	bool getCredentials = false;

	// Need credentials for sandboxed user
	wstring sandboxUserCred;
	wcout << L"Sandbox switched to secure desktop to enter credentials." << '\n' << endl;
	wcout << L"Enter password for " << user << ": ";
	if (getline(wcin, userCreds))
		getCredentials = true;
	wcout << '\n' << endl;

	return getCredentials;
}

bool launchProcessOnDesktop(wstring process, wstring desktop, bool resume)
{
	bool launchProcessOnDesktop = false;

	if (!switchTo(desktop))
		return launchProcessOnDesktop;

	// Create new process on admin only desktop to get console there
	PROCESS_INFORMATION processInfo = { 0 };
	STARTUPINFOW processSettings = { 0 };
	processSettings.cb = sizeof(STARTUPINFOW);

	vector<WCHAR> newDesktop(desktop.begin(), desktop.end());
	newDesktop.push_back(0);
	processSettings.lpDesktop = &newDesktop[0];

	if (CreateProcess(process.c_str(), NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE | CREATE_SUSPENDED, NULL, NULL, &processSettings, &processInfo))
		launchProcessOnDesktop = true;

	if (resume)
		ResumeThread(processInfo.hThread);

	// Give console.exe time to fully start up
	WaitForSingleObject(processInfo.hProcess, 60000);

	return launchProcessOnDesktop;
}

// Function that launches file manager with impersonation token
// Returns true on successful completion
// Returns false otherwise
bool launchFileManager(const wstring &userToAllow, const wstring &userToDeny, const wstring &fileManager, const wstring &desktop, bool isExplorer)
{
	bool launchFileManagerWorked = false;

	wstring sandboxUserCred;

	// Switch to admin only desktop to get password
	// Get a console on new desktop
	// Get password
	switchTo(ADMIN);
	if (!getCredentials(userToAllow, sandboxUserCred))
		return launchFileManagerWorked;

	// Unused Code to set desktop in STARTUPINFOW, here for documentation
	// Undocumented feature of CreateProcessWithLogonW() causes it to assing new logon SID
	//	when desktop is specified, causing GUI problems
	//	vector<WCHAR> newDesktop(desktop.begin(), desktop.end());
	//	newDesktop.push_back(0);
	//	processSettings.lpDesktop = &newDesktop[0];

	// Switch to indicated desktop to launch program
	switchTo(desktop);

	// By the power of Grayskull . . . I HAVE THE POWER!!!
	SetPrivilege(SE_DEBUG_NAME, TRUE);

	// Start Launcher as new user to get token
	// Using CreateProcessWithLogonW() to retain Logon SID
	PROCESS_INFORMATION launcherInfo = { 0 };
	STARTUPINFOW launcherSettings = { 0 };
	launcherSettings.cb = sizeof(STARTUPINFOW);
	wstring toStart;

	// Creating new process with CreateProcessWithLogonW() to get impersonation of specified user with old Logon SID
	if (isExplorer)
	{
		toStart = L"C:\\Windows\\Explorer.exe";

		// Get a handle to explorer.exe
		// Return on failure
		DWORD explorerID;
		HWND explorerWindow = FindWindow(L"Shell_TrayWnd", NULL);
		if (explorerWindow == NULL)
			return launchFileManagerWorked;
		GetWindowThreadProcessId(explorerWindow, &explorerID);
		HANDLE explorerHANDLE = OpenProcess(PROCESS_ALL_ACCESS, FALSE, explorerID);
		if (explorerHANDLE == NULL)
			return launchFileManagerWorked;

		// Try to close Explorer.exe using Window message exit code
		//	If that fails, terminate process
		if (!PostMessage(explorerWindow, WM_USER + 436, 0, 0))
			TerminateProcess(explorerHANDLE, 0);

		// Give shell time to exit fully
		// WaitForSingleObject() isn't working correctly with explorer.exe on Windows 10
		DWORD exitCode;
		int count = 0;
		do
		{
			Sleep(500);
			GetExitCodeProcess(explorerHANDLE, &exitCode);
			count++;
		} while (exitCode == STILL_ACTIVE && count < 10);

		if (!CreateProcessWithLogonW(userToAllow.c_str(), NULL, sandboxUserCred.c_str(), LOGON_WITH_PROFILE, KEEPALIVE.c_str(), NULL, CREATE_NEW_CONSOLE, NULL, NULL, &launcherSettings, &launcherInfo))
			return launchFileManagerWorked;
	}
	else
	{
		toStart = fileManager;

		if (!CreateProcessWithLogonW(userToAllow.c_str(), NULL, sandboxUserCred.c_str(), LOGON_WITH_PROFILE, KEEPALIVE.c_str(), NULL, CREATE_NEW_CONSOLE, NULL, NULL, &launcherSettings, &launcherInfo))
			return launchFileManagerWorked;
	}

	// Gives the keepAlive program time to start fully
	WaitForSingleObject(launcherInfo.hProcess, 5000);

	// Code to impersonate using LogonUser() left for documentation
	// Creates new Logon SID
	// LogonUser(userToAllow.c_str(), NULL, sandboxUserCred.c_str(), LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &tokenHandle)

	// Clear password as soon as possible
	SecureZeroMemory(&sandboxUserCred, sandboxUserCred.size());

	PROCESS_INFORMATION processInfo = { 0 };
	STARTUPINFOW processSettings = { 0 };
	processSettings.cb = sizeof(STARTUPINFOW);
	SECURITY_ATTRIBUTES processDACL;
	HANDLE processTokenHandle = { 0 };
	HANDLE tokenHandle = { 0 };

	// Taking token from CreateProcessWithLogonW(), duplicating it, setting new default DACL and using that to launch file manager
	if (createDACL(userToAllow, &processDACL, userToDeny)
		&& OpenProcessToken(launcherInfo.hProcess, MAXIMUM_ALLOWED, &processTokenHandle)
		&& DuplicateTokenEx(processTokenHandle, MAXIMUM_ALLOWED, &processDACL, SecurityDelegation, TokenPrimary, &tokenHandle)
		&& setDefaultDACL(processDACL, tokenHandle)
		&& CreateProcessWithTokenW(tokenHandle, NULL, toStart.c_str(), NULL, 0, 0, NULL, &processSettings, &processInfo))
		launchFileManagerWorked = true;

	// Let the power return.
	SetPrivilege(SE_DEBUG_NAME, FALSE);

	// Clean up
//	ResumeThread(launcherInfo.hThread);
	LocalFree(processDACL.lpSecurityDescriptor);
	CloseHandle(launcherInfo.hThread);
	CloseHandle(launcherInfo.hProcess);
	CloseHandle(tokenHandle);

	// These were only created if everything worked
	if (launchFileManagerWorked)
	{
		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);
	}

	return launchFileManagerWorked;
}

// Takes name of desktop to be created and security attribute structure to be applied to desktop
// Returns true if creation successful
// Returns false otherwise
bool opener(const wstring &sandboxD, SECURITY_ATTRIBUTES* denyDACL, HDESK &createSandbox)
{
	bool openerWorked = false;

	// true iff a handle to sandboxD desktop is returned
	createSandbox = CreateDesktop(sandboxD.c_str(), NULL, NULL, 0, GENERIC_ALL, denyDACL);

	if (createSandbox != NULL)
		openerWorked = true;

	return openerWorked;
}

// Takes name of desktop to be created and security attribute structure to be applied to desktop
// Returns true if creation successful
// Returns false otherwise
bool opener(const wstring &windowStation, SECURITY_ATTRIBUTES* allowDACL, HWINSTA &windowStationHandle)
{
	bool openerWorked = false;

	// true iff a handle to sandboxD desktop is returned
	windowStationHandle = CreateWindowStation(windowStation.c_str(), NULL, GENERIC_ALL, allowDACL);

	if (windowStationHandle != NULL && SetProcessWindowStation(windowStationHandle))
		openerWorked = true;

	return openerWorked;
}

// Returns name of currently active desktop
wstring currentDesktopName()
{

	HDESK currentDesktop = OpenInputDesktop(0, FALSE, DESKTOP_SWITCHDESKTOP);
	wstring currentNameString;

	// if currentDesktop handle is valid, get name of currentDesktop
	if (currentDesktop != NULL)
	{
		const int OBJECTNAME = 2; // Tells GetUserObjectInformation() to return name of desktop
		DWORD sizeNeeded = 0;
		// Get size of buffer needed
		GetUserObjectInformation(currentDesktop, OBJECTNAME, NULL, NULL, &sizeNeeded);

		// Create buffer of correct size
		TCHAR *currentName = new TCHAR[sizeNeeded];
		// Get name
		if (GetUserObjectInformation(currentDesktop, OBJECTNAME, currentName, sizeNeeded, NULL))
			currentNameString = currentName;

		delete[] currentName;

		CloseDesktop(currentDesktop);
	}

	return currentNameString;
}

// Returns name of currently window station
wstring currentWindowStationName()
{

	HWINSTA currentWindowStation = GetProcessWindowStation();

	wstring currentNameString;

	// if currentDesktop handle is valid, get name of currentDesktop
	if (currentWindowStation != NULL)
	{
		const int OBJECTNAME = 2; // Tells GetUserObjectInformation() to return name of desktop
		DWORD sizeNeeded = 0;
		// Get size of buffer needed
		GetUserObjectInformation(currentWindowStation, OBJECTNAME, NULL, NULL, &sizeNeeded);

		// Create buffer of correct size
		TCHAR *currentName = new TCHAR[sizeNeeded];
		// Get name
		if (GetUserObjectInformation(currentWindowStation, OBJECTNAME, currentName, sizeNeeded, NULL))
			currentNameString = currentName;

		delete[] currentName;

		CloseWindowStation(currentWindowStation);
	}

	return currentNameString;
}

// Switches process and display to indicated destkop
bool switchTo(const wstring &newDesktop, const wstring &newWindowStation)
{
	bool switchToWorked = false;

	// Gets handle to workstation/desktop to switch to
	HWINSTA windowStationHandle = OpenWindowStation(newWindowStation.c_str(), FALSE, GENERIC_ALL);
	if (!SetProcessWindowStation(windowStationHandle))
		return switchToWorked;
	else
		CloseWindowStation(windowStationHandle);

	HDESK newDesktopHandle = OpenDesktop(newDesktop.c_str(), 0, FALSE, GENERIC_ALL);

	// switchToWorked is true iff SwitchDesktop worked
	if (newDesktopHandle != NULL && SetThreadDesktop(newDesktopHandle) && SwitchDesktop(newDesktopHandle))
	{
		CloseDesktop(newDesktopHandle);
		switchToWorked = true;
	}

	return switchToWorked;
}

// Sets and removes SE_DEBUG privilege
// Code mostly straight from MSDN example
BOOL SetPrivilege(
	LPCTSTR Privilege,  // Privilege to enable/disable 
	BOOL bEnablePrivilege  // TRUE to enable. FALSE to disable 
	)
{
	HANDLE hToken;
	OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);

	TOKEN_PRIVILEGES tp = { 0 };
	// Initialize everything to zero 
	LUID luid;
	DWORD cb = sizeof(TOKEN_PRIVILEGES);
	if (!LookupPrivilegeValue(NULL, Privilege, &luid))
		return FALSE;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege) {
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	}
	else {
		tp.Privileges[0].Attributes = 0;
	}
	AdjustTokenPrivileges(hToken, FALSE, &tp, cb, NULL, NULL);

	CloseHandle(hToken);

	if (GetLastError() != ERROR_SUCCESS)
		return FALSE;

	return TRUE;
}
