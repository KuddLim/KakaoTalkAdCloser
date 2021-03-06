
// KakaoTalkAdCloserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "KakaoTalkAdCloser.h"
#include "KakaoTalkAdCloserDlg.h"
#include "afxdialogex.h"
#include "AdHider.h"
#include "VersionChecker.h"
#include "Version.h"
#include "Settings.h"

#include <cstdint>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {
    const uint32_t UM_TRAYICON_MSG = WM_USER + 100;
}

using namespace kudd;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CKakaoTalkAdCloserDlg dialog



CKakaoTalkAdCloserDlg::CKakaoTalkAdCloserDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_KAKAOTALKADCLOSER_DIALOG, pParent)
    , _forceClose(false)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CKakaoTalkAdCloserDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CKakaoTalkAdCloserDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_DESTROY()
    ON_MESSAGE(UM_TRAYICON_MSG, OnTrayIconMsg)
    ON_COMMAND(ID_CONTEXTMENU_SHOW, OnShowDlg)
    ON_COMMAND(ID_CONTEXTMENU_EXIT, OnExitHider)
    ON_BN_CLICKED(IDC_CHECK_RUN_ON_WINDOWS_STARTUP, OnRunOnStartup)
    ON_BN_CLICKED(IDC_CHECK_RUN_MINIMIZED, OnRunMinimized)
    ON_BN_CLICKED(IDC_CHECK_CHECK_VERSION_ON_STARTUP, OnCheckVersionOnStartup)
    ON_BN_CLICKED(IDC_BUTTON_KAKAO_WAITTIME_INFO_BOX, OnKakaoWaitTimeChangeHelp)
    ON_CBN_SELCHANGE(IDC_COMBO_WAIT_SECONDS, OnKakaoWaitTimeChange)
    ON_WM_CLOSE()
    ON_WM_QUERYENDSESSION()
END_MESSAGE_MAP()


// CKakaoTalkAdCloserDlg message handlers

BOOL CKakaoTalkAdCloserDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != nullptr)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);         // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    // TODO: Add extra initialization here

    initUi();

    if (Settings::get().checkUpdate() && checkVersion()) {
        _forceClose = true;
        PostQuitMessage(0);
        return TRUE;
    }

    _adHider.reset(new AdHider(Settings::get().kakaoWaitTime()));
    _adHider->startup();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CKakaoTalkAdCloserDlg::initUi()
{
    registerTrayIcon();

    SetWindowText(L"" VERSION_NO);

    auto& s = Settings::get();

    CButton* check = static_cast<CButton*>(GetDlgItem(IDC_CHECK_RUN_ON_WINDOWS_STARTUP));
    check->SetWindowText(L"윈도우 시작시 실행");
    check->SetCheck(s.runWindowsStartup() ? BST_CHECKED : BST_UNCHECKED);

    check = static_cast<CButton*>(GetDlgItem(IDC_CHECK_RUN_MINIMIZED));
    check->SetWindowText(L"최소화 상태로 실행");
    check->SetCheck(s.runMinimized() ? BST_CHECKED : BST_UNCHECKED);

    check = static_cast<CButton*>(GetDlgItem(IDC_CHECK_CHECK_VERSION_ON_STARTUP));
    check->SetWindowText(L"실행시 새 버전 체크");
    check->SetCheck(s.checkUpdate() ? BST_CHECKED : BST_UNCHECKED);

    CString label(L"카카오톡 실행 대기시간 (초)");
    CStatic* staticText = static_cast<CStatic*>(GetDlgItem(IDC_STATIC_WAIT_SECONDS));
    staticText->SetWindowText(label);

    CClientDC dc(this);
    CFont* font = staticText->GetFont();
    CFont* oldFont = dc.SelectObject(font);
    CSize textSize = dc.GetTextExtent(label, label.GetLength());
    dc.SelectObject(oldFont);

    CRect textRect;
    staticText->GetWindowRect(textRect);
    ScreenToClient(textRect);
    staticText->MoveWindow(textRect.left, textRect.top, textSize.cx, textSize.cy);

    CRect comboRect;
    auto* cmb = static_cast<CComboBox*>(GetDlgItem(IDC_COMBO_WAIT_SECONDS));
    cmb->GetClientRect(comboRect);

    int32_t x = textRect.left + textSize.cx + 5;
    int32_t y = textRect.top - 5;
    cmb->MoveWindow(x, y, comboRect.Width(), comboRect.Height());

    CRect buttonRect;
    auto* button = static_cast<CButton*>(GetDlgItem(IDC_BUTTON_KAKAO_WAITTIME_INFO_BOX));
    button->GetClientRect(buttonRect);
    x += (comboRect.Width() + 2);
    button->MoveWindow(x, y, buttonRect.Width(), buttonRect.Height());

    int32_t sel = -1, index = 0;

    CString fmt;
    for (int32_t i = 5; i <= MAX_KAKAO_WAIT_SECONDS; ++i) {
        fmt.Format(L"%d", i);
        index = cmb->InsertString(-1, fmt);
        cmb->SetItemData(index, i);

        if (i == s.kakaoWaitTime()) {
            sel = index;
        }
    }
    cmb->SetCurSel(sel >= 0 ? sel : index);

    if (s.runMinimized()) {
        PostMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
    }
}

bool CKakaoTalkAdCloserDlg::checkVersion()
{
    if (VersionChecker::get().check(LAST_KNOWN_UPDATE)) {
        if (AfxMessageBox(L"최신 버전이 있습니다. 다운로드 하시겠습니까?", MB_YESNO) == IDYES) {
            // TODO: open browser
            ShellExecute(0, NULL, L"https://github.com/KuddLim/KakaoTalkAdCloser/releases", NULL, NULL, SW_SHOWDEFAULT);
            return true;
        }
    }

    return false;
}

