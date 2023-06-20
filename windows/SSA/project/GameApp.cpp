// GameApp.cpp: implementation of the CGameApp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <atlbase.h>
#include "Project.h"
#include "GameApp.h"
#include "frmsettings.h"
#include "projectdlg.h"
#include "frmGame.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern CProjectDlg *thisDlg;
extern CProjectApp theApp;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGameApp::CGameApp()
{
}

CGameApp::~CGameApp()
{
	D3DXUninitialize();
}

void CGameApp::Begin()
{
	// Begin the game itself - open the window, etc :)
	OutputDebugString("Beginning game...\n");
	Game->DoModal();
}


int CGameApp::InitInstance()
{
	// Initialize the game - load necesary files, prepare the
	// full-screen window (but don't activate it yet). This is
	// the init stuff done while the little space ship is
	// animating
	int ret;
	unsigned int idx;
	HRESULT hr;
	static int init=-1;
	unsigned long dev, mode;
	CRegKey *myKey;
	char devName[256];
	DWORD dwLen;
    D3DX_VIDMODEDESC VidMode;

	if (-1 == init) {
		init=1;

		ret=D3DXInitialize();

		if (S_OK != ret) {
			switch (ret) {
			case D3DXERR_NOMEMORY: AfxMessageBox("Not enough memory to start the 3D engine!"); break;
			case D3DXERR_STARTUPFAILED: AfxMessageBox("Unable to start the 3D engine!"); break;
			default: AfxMessageBox("Unknown error starting the 3D engine!"); break;
			}
			return 0;
		}

		// Enumerate available gfx devices
		dwNumDevices=D3DXGetDeviceCount();
		Debug("There are %d devices...", dwNumDevices);
		if (0 == dwNumDevices) return 0;

		if (dwNumDevices > 16) {
			Debug("%d gfx hardware devices detected - limiting to 16.\n", dwNumDevices);
			dwNumDevices=16;
			AfxMessageBox("Note: Only the first 16 graphics devices will be available.");
		}

		for (idx=0; idx<dwNumDevices; idx++) {
			hr=D3DXGetDeviceDescription(idx, &DeviceList[idx]);
			Debug("%2d: %-99s - %08x (%c)", idx, DeviceList[idx].driverDesc, DeviceList[idx].monitor, DeviceList[idx].onPrimary ? 'Y' : 'N');
		}
	}
	
	// Load default device from the registry. If it doesn't exist, we'll ask for it.
	// If it doesn't match an available one, we'll ask too.
	// any problems, we return -1 and will be called again.
	myKey=new CRegKey;

	dev=-1;

	if (ERROR_SUCCESS==myKey->Open(HKEY_LOCAL_MACHINE, REGKEY)) {
		myKey->QueryValue(dev, "D3DDevice");
		dwLen=256;
		myKey->QueryValue(devName, "D3DDeviceName", &dwLen);
		myKey->QueryValue(mode, "D3DMode");
		myKey->Close();
	}

	delete myKey;

	if ((dev >= dwNumDevices) || (dev < 0)) {
		return -1;
	}

	if (strcmp(DeviceList[dev].driverDesc, devName) != 0) {
		return -1;
	}

	// Initialize the graphics system with that device
    D3DXGetVideoMode(dev, 0, mode, &VidMode );

	Game=new frmGame;
	Game->dev=dev;
	memcpy(&Game->VidMode, &VidMode, sizeof(VidMode));
	theApp.m_pMainWnd=NULL;
	theApp.m_pActiveWnd=NULL;

	return 1;	// Ok to proceed
}

void CGameApp::Debug(char *s, ...)
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

void CGameApp::CleanUp() {
	delete Game;
}