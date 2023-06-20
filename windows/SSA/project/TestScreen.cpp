// TestScreen.cpp : implementation file
//

#include "stdafx.h"
#include "Project.h"
#include "TestScreen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TestScreen dialog


TestScreen::TestScreen(CWnd* pParent /*=NULL*/)
	: CDialog(TestScreen::IDD, pParent)
{
	//{{AFX_DATA_INIT(TestScreen)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void TestScreen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TestScreen)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(TestScreen, CDialog)
	//{{AFX_MSG_MAP(TestScreen)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TestScreen message handlers

BOOL TestScreen::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_Timer=0;
	pCtx=NULL;

	if (S_OK==D3DXCreateContextEx(ret, D3DX_CONTEXT_FULLSCREEN, m_hWnd, NULL, VidMode.bpp, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, VidMode.width, VidMode.height, D3DX_DEFAULT, &pCtx)) {
		m_Timer=SetTimer(2, 8500, NULL);
	} else {
		EndDialog(-1);
	}

	return TRUE; // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void TestScreen::OnTimer(UINT nIDEvent) 
{
	if (0 != m_Timer) {
		KillTimer(m_Timer);
	}

	if (pCtx) {
		pCtx->Release();
	}

	EndDialog(IDOK);
}
