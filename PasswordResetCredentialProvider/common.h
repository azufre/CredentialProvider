//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.
//
// This file contains some global variables that describe what our
// sample tile looks like.  For example, it defines what fields a tile has 
// and which fields show in which states of LogonUI.

#pragma once
#include <credentialprovider.h>
#include <ntsecapi.h>
#define SECURITY_WIN32
#include <security.h>
#include <intsafe.h>

#define MAX_ULONG  ((ULONG)(-1))

#define GATE_FRAMEWORK_MODULE _T("GateFramework.dll")


// The indexes of each of the fields in our credential provider's tiles.
enum SAMPLE_FIELD_ID 
{
	SFI_TILEIMAGE      = 0,
	SFI_FIMTITLE       = 1,
	SFI_USERNAME       = 2,
	SFI_SUBMIT_BUTTON  = 3, 
	SFI_SAMPLE         = 3, 
	SFI_NUM_FIELDS     = 4, // Note: if new fields are added, keep NUM_FIELDS last.  This is used as a count of the number of fields
};

// The first value indicates when the tile is displayed (selected, not selected)
// the second indicates things like whether the field is enabled, whether it has key focus, etc.
struct FIELD_STATE_PAIR
{
	CREDENTIAL_PROVIDER_FIELD_STATE cpfs;
	CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE cpfis;
};

// These two arrays are seperate because a credential provider might
// want to set up a credential with various combinations of field state pairs 
// and field descriptors.

// The field state value indicates whether the field is displayed
// in the selected tile, the deselected tile, or both.
// The Field interactive state indicates when 
static const FIELD_STATE_PAIR s_rgFieldStatePairs[] = 
{
	{ CPFS_DISPLAY_IN_BOTH,          CPFIS_NONE    }, // SFI_TILEIMAGE
	{ CPFS_DISPLAY_IN_BOTH,          CPFIS_NONE    }, // SFI_FIMTITLE
	//{ CPFS_DISPLAY_IN_SELECTED_TILE, CPFIS_FOCUSED }, // SFI_USERNAME
	{ CPFS_HIDDEN, CPFIS_NONE }, // SFI_USERNAME
	{ CPFS_HIDDEN, CPFIS_NONE    }, // SFI_SUBMIT_BUTTON   
	//{ CPFS_DISPLAY_IN_SELECTED_TILE, CPFIS_NONE    }, // SFI_SUBMIT_BUTTON   
	{ CPFS_DISPLAY_IN_SELECTED_TILE, CPFIS_NONE    }, // SFI_SAMPLE   
};

// Field descriptors for unlock and logon.
// The first field is the index of the field.
// The second is the type of the field.
// The third is the name of the field, NOT the value which will appear in the field.
static const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR s_rgCredProvFieldDescriptors[] =
{
	{ SFI_TILEIMAGE,     CPFT_TILE_IMAGE,    L"Image"                   }, // SFI_TILEIMAGE
	{ SFI_FIMTITLE,      CPFT_LARGE_TEXT,    L"FIM Title"               }, // SFI_FIMTITLE
	{ SFI_USERNAME,      CPFT_EDIT_TEXT,     L"Domain\\Username or UPN" }, // SFI_USERNAME
	{ SFI_SUBMIT_BUTTON, CPFT_SUBMIT_BUTTON, L"Submit"                  }, // SFI_SUBMIT_BUTTON
	{ SFI_SAMPLE,        CPFT_SMALL_TEXT,    L"Sample"                  }, // SFI_SAMPLE
};