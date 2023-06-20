// ProjectDlg.cpp : implementation file
// Since it manipulates the global "theGame"
// It's pupose is to load and init the game,
// while displaying an animated little window
//

#include "stdafx.h"
#include <d3drmwin.h>
#include <process.h>
#include <atlbase.h>
#include "Project.h"
#include "ProjectDlg.h"
#include "GameApp.h"
#include "frmsettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CGameApp *theGame;
void __cdecl GameInitThread(void *data);
CProjectDlg *thisDlg=NULL;

/////////////////////////////////////////////////////////////////////////////
// CProjectDlg dialog

CProjectDlg::CProjectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProjectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProjectDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CProjectDlg::~CProjectDlg() {
}
	

void CProjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProjectDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CProjectDlg, CDialog)
	//{{AFX_MSG_MAP(CProjectDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_APP, OnWMApp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProjectDlg message handlers

BOOL CProjectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	thisDlg=this;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CProjectDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CProjectDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CProjectDlg::PreCreateWindow(CREATESTRUCT& cs) 
{
	return CDialog::PreCreateWindow(cs);
}

void CProjectDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	m_hbShip[0]=new CBitmap;
	m_hbShip[1]=new CBitmap;
	m_hbShip[2]=new CBitmap;
	m_hbShip[3]=new CBitmap;
	
	m_hbShip[0]->LoadBitmap(MAKEINTRESOURCE(IDB_Ship1));
	m_hbShip[1]->LoadBitmap(MAKEINTRESOURCE(IDB_Ship2));
	m_hbShip[2]->LoadBitmap(MAKEINTRESOURCE(IDB_Ship3));
	m_hbShip[3]->LoadBitmap(MAKEINTRESOURCE(IDB_Ship4));
	m_AnimFrame=0;
	m_nTimer=SetTimer(1, 150, NULL);

	_beginthread(GameInitThread, 0, NULL);
}

void CProjectDlg::OnDestroy() 
{
	if (0!=m_nTimer) {
		KillTimer(m_nTimer);
	}

	delete m_hbShip[0];
	delete m_hbShip[1];
	delete m_hbShip[2];
	delete m_hbShip[3];

	CDialog::OnDestroy();
}

void CProjectDlg::OnTimer(UINT nIDEvent) 
{
	HWND myWnd;
	HDC myDC, srcDC;

	m_AnimFrame++;
	if (m_AnimFrame>3) m_AnimFrame=0;

	GetDlgItem( IDC_Anim, &myWnd);
	if (myWnd) {
		myDC=::GetDC(myWnd);
		if (myDC) {
			srcDC = ::CreateCompatibleDC(myDC);
			::SelectObject(srcDC, m_hbShip[m_AnimFrame]->m_hObject);
			::BitBlt(myDC, 0, 0, 125, 139, srcDC, 0, 0, SRCCOPY);
			::ReleaseDC(myWnd, myDC);
			::DeleteDC(srcDC);
		}
	}

	CDialog::OnTimer(nIDEvent);
}

void __cdecl GameInitThread(void *data) {
	int ret;

	theGame=new CGameApp;
	
	ret=theGame->InitInstance();

	if (-1 == ret) {
		AfxMessageBox("We have detected that this is the first time you have run\nSuper Space Acer, or that your video settings have changed.\nPlease select the correct video device on the next screen.");
	}

	while (-1==ret) {					// need a D3D hardware device to use
		thisDlg->SendMessage(WM_APP);	// wait for device selection to occur
		ret=theGame->InitInstance();	// try to initialize again
	}

	if (0==ret)	{	// initialize..
		OutputDebugString("Game object failed to initialize\n");
		delete theGame;
		theGame=NULL;
	}

	// Now kill off this object and end this thread
	if (thisDlg != NULL) {
		thisDlg->EndDialog(IDOK);
	}
}

long CProjectDlg::OnWMApp(unsigned int, long)
{
	// We get this message when we're supposed to query
	// the user about which hardware device he wants to use

	frmSettings *frm;
	int ret;
	CRegKey *myKey;

	frm=new frmSettings;
	ret=frm->DoModal();

	if (-1==ret) {
		if (NULL != theGame) {
			delete theGame;
			theGame=NULL;
		}
		EndDialog(IDCANCEL);
	}
	
	// otherwise, let's write this value to the registry so
	// that the game thread can find it.
	myKey=new CRegKey;

	if (ERROR_SUCCESS==myKey->Create(HKEY_LOCAL_MACHINE, REGKEY)) {
		myKey->SetValue(frm->m_Device, "D3DDevice");
		myKey->SetValue(DeviceList[ret].driverDesc, "D3DDeviceName");
		myKey->SetValue(frm->m_Mode, "D3DMode");
		myKey->Close();
	}

	delete myKey;
	delete frm;
	return 0;
}
