// NEWFLASHBURN.h : main header file for the NEWFLASHBURN application
//

#if !defined(AFX_NEWFLASHBURN_H__EA0E10E1_BE84_412A_B4B5_4F68A6749328__INCLUDED_)
#define AFX_NEWFLASHBURN_H__EA0E10E1_BE84_412A_B4B5_4F68A6749328__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CNEWFLASHBURNApp:
// See NEWFLASHBURN.cpp for the implementation of this class
//

class CNEWFLASHBURNApp : public CWinApp
{
public:
	CNEWFLASHBURNApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNEWFLASHBURNApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CNEWFLASHBURNApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWFLASHBURN_H__EA0E10E1_BE84_412A_B4B5_4F68A6749328__INCLUDED_)
