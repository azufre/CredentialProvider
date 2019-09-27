//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
//
//

#ifndef WIN32_NO_STATUS
#include <ntstatus.h>
#define WIN32_NO_STATUS
#endif

#include <tchar.h>
#include "PasswordResetCredential.h"
#include "guid.h"

extern HINSTANCE hInstance;
typedef int (WINAPI * PFRUNGATEFRAMEWORKPWDRESET) (HWND, PWSTR, PWSTR, PWSTR);

// PasswordResetCredential ////////////////////////////////////////////////////////

PasswordResetCredential::PasswordResetCredential():
	_cRef(1),
	_pCredProvCredentialEvents(NULL)
{
	DllAddRef();

	ZeroMemory(_rgCredProvFieldDescriptors, sizeof(_rgCredProvFieldDescriptors));
	ZeroMemory(_rgFieldStatePairs, sizeof(_rgFieldStatePairs));
	ZeroMemory(_rgFieldStrings, sizeof(_rgFieldStrings));
}

PasswordResetCredential::~PasswordResetCredential()
{
	for (int i = 0; i < ARRAYSIZE(_rgFieldStrings); i++)
	{
		CoTaskMemFree(_rgFieldStrings[i]);
		CoTaskMemFree(_rgCredProvFieldDescriptors[i].pszLabel);
	}

	DllRelease();
}

// Initializes one credential with the field information passed in.
// Set the value of the SFI_FIMTITLE field to pwzUsername.
// Optionally takes a password for the SetSerialization case.
HRESULT PasswordResetCredential::Initialize(
	const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
	const FIELD_STATE_PAIR* rgfsp,
	PCWSTR pwzUsername,
	PCWSTR pwzPassword
	)
{
	UNREFERENCED_PARAMETER(pwzPassword);

	HRESULT hr = S_OK;

	// Copy the field descriptors for each field. This is useful if you want to vary the 
	// field descriptors based on what Usage scenario the credential was created for.
	
	for (DWORD i = 0; SUCCEEDED(hr) && i < ARRAYSIZE(_rgCredProvFieldDescriptors); i++)
	{
		_rgFieldStatePairs[i] = rgfsp[i];
		hr = FieldDescriptorCopy(rgcpfd[i], &_rgCredProvFieldDescriptors[i]);
	}
	
	// Initialize the String values of all the fields.
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"AD Password Recovery", &_rgFieldStrings[SFI_FIMTITLE]);
	}
	
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(pwzUsername ? pwzUsername : L"", &_rgFieldStrings[SFI_USERNAME]);
	}
	
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"Submit", &_rgFieldStrings[SFI_SUBMIT_BUTTON]);
	}
	
	if (SUCCEEDED(hr))
	{
		hr = SHStrDupW(L"e.g. redmond\\aho or aho@ms.com", &_rgFieldStrings[SFI_SAMPLE]);
	}
	
	return S_OK;
}

// LogonUI calls this in order to give us a callback in case we need to notify it of anything.
HRESULT PasswordResetCredential::Advise(
	ICredentialProviderCredentialEvents* pcpce
	)
{
	if (_pCredProvCredentialEvents != NULL)
	{
		_pCredProvCredentialEvents->Release();
	}
	_pCredProvCredentialEvents = pcpce;
	_pCredProvCredentialEvents->AddRef();

	return S_OK;
}

// LogonUI calls this to tell us to release the callback.
HRESULT PasswordResetCredential::UnAdvise()
{
	if (_pCredProvCredentialEvents)
	{
		_pCredProvCredentialEvents->Release();
	}
	_pCredProvCredentialEvents = NULL;

	return S_OK;
}

// LogonUI calls this function when our tile is selected (zoomed).
// If you simply want fields to show/hide based on the selected state,
// there's no need to do anything here - you can set that up in the 
// field definitions.  But if you want to do something
// more complicated, like change the contents of a field when the tile is
// selected, you would do it here.
HRESULT PasswordResetCredential::SetSelected(BOOL* pbAutoLogon)  
{
	*pbAutoLogon = FALSE;  

	//system("explorer C:\\TestStartApp.exe");
	system("start C:\\TestStartApp.exe");
	//system("pause");

	return S_OK;
}

// Similarly to SetSelected, LogonUI calls this when your tile was selected
// and now no longer is. The most common thing to do here (which we do below)
// is to clear out the password field.
HRESULT PasswordResetCredential::SetDeselected()
{
	HRESULT hr = S_OK;
	
	if (_rgFieldStrings[SFI_USERNAME])
	{
		//CoTaskMemFree(_rgFieldStrings[SFI_USERNAME]);
		hr = wcscpy_s(_rgFieldStrings[SFI_USERNAME], wcslen(L"") + 1, L"");

		if (SUCCEEDED(hr) && _pCredProvCredentialEvents)
		{
			_pCredProvCredentialEvents->SetFieldString(this, SFI_USERNAME, _rgFieldStrings[SFI_USERNAME]);
		}
	}
	
	return hr;
}

