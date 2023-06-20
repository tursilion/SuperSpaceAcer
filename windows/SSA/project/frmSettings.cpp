// frmSettings.cpp : implementation file
//

#include "stdafx.h"
#include "Project.h"
#include "frmSettings.h"
#include "gameapp.h"
#include "testscreen.h"
#include "frmhelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CGameApp *theGame;
/////////////////////////////////////////////////////////////////////////////
// frmSettings dialog


frmSettings::frmSettings(CWnd* pParent /*=NULL*/)
	: CDialog(frmSettings::IDD, pParent)
{
	//{{AFX_DATA_INIT(frmSettings)
	//}}AFX_DATA_INIT
}


void frmSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(frmSettings)
	DDX_Control(pDX, IDC_LIST2, m_List2);
	DDX_Control(pDX, IDC_LIST1, m_List1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(frmSettings, CDialog)
	//{{AFX_MSG_MAP(frmSettings)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, OnClickList1)
	ON_BN_CLICKED(IDC_Test, OnTest)
	ON_BN_CLICKED(IDC_Cancel, OnCancel)
	ON_BN_CLICKED(IDC_myHELP, OnmyHELP)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// frmSettings message handlers

void frmSettings::OnOK() 
{
	POSITION pos, pos2;
	DWORD ret, ret2, tmp;

	UpdateData(true);

	pos=m_List1.GetFirstSelectedItemPosition();
	ret=m_List1.GetNextSelectedItem(pos);
	
	if (-1 == ret) {
		AfxMessageBox("You must select a device, and a mode.");
		return;
	}

	pos2=m_List2.GetFirstSelectedItemPosition();
	ret2=m_List2.GetNextSelectedItem(pos2);
	tmp=m_List2.GetItemData(ret2);

	if (-1 == ret2) {
		AfxMessageBox("You select a mode.");
		return;
	}

	m_Device=ret;
	m_Mode=tmp;

	theGame->Debug("Selected Item: %d, mode %d\n", ret, ret2);

	EndDialog(ret);
}

void frmSettings::OnCancel() 
{
	AfxMessageBox("Game Initialization Aborted");
	EndDialog(-1);
}

BOOL frmSettings::OnInitDialog()
{
	DWORD idx;

	CDialog::OnInitDialog();
	
	// fill in the list with the detected hardware devices
	// Naturally the list must have been filled in before calling here

	for (idx=0; idx<dwNumDevices; idx++) {
		m_List1.InsertItem(idx, DeviceList[idx].driverDesc, 0);
	}

	UpdateData(false);	// update box

	return TRUE;
}

void frmSettings::OnClickList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	DWORD ret, idx, tmp;
	POSITION pos;
	DWORD dwNumVidModes;
    D3DX_VIDMODEDESC VidMode;
	char szBuf[256];

	UpdateData(true);	// get selection

	pos=m_List1.GetFirstSelectedItemPosition();
	ret=m_List1.GetNextSelectedItem(pos);

    dwNumVidModes = D3DXGetMaxNumVideoModes( ret, D3DX_GVM_REFRESHRATE );
 
	m_List2.DeleteAllItems();

    for( idx = 0; idx<dwNumVidModes; idx++ )
    {
        if (S_OK == D3DXGetVideoMode( ret, 0, idx, &VidMode )) {
			if (VidMode.bpp != 8) {
				sprintf(szBuf, "%d x %d, %d bits", VidMode.width, VidMode.height, VidMode.bpp);
				tmp=m_List2.InsertItem(idx, szBuf, 0);
				m_List2.SetItemData(tmp, idx);
			}
		} else {
			break;
		}
    }

	UpdateData(false);

	*pResult = 0;
}

void frmSettings::OnTest() 
{
	POSITION pos, pos2;
	DWORD ret, ret2, tmp;
    D3DX_VIDMODEDESC VidMode;
	TestScreen *tst;

	UpdateData(true);

	pos=m_List1.GetFirstSelectedItemPosition();
	ret=m_List1.GetNextSelectedItem(pos);
	
	if (-1 == ret) {
		AfxMessageBox("You must first select a device, and a mode.");
		return;
	}

	pos2=m_List2.GetFirstSelectedItemPosition();
	ret2=m_List2.GetNextSelectedItem(pos2);

	if (-1 == ret2) {
		AfxMessageBox("You must first select a mode to test.");
		return;
	}

	AfxMessageBox("You will see a simple graphic and some text\non a full-screen form. If you can not see it,\nyou need to try a different mode.\n\nWait 10 seconds after clicking OK and this form will return.");

	tmp=m_List2.GetItemData(ret2);
    D3DXGetVideoMode( ret, 0, tmp, &VidMode );

	tst=new TestScreen;
	tst->ret=ret;
	memcpy(&tst->VidMode, &VidMode, sizeof(VidMode));
	if (-1 == tst->DoModal()) {
		AfxMessageBox("Unable to Initialize Selected Mode - Please choose another.");
	} else {
		AfxMessageBox("If you could not see the graphic, you must select a different mode.");
	}
	delete tst;
}

void frmSettings::OnmyHELP() 
{
	frmHelp *hlp;

	hlp=new frmHelp;
	hlp->DoModal();

	delete hlp;
}
