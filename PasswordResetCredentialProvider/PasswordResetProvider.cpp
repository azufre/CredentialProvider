//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
//
// PasswordResetProvider implements ICredentialProvider, which is the main
// interface that logonUI uses to decide which tiles to display.

#include <credentialprovider.h>
#include "PasswordResetProvider.h"
#include "PasswordResetCredential.h"
#include "guid.h"

// PasswordResetProvider ////////////////////////////////////////////////////////

PasswordResetProvider::PasswordResetProvider():
	_cRef(1),
	_dwNumCreds(0)
{
	DllAddRef();

	ZeroMemory(_rgpCredentials, sizeof(_rgpCredentials));
}

PasswordResetProvider::~PasswordResetProvider()
{
	for (size_t i = 0; i < _dwNumCreds; i++)
	{
		if (_rgpCredentials[i] != NULL)
		{
			_rgpCredentials[i]->Release();
		}
	}

	DllRelease();
}

// SetUsageScenario is the provider's cue that it's going to be asked for tiles
// in a subsequent call.  In this sample we have chosen to precreate the credentials 
// for the usage scenario passed in cpus instead of saving off cpus and only creating
// the credentials when we're asked to.
// This sample only handles the logon and unlock scenarios as those are the most common.
HRESULT PasswordResetProvider::SetUsageScenario(
	CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
	DWORD dwFlags
	)
{
	UNREFERENCED_PARAMETER(dwFlags);

	HRESULT hr;
	static bool s_bCredsEnumerated = false;

	// Decide which scenarios to support here. Returning E_NOTIMPL simply tells the caller
	// that we're not designed for that scenario.
	switch (cpus)
	{
	case CPUS_LOGON:
	case CPUS_UNLOCK_WORKSTATION:       
	case CPUS_CREDUI:
		// A more advanced credprov might only enumerate tiles for the user whose owns the locked
		// session, since those are the only creds that wil work
		if (!s_bCredsEnumerated)
		{
			hr = this->_EnumerateCredentials();
			s_bCredsEnumerated = true;
		}
		else
		{
			hr = S_OK;
		}
		break;

	case CPUS_CHANGE_PASSWORD:
		hr = E_NOTIMPL;
		break;

	default:
		hr = E_INVALIDARG;
		break;
	}

	return hr;
}

// SetSerialization takes the kind of buffer that you would normally return to LogonUI for
// an authentication attempt.  It's the opposite of ICredentialProviderCredential::GetSerialization.
// GetSerialization is implement by a credential and serializes that credential.  Instead,
// SetSerialization takes the serialization and uses it to create a credential.
//
// SetSerialization is called for two main scenarios.  The first scenario is in the credui case
// where it is prepopulating a tile with credentials that the user chose to store in the OS.
// The second situation is in a remote logon case where the remote client may wish to 
// prepopulate a tile with a username, or in some cases, completely populate the tile and
// use it to logon without showing any UI.
//
// Since this sample doesn't support CPUS_CREDUI, we have not implemented the credui specific
// pieces of this function.  For information on that, please see the credUI sample.
STDMETHODIMP PasswordResetProvider::SetSerialization(
	const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs
	)
{
	UNREFERENCED_PARAMETER(pcpcs);

	return E_NOTIMPL;
}

// Called by LogonUI to give you a callback.  Providers often use the callback if they
// some event would cause them to need to change the set of tiles that they enumerated
HRESULT PasswordResetProvider::Advise(
	ICredentialProviderEvents* pcpe,
	UINT_PTR upAdviseContext
	)
{
	UNREFERENCED_PARAMETER(pcpe);
	UNREFERENCED_PARAMETER(upAdviseContext);

	return E_NOTIMPL;
}

// Called by LogonUI when the ICredentialProviderEvents callback is no longer valid.
HRESULT PasswordResetProvider::UnAdvise()
{
	return E_NOTIMPL;
}

