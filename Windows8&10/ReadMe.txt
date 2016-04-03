ProtectedDesktops:
A program that uses native Windows features to create a fully sandboxed environment on Windows.
(Special instructions for Windows 8 & 10)


This program was written as part of an undergraduate research project
presented at the Honors Transfer Council of California, 26 Mar. 2016.


Copyright 2016 Joshua DuFault

Contact: Josh.DuFault@gmail.com


Version 0.2

This program is in beta. It has been tested on Windows 7 x64, Windows 8 x64, and Windows 10 x64.



This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ProtectedDesktops.  If not, see <http://www.gnu.org/licenses/>.



Additional Files:


RunMe.bat

RunMe.exe



Directions:


1. To create local user accounts:

Start button -> Settings ->  Accounts -> Family & other users -> Add someone else to this PC ->
I don't have this person's sign-in information -> Add a user without a Microsoft account


2. To change the main user to a standard user:

Log in from new admin account to change the main to standard user

Need to create shortcut
Set to run as admin


3. Copy both files into the ProtectedDesktops folder.


4. Right click on RunMe.bat and select "Run as administrator."


5. On the new console window that opens, type RunMe and hit Enter.


6. Use the included launcher program to operate the sandbox.



Description:

In testing, the SetThreadDesktop() function failed consistently on Windows 10 and was unreliable on Windows 8 even though permissions are correct and the thread has not created any windows or hooks. This appears to be related to how conhost.exe is used on the newer operating systems. Conhost.exe launches as a child process to all console processes and handles GUI features.

This solution starts an elevated command prompt which then has conhost.exe attached to itself. A launcher program used at the command prompt starts the sandbox.

If anyone has another solution to this problem, please let me know.

