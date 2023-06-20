// ProjectDlg.h : header file
//

#if !defined(AFX_PROJECTDLG_H__2CDA12CB_672F_11D4_AE1B_00A0CC53AB80__INCLUDED_)
#define AFX_PROJECTDLG_H__2CDA12CB_672F_11D4_AE1B_00A0CC53AB80__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CProjectDlg dialog

class CProjectDlg : public CDialog
{
// Construction
public:
	long OnWMApp(unsigned int, long);
	int m_AnimFrame;
	CBitmap *m_hbShip[4];
	int m_nTimer;
	CProjectDlg(CWnd* pParent = NULL);	// standard constructor
	~CProjectDlg();

// Dialog Data
	//{{AFX_DATA(CProjectDlg)
	enum { IDD = IDD_PROJECT_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProjectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CProjectDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROJECTDLG_H__2CDA12CB_672F_11D4_AE1B_00A0CC53AB80__INCLUDED_)
