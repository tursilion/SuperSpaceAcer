// Project.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include <d3drmwin.h>
#include "Project.h"
#include "ProjectDlg.h"
#include "GameApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProjectApp

BEGIN_MESSAGE_MAP(CProjectApp, CWinApp)
	//{{AFX_MSG_MAP(CProjectApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProjectApp construction

CProjectApp::CProjectApp()
{
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CProjectApp object

CProjectApp theApp;
CGameApp *theGame;		// This will be initialized in the modal dialog
bool bNetworkUp;
D3DX_DEVICEDESC DeviceList[16];
DWORD dwNumDevices;

/////////////////////////////////////////////////////////////////////////////
// CProjectApp initialization

BOOL CProjectApp::InitInstance()
{
	bNetworkUp=true;

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		bNetworkUp=false;
	}

	theGame=NULL;

	CProjectDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	if (NULL != theGame) {
		theGame->Begin();		// run the game, if correctly initialized
		theGame->CleanUp();		// Clean up when done
	}

	if (NULL != theGame) {
		delete theGame;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int CProjectApp::DoMessageBox(LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt) 
{
	strcpy((char*)m_pszAppName, "Super Space Acer");
	
	return CWinApp::DoMessageBox(lpszPrompt, nType, nIDPrompt);
}
