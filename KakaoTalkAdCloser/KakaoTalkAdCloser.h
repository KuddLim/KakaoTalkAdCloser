
// KakaoTalkAdCloser.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
    #error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CKakaoTalkAdCloserApp:
// See KakaoTalkAdCloser.cpp for the implementation of this class
//

class CKakaoTalkAdCloserApp : public CWinApp
{
public:
    CKakaoTalkAdCloserApp();

// Overrides
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();

private:
    HANDLE _mutex;

// Implementation

    DECLARE_MESSAGE_MAP()
};

extern CKakaoTalkAdCloserApp theApp;
