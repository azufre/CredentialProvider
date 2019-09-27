//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
//
//
Overview
--------
This sample implements a simple credential provider that works with both LogonUI and CredUI.
It is built on top of FIM 2010 Add-ins and Extensions's Self-Service Password Reset feature.
You will need to first install FIM 2010 Add-ins and Extensions.

This sample implements a simplified credential provider that is based on the sample 
credential provider from Windows SDK.  When run, the credential provider should enumerate one
tile which exposes Self-Service Password Reset functionality. Users should click on the tile,
enter their Domain\Username or UPN and click Submit. That would initiate the Password Reset
Sequence.

How to run this sample
----------------------
Once you have built the project, copy PasswordResetCredentialProvider.dll to the System32
directory on a Vista machine and run Register.reg from an elevated command prompt. The 
credential should appear the next time a logon is invoked (such as when switching users).

Recommended steps for testing / installation
--------------------------------------------
1. Compile *Release* x86/x64 version of the binaries.
2. Copy binaries to the test box with FIM 2010 Add-ins and Extensions installed.
3. Launch CredUILauncher.exe.
4. If it can't be started because SxS error,
   install Visual C++ 2008 Redistributable Package (x86/64).
5. Reboot if prompted.
6. Launch CredUILauncher.exe and make sure it can be started. You should see a CredUI prompt.
7. Execute Register.reg.
8. Launch CredUILauncher.exe and you should see the new Password Reset Credential Provider.
9. Copy PasswordResetCredentialProvider.dll to %SystemRoot%\System32.
10. Logoff and logon.

What this sample demonstrates
-----------------------------
This sample demonstrates an alternate entry point to Self-Service Password Reset.


