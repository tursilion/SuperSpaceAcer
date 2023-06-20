// frmGame.cpp : implementation file
// The primary class for this game
//

#include "stdafx.h"
#include "Project.h"
#include "frmGame.h"
#include "shlwapi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// frmGame dialog


frmGame::frmGame(CWnd* pParent /*=NULL*/)
	: CDialog(frmGame::IDD, pParent)
{
	//{{AFX_DATA_INIT(frmGame)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void frmGame::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(frmGame)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(frmGame, CDialog)
	//{{AFX_MSG_MAP(frmGame)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// frmGame message handlers

BOOL frmGame::OnInitDialog() 
{
	// Initialize game variables
	pCtx=NULL;
	nFirstCall=1;
	nTimer=0;

	// Set up - we'll start in half a second
	nTimer=SetTimer(2, 500, NULL);
	if (0 == nTimer) {
		AfxMessageBox("Initialization of game timer failed, free up\nresources and try again.");
		EndDialog(1);	// won't come back
	}

	CDialog::OnInitDialog();

	return TRUE;   // return TRUE unless you set the focus to a control
	               // EXCEPTION: OCX Property Pages should return FALSE
}

void frmGame::OnFirstCall()
{
	KillTimer(nTimer);	// stop the timer!

	if (S_OK!=D3DXCreateContextEx(dev, D3DX_CONTEXT_FULLSCREEN, m_hWnd, NULL, VidMode.bpp, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, VidMode.width, VidMode.height, D3DX_DEFAULT, &pCtx)) {
		AfxMessageBox("Unable to initialize graphics! Please try again, and select a new mode.", MB_OK | MB_ICONSTOP);
		SHDeleteKey(HKEY_LOCAL_MACHINE, REGKEY);
		EndDialog(1);
		return;
	}
	// Set up the game timer - 60hz clock (so about 17ms (which is, in turn, 58.8fps))
	// Set timer has an accuracy of 55ms - this is not useful
	// Instead, try timeSetEvent
	nTimer=SetTimer(2, 17, NULL);
}

void frmGame::OnTimer(UINT nIDEvent) 
{
	static int t=0;

	CDialog::OnTimer(nIDEvent);

	// Handle first frame initialization
	if (nFirstCall) {
		OutputDebugString("Doing first call....\n");
		OnFirstCall();
		nFirstCall=0;
		return;
	}

	t++;

	if (t>300) EndDialog(1);

}

void frmGame::Debug(char *s, ...)
{
	char szMessage[1024];
	va_list va;

	ZeroMemory(szMessage, 1024);
	va_start(va, s);
	_vsnprintf(szMessage, 1023, s, va);
	va_end(va);
	strcat(szMessage, "\n");
	
	OutputDebugString(szMessage);
}
