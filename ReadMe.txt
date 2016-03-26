ProtectedDesktops:
A program that uses native Windows features to create a fully sandboxed environment on Windows.

This program was written as part of an undergraduate research project
presented at the Honors Transfer Council of California, 26 Mar. 2016.

Copyright 2016 Joshua DuFault


Version 0.1

This program is in beta. It has been tested on Windows 7 x64.


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



Application Files:


ProtectedDesktops.exe

Console.exe

KeepAlive.exe

DesktopSwitcher.exe

Settings.ini

CloseSandbox.bat



Directions:


1. The user must be running Windows Vista or later and have 3 user accounts set up as follows:

	Account 1: the main user. This is the account you use at the Windows logon screen. This account 

should be a Standard User Account

	Account 2: an administrator account.

	Account 3: an account to be used with the sandbox. The account could be used to isolate 

potentially dangerous programs or protect sensitive data. It obviously can't be used for both at the 

same time.

All accounts must have a password.

The user must have logged on to all 3 accounts through the main Windows logon screen at least once.

Make user you can a run as for Account 3. Hold down shit and right click on a simple program like 

notepad. Select "Run as different user." Enter the username and password of Account 3. Verify the 

program ran.



2. The user must have a file manager other than Windows Explorer installed in a protected location. For 

example: have Explorer++ installed in C:\Program Files (x86)\


3. The Settings.ini file must be updated before use. It is in the following format:

	Main user

	Sandboxed user

	Path to file manager

The included Settings.ini file has an example.


4. The CloseSandbox.bat file also has to be updated.

	The file can be opened in any text editor such as notepad.

	The part of the line that says "Sandbox" must be changed to the name of Account 3.
	
	For example: taskkill /f /t /fi "USERNAME eq MyOtherUserAccount"


4. All program files should be saved in a protected location such as C:\Program Files (x86)\


5. Run ProtectedDesktops.exe and enter the requested information when prompted.


The sandbox is now set up. Run DesktopSwitcher.exe to switch between the sandbox and the main desktop. 

Run CloseSandbox.bat to close the sandbox by right clicking and selecting "Run as administrator." 

Programs launched in the sandbox have no access to programs launched on the main desktop and vice versa. 

Processes cannot jump to the other desktop. They cannot keylog or launch shatter attacks across the 

desktop barrier. They cannot read or write to the memory of processes on the other side of the sandbox. 

Processes run on the sandbox have a different set of user folders then the main user. The two users 

cannot read or write to each other's files. Otherwise, programs run like normal in the sandbox. 

Restarting the computer reverts all settings adjusted by the program back to normal.



Background:


Antivirus programs block only 45% of malware attacks according to Brian Dye, the senior vice president 

for information security at Symantec in a Wall Street Journal interview. Many security products use 

sandboxes to attempt to provide better protection by isolating potentially dangerous processes or 

protect sensitive information; however, many of the implementations are easily bypassed or cause 

additional security problems. Common sandboxing techniques currently in use and their limitations are 

listed below.


	1. Running programs on the same desktop with reduced privileges using a restricted or 

impersonation token:
	
		Malware can use windows messaging based attacks i.e. shatter attacks to jump to a more 

privileged program.
	
		All programs in the same session can read each others memory by default and steal 

passwords or other sensitive data from memory.


	2. Creating separate desktops to isolate potentially dangerous programs or protect sensitive 

data.

		Desktops are secured by user, not by process. Malware launched by the same user can 

easily jump between desktops.

		Programs that switch to a dimmed desktop the way Windows does when UAC is invoked are 

merely mimicking the Windows secure desktop. Only processes with system privileges have access to that 

desktop. Instead, these programs create a simple alternate desktop that is easily accessible.


	3. Combining numbers 1 and 2, and using a restricted token with a made up SID and adding a deny 

for that SID to the main desktop's access control list to prevent malware from jumping to the main 

desktop.

		Restricted tokens must contain the RESTRICTED SID which gives read access to a users 

files and process memory. Malware can still steal passwords from memory or read sensitive files.


	4. Using Job Objects to prevent malware from accessing other processes.

		Job objects don't prevent global hooks. Keylogging and code injection are still 

possible. Malware can still steal passwords and jump to less restricted processes.

		In Windows 7 and below, Job Objects don't nest. This means if a program is put into a 

Job Object, it can't use its own. Many programs use Job Objects to implement their own internal 

sandboxes.


	5. Using integrity levels to prevent shatter attacks, code injection, and key logging.
	
		Integrity levels block most windows messages to more privileged processes but not all. 

Some messages are still allowed through and can be used to launch a shatter attack.

		A program installed in a protected area can have a request in its manifest to bypass 

this protection entirely. Some trusted programs are frequently taken over by malware e.g. internet 

