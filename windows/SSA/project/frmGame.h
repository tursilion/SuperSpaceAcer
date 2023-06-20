#if !defined(AFX_FRMGAME_H__F73DEDA5_7145_11D4_AE1B_00A0CC53AB80__INCLUDED_)
#define AFX_FRMGAME_H__F73DEDA5_7145_11D4_AE1B_00A0CC53AB80__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// frmGame.h : header file
//

#include "resource.h"
/////////////////////////////////////////////////////////////////////////////
// frmGame dialog

class frmGame : public CDialog
{
// Construction
public:
	void Debug(char *s, ...);
	void OnFirstCall();
	UINT nTimer;
	int nFirstCall;
	LPD3DXCONTEXT pCtx;
	D3DX_VIDMODEDESC VidMode;
	unsigned long dev;
	frmGame(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(frmGame)
	enum { IDD = IDD_FRMGAME_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(frmGame)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(frmGame)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FRMGAME_H__F73DEDA5_7145_11D4_AE1B_00A0CC53AB80__INCLUDED_)