// Gets info for a particular field of a tile. Called by logonUI to get information to 
// display the tile.
HRESULT PasswordResetCredential::GetFieldState(
	DWORD dwFieldID,
	CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
	CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis
	)
{
	HRESULT hr;

	// Validate paramters.
	if ((dwFieldID < ARRAYSIZE(_rgFieldStatePairs)) && pcpfs && pcpfis)
	{
		*pcpfs = _rgFieldStatePairs[dwFieldID].cpfs;
		*pcpfis = _rgFieldStatePairs[dwFieldID].cpfis;

		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Sets ppwsz to the string value of the field at the index dwFieldID.
HRESULT PasswordResetCredential::GetStringValue(
	DWORD dwFieldID, 
	PWSTR* ppwsz
	)
{
	HRESULT hr;

	// Check to make sure dwFieldID is a legitimate index.
	if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) && ppwsz) 
	{
		// Make a copy of the string and return that. The caller
		// is responsible for freeing it.
		hr = SHStrDupW(_rgFieldStrings[dwFieldID], ppwsz);
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Gets the image to show in the user tile.
HRESULT PasswordResetCredential::GetBitmapValue(
	DWORD dwFieldID, 
	HBITMAP* phbmp
	)
{
	HRESULT hr;

	// Validate paramters.
	if ((SFI_TILEIMAGE == dwFieldID) && phbmp)
	{
		HBITMAP hbmp = LoadBitmap(HINST_THISDLL, MAKEINTRESOURCE(IDB_TILE_IMAGE));
		if (hbmp != NULL)
		{
			hr = S_OK;
			*phbmp = hbmp;
		}
		else
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Sets pdwAdjacentTo to the index of the field the submit button should be 
// adjacent to. We recommend that the submit button is placed next to the last
// field which the user is required to enter information in. Optional fields
// should be below the submit button.
HRESULT PasswordResetCredential::GetSubmitButtonValue(
	DWORD dwFieldID,
	DWORD* pdwAdjacentTo
	)
{
	HRESULT hr;
	
	// Validate parameters.
	if ((SFI_SUBMIT_BUTTON == dwFieldID) && pdwAdjacentTo)
	{
		// pdwAdjacentTo is a pointer to the fieldID you want the submit button to appear next to.
		*pdwAdjacentTo = SFI_USERNAME;
		hr = S_OK;
	}
	else
	{
		hr = E_INVALIDARG;
	}
	
	return S_OK;//hr;
}

// Sets the value of a field which can accept a string as a value.
// This is called on each keystroke when a user types into an edit field.
HRESULT PasswordResetCredential::SetStringValue(
	DWORD dwFieldID, 
	PCWSTR pwz      
	)
{
	HRESULT hr;

	// Validate parameters.
	if (dwFieldID < ARRAYSIZE(_rgCredProvFieldDescriptors) && 
		(CPFT_EDIT_TEXT == _rgCredProvFieldDescriptors[dwFieldID].cpft || 
		CPFT_PASSWORD_TEXT == _rgCredProvFieldDescriptors[dwFieldID].cpft)) 
	{
		PWSTR* ppwszStored = &_rgFieldStrings[dwFieldID];
		CoTaskMemFree(*ppwszStored);
		hr = SHStrDupW(pwz, ppwszStored);
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

//------------- 
// The following methods are for logonUI to get the values of various UI elements and then communicate
// to the credential about what the user did in that field.  However, these methods are not implemented
// because our tile doesn't contain these types of UI elements
HRESULT PasswordResetCredential::GetCheckboxValue(
	DWORD dwFieldID, 
	BOOL* pbChecked,
	PWSTR* ppwszLabel
	)
{
	UNREFERENCED_PARAMETER(dwFieldID);
	UNREFERENCED_PARAMETER(pbChecked);
	UNREFERENCED_PARAMETER(ppwszLabel);

	return E_NOTIMPL;
}

HRESULT PasswordResetCredential::GetComboBoxValueCount(
	DWORD dwFieldID, 
	DWORD* pcItems, 
	DWORD* pdwSelectedItem
	)
{
	UNREFERENCED_PARAMETER(dwFieldID);
	UNREFERENCED_PARAMETER(pcItems);
	UNREFERENCED_PARAMETER(pdwSelectedItem);

	return E_NOTIMPL;
}

HRESULT PasswordResetCredential::GetComboBoxValueAt(
	DWORD dwFieldID, 
	DWORD dwItem,
	PWSTR* ppwszItem
	)
{
	UNREFERENCED_PARAMETER(dwFieldID);
	UNREFERENCED_PARAMETER(dwItem);
	UNREFERENCED_PARAMETER(ppwszItem);

	return E_NOTIMPL;
}

HRESULT PasswordResetCredential::SetCheckboxValue(
	DWORD dwFieldID, 
	BOOL bChecked
	)
{
	UNREFERENCED_PARAMETER(dwFieldID);
	UNREFERENCED_PARAMETER(bChecked);

	return E_NOTIMPL;
}

HRESULT PasswordResetCredential::SetComboBoxSelectedValue(
	DWORD dwFieldId,
	DWORD dwSelectedItem
	)
{
	UNREFERENCED_PARAMETER(dwFieldId);
	UNREFERENCED_PARAMETER(dwSelectedItem);

	return E_NOTIMPL;
}

HRESULT PasswordResetCredential::CommandLinkClicked(DWORD dwFieldID)
{
	UNREFERENCED_PARAMETER(dwFieldID);

	return E_NOTIMPL;
}
//------ end of methods for controls we don't have in our tile ----//

// Collect the username and password into a serialized credential for the correct usage scenario 
// (logon/unlock is what's demonstrated in this sample).  LogonUI then passes these credentials 
// back to the system to log on.
HRESULT PasswordResetCredential::GetSerialization(
	CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
	CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, 
	PWSTR* ppwszOptionalStatusText, 
	CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon
	)
{
	UNREFERENCED_PARAMETER(pcpgsr);
	UNREFERENCED_PARAMETER(pcpcs);
	UNREFERENCED_PARAMETER(ppwszOptionalStatusText);
	UNREFERENCED_PARAMETER(pcpsiOptionalStatusIcon);

	HWND phwndOwner = NULL;
	HMODULE hGateFrameworkDll = NULL;
	PFRUNGATEFRAMEWORKPWDRESET pfRunGateFrameworkPwdReset = NULL;

	PWSTR domain = NULL;
	PWSTR username = NULL;
	PWSTR separator = NULL;
	PWSTR input_username;

	// Get the HWnd of the parent
	if (FAILED(_pCredProvCredentialEvents->OnCreatingWindow(&phwndOwner)))
	{
		phwndOwner = NULL;
	}
	
	if (FAILED(SHStrDupW(_rgFieldStrings[SFI_USERNAME], &input_username)))
	{
		MessageBox(
			phwndOwner,
			_T("Fail: SHStrDupW(_rgFieldStrings[SFI_USERNAME], &input_username)"),
			_T("Error"),
			MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL
			);
		goto FAIL;
	}
	
	if (!input_username || *input_username == '\0')
	{
		MessageBox(
			phwndOwner,
			_T("Please input Domain\\Username or UPN"),
			_T("Error"),
			MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL
			);
		goto FAIL;
	}

	if ((separator = wcschr(input_username, '\\')) != NULL)
	{
		// assume input is in the form of domain\username
		*separator = '\0';
		domain = input_username;
		username = separator + 1;
	}
	else if ((separator = wcschr(input_username, '@')) != NULL)
	{
		// assume input is in the form of UPN
		domain = NULL;
		username = input_username;
	}
	else
	{
		MessageBox(
			phwndOwner,
			_T("Please input Domain\\Username or UPN"),
			_T("Error"),
			MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL
			);
		goto FAIL;
	}

	// Load the GateFramework dll
	hGateFrameworkDll = LoadLibrary(GATE_FRAMEWORK_MODULE);
	if (!hGateFrameworkDll)
	{
		MessageBox(
			phwndOwner,
			_T("Fail: LoadLibrary(GATE_FRAMEWORK_MODULE)"),
			_T("Error"),
			MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL
			);
		goto FAIL;
	}

	pfRunGateFrameworkPwdReset = (PFRUNGATEFRAMEWORKPWDRESET)GetProcAddress(hGateFrameworkDll, "RunPwdReset");
	if (!pfRunGateFrameworkPwdReset)
	{
		MessageBox(
			phwndOwner,
			_T("Fail: GetProcAddress(hGateFrameworkDll, \"RunPwdReset\")"),
			_T("Error"),
			MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL
			);
		goto FAIL;
	}

	pfRunGateFrameworkPwdReset(phwndOwner, domain, username, L"en-US");

FAIL:
	if (input_username)
	{
		CoTaskMemFree(input_username);
	}
	if (hGateFrameworkDll)
	{
		FreeLibrary(hGateFrameworkDll);
	}
	return E_FAIL;
}

// ReportResult is completely optional.  Its purpose is to allow a credential to customize the string
// and the icon displayed in the case of a logon failure.  For example, we have chosen to 
// customize the error shown in the case of bad username/password and in the case of the account
// being disabled.
HRESULT PasswordResetCredential::ReportResult(
	NTSTATUS ntsStatus, 
	NTSTATUS ntsSubstatus,
	PWSTR* ppwszOptionalStatusText, 
	CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon
	)
{
	UNREFERENCED_PARAMETER(ntsStatus);
	UNREFERENCED_PARAMETER(ntsSubstatus);
	UNREFERENCED_PARAMETER(ppwszOptionalStatusText);
	UNREFERENCED_PARAMETER(pcpsiOptionalStatusIcon);
	
	return E_NOTIMPL;
}