browsers.

		Programs running at one integrity level have access to any other program running at the 

same or lower integrity level.

		Many applications use integrity levels in internal sandboxes creating a child process at 

a lower integrity level than the parent program to run potentially dangerous code. Putting such an 

application in a general purpose sandbox using integrity levels can damage the application's internal 

sandbox by putting the parent and child processes at the same integrity level.

		Programs not designed to run at a low or untrusted integrity level frequently don't work 

correctly when forced to do so.


	6. Using all of the above with a token so restricted that it has no access to the file system.
	
		Effective from a security point of view but such a program is not able to do anything 

useful on its own. It needs to be built specifically to be used with a more privileged broker to provide 

a gateway to system resources. Individual application sandboxes can be written this way but a program 

not built to run in such a sandbox won't work.



Hypothesis: It's possible to use Windows' built in security features in a way to make a sandbox that 

better protects a user's data.


It is possible to launch a program using an impersonation token as an alternate user on a newly created 

desktop and then set the discretionary access control list (DACL) on the desktops to prevent programs 

from jumping over. This will prevent a program launched by any user from accessing the other desktop. 

Desktops block all window messages and global hooks preventing shatter attacks and keylogging. A process 

doesn't have write access to any program launched by a different user, preventing code injection. Each 

user has its own set of files and can't access another user's files.

Any process, regardless of user, can read the memory of any other process in the same logon session at 

the same integrity level. A malicious program can still steal sensitive data out of memory. Processes 

are protected by DACLs and adding a deny entry in each process blocking the sandboxed user would prevent 

those programs from stealing information out of memory. However, this would mean constantly watching for 

every new process that's created to add the needed denies. An exception is that a desktop similiar to 

the Windows secure desktop can be created to enter passwords. Programs running as administrator should 

be kept to a minimum because they can the most damage if compromised but since the StartSandbox.exe 

process has to be run as admin anyway to set all the DACLs, it can create a desktop that only programs 

running as an administrator can access. This won't protect standard processes.

The key to this sandbox is the default DACL. If no DACL is specified when a new process is launched, 

Windows will create a default DACL based on the parent process's token. After the initial startup, the 

parent process is typically explorer.exe. Explorer.exe consists of Windows Explorer and the Start Menu. 

The default DACL can be modified to add the neccessary deny entries to create a secure sandbox. Then, 

any new program launched will automatically receive the same DACL and any new program that it launches 

will receive the same DACL and so on. The only exception is if a launching program specifies a DACL 

during creation which is typically only done for a reason such as creating an internal application 

sandbox. In this case, the default DACLs are still usually inherited automatically, the launching 

program just adds further restrictions.

The resulting sandbox barrier blocks data theft from memory and files, keylogging, code injection, and 

shatter attacks across the sandbox barrier. Even attempts to shut down another process across the 

barrier are blocked. The clipboard and public user folders "C:\Users\Public" can be used to transfer 

files and data in and out of the sandbox. This sandbox can be used either to contain malicious programs 

or protect sensitive  information. Only a process with admin credentials can see across the barrier. A 

desktop switching program run with admin credentials is neccessary to switch visible desktops. A batch 

file running taskkill as administrator can be used to end all programs in the sandbox. The desktop will 

automatically close when all programs with access to it are closed.

Integrity levels and Job Objects aren't considered because they interfer with commonly used 

applications.



Testing:


The sandbox successfully blocked the following actions across the sandbox barrier:

	1. Reading and writing information in a process' memory.

	2. Reading or writing to the other user's files.

	3. Keylogging.

	4. Windows messages.


All tested programs worked normally in the sandbox. This includes involved programs that set their own 

internal sandboxes.



Remaining vulnerabilities:


	1. This only protects against malware running with user rights. If it gets administrator access 

either through user error or a novel privilege escalation attack, security is compromised.

	2. There is no way to isolate the Windows kernel and no way to protect against a kernel exploit.

	3. Processes on either side of the sandbox access some of the same resources. There are multiple 

writable locations that both sides have access to. It might be possible for malware to leave malicious 

code waiting in a place that a vulnerable program on the other side will access.



Conclusion:


The remaining vulnerabilities are significantly more difficult than merely reading a password out of 

memory. Perfect security is impossible but this sandbox is more effective than several other currently 

used types. It is also lighter on system resources:

	Total application size: less than 1MB.

	CPU use after initial setup: 0.

	Installation: not required.


Microsoft could implement a similiar sandbox in an update to Windows 10. Windows 10 currently offers a 

multiple desktop feature but it is entirely cosmetic and offers no security. This would give a native 

way for users to protect sensitive information, such as banking data, from theft and keep important 

files safe from destructive malware, such as ransomware.


Future research possibilities include setting default DACLs on kernel objects and inherited permissions 

on files and registry keys to isolate them against vulnerability 3.