// Called by LogonUI to determine the number of fields in your tiles.  This
// does mean that all your tiles must have the same number of fields.
// This number must include both visible and invisible fields. If you want a tile
// to have different fields from the other tiles you enumerate for a given usage
// scenario you must include them all in this count and then hide/show them as desired 
// using the field descriptors.
HRESULT PasswordResetProvider::GetFieldDescriptorCount(
	DWORD* pdwCount
	)
{
	*pdwCount = SFI_NUM_FIELDS;

	return S_OK;
}

// Gets the field descriptor for a particular field
HRESULT PasswordResetProvider::GetFieldDescriptorAt(
	DWORD dwIndex, 
	CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd
	)
{    
	HRESULT hr;

	// Verify dwIndex is a valid field.
	if ((dwIndex < SFI_NUM_FIELDS) && ppcpfd)
	{
		hr = FieldDescriptorCoAllocCopy(s_rgCredProvFieldDescriptors[dwIndex], ppcpfd);
	}
	else
	{ 
		hr = E_INVALIDARG;
	}

	return hr;
}

// Sets pdwCount to the number of tiles that we wish to show at this time.
// Sets pdwDefault to the index of the tile which should be used as the default.
//
// The default tile is the tile which will be shown in the zoomed view by default. If 
// more than one provider specifies a default tile the behavior is the last used cred
// prov gets to specify the default tile to be displayed
//
// If *pbAutoLogonWithDefault is TRUE, LogonUI will immediately call GetSerialization
// on the credential you've specified as the default and will submit that credential
// for authentication without showing any further UI.
HRESULT PasswordResetProvider::GetCredentialCount(
	DWORD* pdwCount,
	DWORD* pdwDefault,
	BOOL* pbAutoLogonWithDefault
	)
{
	HRESULT hr;

	*pdwDefault = CREDENTIAL_PROVIDER_NO_DEFAULT; // Never want a zoomed view
	*pbAutoLogonWithDefault = FALSE;

	*pdwCount = _dwNumCreds; 
	if (*pdwCount > 0)
	{
		hr = S_OK;
	}
	else
	{
		hr = E_FAIL;
	}

	return hr;
}

// Returns the credential at the index specified by dwIndex. This function is called by logonUI to enumerate
// the tiles.
HRESULT PasswordResetProvider::GetCredentialAt(
	DWORD dwIndex, 
	ICredentialProviderCredential** ppcpc
	)
{
	HRESULT hr;

	// Validate parameters.
	if((dwIndex < _dwNumCreds) && ppcpc)
	{
		hr = _rgpCredentials[dwIndex]->QueryInterface(IID_ICredentialProviderCredential, reinterpret_cast<void**>(ppcpc));
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Creates a Credential with the SFI_FIMTITLE field's value set to pwzUsername.
HRESULT PasswordResetProvider::_EnumerateOneCredential(
	DWORD dwCredentialIndex,
	PCWSTR pwzUsername
	)
{
	HRESULT hr;

	// Allocate memory for the new credential.
	PasswordResetCredential* ppc = new PasswordResetCredential();

	if (ppc)
	{
		// Set the Field State Pair and Field Descriptors for ppc's fields
		// to the defaults (s_rgCredProvFieldDescriptors, and s_rgFieldStatePairs) and the value of SFI_FIMTITLE
		// to pwzUsername.
		hr = ppc->Initialize(s_rgCredProvFieldDescriptors, s_rgFieldStatePairs, pwzUsername);

		if (SUCCEEDED(hr))
		{
			_rgpCredentials[dwCredentialIndex] = ppc;
			_dwNumCreds++;
		}
		else
		{
			// Release the pointer to account for the local reference.
			ppc->Release();
		}
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}

	return hr;
}

// Sets up all the credentials for this provider. Since we always show the same tiles, 
// we just set it up once.
HRESULT PasswordResetProvider::_EnumerateCredentials()
{
	HRESULT hr = _EnumerateOneCredential(0, NULL);

	return hr;
}

// Boilerplate code to create our provider.
HRESULT PasswordResetProvider_CreateInstance(REFIID riid, void** ppv)
{
	HRESULT hr;

	PasswordResetProvider* pProvider = new PasswordResetProvider();

	if (pProvider)
	{
		hr = pProvider->QueryInterface(riid, ppv);
		pProvider->Release();
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}

	return hr;
}
