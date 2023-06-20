#if !defined(AFX_TESTSCREEN_H__F73DEDA3_7145_11D4_AE1B_00A0CC53AB80__INCLUDED_)
#define AFX_TESTSCREEN_H__F73DEDA3_7145_11D4_AE1B_00A0CC53AB80__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TestScreen.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// TestScreen dialog

class TestScreen : public CDialog
{
// Construction
public:
	LPD3DXCONTEXT pCtx;
	UINT m_Timer;
	D3DX_VIDMODEDESC VidMode;
	DWORD ret;
	TestScreen(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(TestScreen)
	enum { IDD = IDD_TESTSCREEN_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TestScreen)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(TestScreen)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTSCREEN_H__F73DEDA3_7145_11D4_AE1B_00A0CC53AB80__INCLUDED_)
