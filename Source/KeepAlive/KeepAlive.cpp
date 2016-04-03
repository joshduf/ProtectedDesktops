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

// Just here to keep a desktop alive or get a token from
int wmain()
{

	cout << "Desktop set. Press any key to continue: ";
	cin.get();

	return 0;
}
