
// RX Model Viewer.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CRXModelViewerApp:
// See RX Model Viewer.cpp for the implementation of this class
//

class CRXModelViewerApp : public CWinApp
{
public:
	CRXModelViewerApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CRXModelViewerApp theApp;