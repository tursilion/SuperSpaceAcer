// GameApp.h: interface for the CGameApp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMEAPP_H__42CBF703_6DF7_11D4_AE1B_00A0CC53AB80__INCLUDED_)
#define AFX_GAMEAPP_H__42CBF703_6DF7_11D4_AE1B_00A0CC53AB80__INCLUDED_

#include "frmGame.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CGameApp  
{
public:
	frmGame *Game;
	int InitInstance();
	int m_Status;

	void Debug(char *s, ...);
	void CleanUp();
	void Begin();
	CGameApp();
	virtual ~CGameApp();
};

#endif // !defined(AFX_GAMEAPP_H__42CBF703_6DF7_11D4_AE1B_00A0CC53AB80__INCLUDED_)
