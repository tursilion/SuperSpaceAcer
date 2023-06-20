// Project.h : main header file for the PROJECT application
//

#if !defined(AFX_PROJECT_H__2CDA12C9_672F_11D4_AE1B_00A0CC53AB80__INCLUDED_)
#define AFX_PROJECT_H__2CDA12C9_672F_11D4_AE1B_00A0CC53AB80__INCLUDED_

#include "frmGame.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CProjectApp:
// See Project.cpp for the implementation of this class
//

class CProjectApp : public CWinApp
{
public:
	CProjectApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProjectApp)
	public:
	virtual BOOL InitInstance();
	virtual int DoMessageBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt);
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CProjectApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROJECT_H__2CDA12C9_672F_11D4_AE1B_00A0CC53AB80__INCLUDED_)
