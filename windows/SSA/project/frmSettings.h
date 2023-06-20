#if !defined(AFX_FRMSETTINGS_H__85A33981_706D_11D4_AE1B_00A0CC53AB80__INCLUDED_)
#define AFX_FRMSETTINGS_H__85A33981_706D_11D4_AE1B_00A0CC53AB80__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// frmSettings.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// frmSettings dialog

class frmSettings : public CDialog
{
// Construction
public:
	DWORD m_Mode;
	DWORD m_Device;
	BOOL OnInitDialog();
	frmSettings(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(frmSettings)
	enum { IDD = IDD_FRMSETTINGS_DIALOG };
	CListCtrl	m_List2;
	CListCtrl	m_List1;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(frmSettings)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(frmSettings)
	virtual void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnClickList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTest();
	afx_msg void OnHelp();
	afx_msg void OnmyHELP();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FRMSETTINGS_H__85A33981_706D_11D4_AE1B_00A0CC53AB80__INCLUDED_)
