// frmHelp.cpp : implementation file
//

#include "stdafx.h"
#include "Project.h"
#include "frmHelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// frmHelp dialog


frmHelp::frmHelp(CWnd* pParent /*=NULL*/)
	: CDialog(frmHelp::IDD, pParent)
{
	//{{AFX_DATA_INIT(frmHelp)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void frmHelp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(frmHelp)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(frmHelp, CDialog)
	//{{AFX_MSG_MAP(frmHelp)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// frmHelp message handlers
