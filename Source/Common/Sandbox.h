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

#ifndef SANDBOX_H
#define SANDBOX_H

// Requires unicode
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

// This program should work on Vista and up
// The method of restarting explorer.exe used here began with Vista
// Also, this program provides no protection against unpatched operating system vulnerabilities
//	present in earlier versions of Windows
#ifndef _WIN32_WINNT 
#define _WIN32_WINNT _WIN32_WINNT_VISTA 
#endif

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <Aclapi.h>
#include <sddl.h>
#include <windows.h>

const std::wstring SANDBOX = L"SandboxedDesktop";
const std::wstring MAIN = L"Default";
const std::wstring ADMIN = L"AdminOnlyDesktop";
const std::wstring WINDOWSTATIONMAIN = L"WinSta0";
const std::wstring CONSOLEAPP = L"Console.exe";
const std::wstring KEEPALIVE = L"KeepAlive.exe";
const std::wstring SETTINGSFILE = L"settings.ini";

bool getSettings(std::wstring &mainUser, std::wstring &sandboxUser, std::wstring &fileManager);

bool createDACL(SECURITY_ATTRIBUTES *newDACL);
bool createDACL(const std::wstring &userToAllow, SECURITY_ATTRIBUTES *denyDACL, const std::wstring &userToDeny);
bool setDACL(SECURITY_ATTRIBUTES newDACL, HANDLE objectHandle, SE_OBJECT_TYPE objectType);
bool setDefaultDACL(SECURITY_ATTRIBUTES newDACL, HANDLE &tokenHandle);
bool usernameToSIDString(const std::wstring &username, std::wstring &userSIDString);

bool keyboardEchoOff();
bool getCredentials(const std::wstring &user, std::wstring &userCreds);

bool launchProcessOnDesktop(std::wstring process, std::wstring desktop, bool resume);
bool launchFileManager(const std::wstring &mainUser, const std::wstring &sandboxUser, const std::wstring &fileManager, const std::wstring &desktop, bool isExplorer);

bool opener(const std::wstring &sandboxD, SECURITY_ATTRIBUTES* denyDACL, HDESK &createSandbox);
bool opener(const std::wstring &sandboxD, SECURITY_ATTRIBUTES* denyDACL, HWINSTA &createSandbox); // Not currently used, provided for potential future use

std::wstring currentDesktopName();
std::wstring currentWindowStationName(); // Not currently used, included for potential future use
bool switchTo(const std::wstring &newDesktop, const std::wstring &newWindowStation = WINDOWSTATIONMAIN);

BOOL SetPrivilege(LPCTSTR Privilege, BOOL bEnablePrivilege);

#endif