void CKakaoTalkAdCloserDlg::registerTrayIcon()
{
    NOTIFYICONDATA  nid;
    nid.cbSize = sizeof(nid);
    nid.hWnd = m_hWnd;
    nid.uID = IDR_MAINFRAME;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = UM_TRAYICON_MSG;
    nid.hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    CString title;
    GetWindowText(title);

    wcsncpy_s(nid.szTip, (LPCTSTR)title, 128);
    Shell_NotifyIcon(NIM_ADD, &nid);
    SendMessage(WM_SETICON, (WPARAM)TRUE, (LPARAM)nid.hIcon);
}

void CKakaoTalkAdCloserDlg::deRegisterTrayIcon()
{
    NOTIFYICONDATA  nid;
    nid.cbSize = sizeof(nid);
    nid.hWnd = m_hWnd;
    nid.uID = IDR_MAINFRAME;

    Shell_NotifyIcon(NIM_DELETE, &nid);
}

void CKakaoTalkAdCloserDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else if (nID == SC_MINIMIZE) {
        //_notifyDlg->ShowWindow(SW_HIDE);
        ShowWindow(SW_HIDE);
    }
    else
    {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CKakaoTalkAdCloserDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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
        CDialogEx::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CKakaoTalkAdCloserDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CKakaoTalkAdCloserDlg::OnClose()
{
    // TODO: Add your message handler code here and/or call default
    if (_forceClose || AfxMessageBox(L"정말 종료하시겠어요?", MB_YESNO) == IDYES) {
        CDialogEx::OnClose();
    }
}

BOOL CKakaoTalkAdCloserDlg::OnQueryEndSession()
{
    CDialogEx::OnClose();

    return TRUE;
}

void CKakaoTalkAdCloserDlg::OnDestroy()
{
    if (_adHider.get()) {
        _adHider->cleanup();
    }

    deRegisterTrayIcon();

    CDialogEx::OnDestroy();

    // TODO: Add your message handler code here
}

BOOL CKakaoTalkAdCloserDlg::PreTranslateMessage(MSG* pMsg)
{
    // TODO: Add your specialized code here and/or call the base class
    if (pMsg->message == WM_KEYDOWN) {
        switch (pMsg->wParam) {
        case VK_ESCAPE:
        case VK_RETURN:
            return TRUE;
        default:
            break;
        }
    }

    return CDialogEx::PreTranslateMessage(pMsg);
}

LRESULT CKakaoTalkAdCloserDlg::OnTrayIconMsg(WPARAM wParam, LPARAM lParam)
{
    switch (lParam) {
    case WM_LBUTTONDBLCLK:
        ShowWindow(SW_SHOW);
        break;
    case WM_RBUTTONDOWN:
        showContextMenu();
        break;
    default:
        break;
    }
    return 0L;
}

void CKakaoTalkAdCloserDlg::showContextMenu()
{
    CMenu menu;
    menu.LoadMenu(IDR_MENU_CONTEXT);

    CPoint pt;
    ::GetCursorPos(&pt);

    CMenu* contextMenu = menu.GetSubMenu(0);
    contextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
}

void CKakaoTalkAdCloserDlg::OnShowDlg()
{
    ShowWindow(SW_SHOW);
}

void CKakaoTalkAdCloserDlg::OnExitHider()
{
    if (AfxMessageBox(L"정말 종료하시겠어요?", MB_YESNO) == IDYES) {
        _forceClose = true;
        PostQuitMessage(0);
    }
}

void CKakaoTalkAdCloserDlg::OnRunOnStartup()
{
#if !defined(_DEBUG)
    auto* btn = static_cast<CButton*>(GetDlgItem(IDC_CHECK_RUN_ON_WINDOWS_STARTUP));
    Settings::get().setRunWindowsStartup(btn->GetCheck() == BST_CHECKED);
#endif
}

void CKakaoTalkAdCloserDlg::OnRunMinimized()
{
    auto* btn = static_cast<CButton*>(GetDlgItem(IDC_CHECK_RUN_MINIMIZED));
    Settings::get().setRunMinimized(btn->GetCheck() == BST_CHECKED);
}

void CKakaoTalkAdCloserDlg::OnCheckVersionOnStartup()
{
    auto* btn = static_cast<CButton*>(GetDlgItem(IDC_CHECK_CHECK_VERSION_ON_STARTUP));
    Settings::get().setCheckUpdate(btn->GetCheck() == BST_CHECKED);
}

void CKakaoTalkAdCloserDlg::OnKakaoWaitTimeChangeHelp()
{
    CString info = L"카카오톡이 실행완료되기 전에 프로그램이 광고창을 닫으려고 하는 경우, \
카카오톡이 비정상 상태로 바뀌는 경우가 있어 일정시간 대기 후에 광고창을 닫아야 합니다. \
PC 성능이 좋을 수록 낮은 값을 사용하시면 됩니다.\n\n\
혹시 카카오톡 대화창이 먹통 된 것 처럼 보이면 트레이 아이콘에서 우클릭 후, 카카오톡을 종료 후 다시 실행하세요.";

    AfxMessageBox(info, MB_ICONINFORMATION);
}

void CKakaoTalkAdCloserDlg::OnKakaoWaitTimeChange()
{
    auto* cmb = static_cast<CComboBox*>(GetDlgItem((IDC_COMBO_WAIT_SECONDS)));
    int32_t sel = cmb->GetCurSel();

    if (sel >= 0) {
        int32_t seconds = static_cast<int32_t>(cmb->GetItemData(sel));
        Settings::get().setKakaoWaitTime(seconds);
        _adHider->setKakaoWaitSeconds(seconds);
    }
}