#include "stdafx.h"
#include "Settings.h"

using namespace kudd;

namespace {
    const CString CATEGORY_SETTINGS(L"Settings");
    const CString ENTRY_RUNMINIMIZED(L"RunMinimized");
    const CString ENTRY_CHECKVER_ON_START(L"CheckVersionOnStart");
    const CString ENTRY_KAKAOTALK_WAIT_SECONDS(L"KakaoTalkWaitSeconds");

    const std::wstring APP_NAME(L"KakaoTalkAdCloser");
    const std::wstring KEY_PATH(L"Software\\Microsoft\\Windows\\CurrentVersion\\Run");
}

Settings& Settings::get()
{
    static Settings __instance__;
    return __instance__;
}

Settings::Settings()
{
    loadAll();
}

void Settings::loadAll()
{
    _checkUpdate = AfxGetApp()->GetProfileInt(CATEGORY_SETTINGS, ENTRY_CHECKVER_ON_START, 0) != 0;
    _runMinimized = AfxGetApp()->GetProfileInt(CATEGORY_SETTINGS, ENTRY_RUNMINIMIZED, 0) != 0;
    _kakaoWaitTime = AfxGetApp()->GetProfileInt(CATEGORY_SETTINGS, ENTRY_KAKAOTALK_WAIT_SECONDS, DEFAULT_KAKAO_WAIT_SECONDS);


    LSTATUS result = ERROR_SUCCESS;

    HKEY theKey = nullptr;
    result = RegOpenKey(HKEY_CURRENT_USER, KEY_PATH.c_str(), &theKey);

    if (result == ERROR_SUCCESS) {
        TCHAR path[MAX_PATH];
        ::GetModuleFileName(nullptr, path, MAX_PATH);

        DWORD dataType = 0;
        DWORD dataSize = MAX_PATH;

        TCHAR value[MAX_PATH] = { 0, };
        LSTATUS result = RegGetValue(theKey, nullptr, APP_NAME.c_str(), RRF_RT_REG_SZ, &dataType, value, &dataSize);

        _runWindowsStartup  = (wcscmp(path, value) == 0);
    }
}

void Settings::setRunWindowsStartup(bool b)
{
    _runWindowsStartup = b;

    LSTATUS result = ERROR_SUCCESS;

    HKEY theKey = nullptr;
    result = RegOpenKey(HKEY_CURRENT_USER, KEY_PATH.c_str(), &theKey);

    if (_runWindowsStartup) {
        if (result == ERROR_SUCCESS) {
            result = RegCreateKey(HKEY_CURRENT_USER, KEY_PATH.c_str(), &theKey);
        }

        if (result == ERROR_SUCCESS) {
            TCHAR path[MAX_PATH];
            ::GetModuleFileName(nullptr, path, MAX_PATH);
            RegSetValueEx(theKey, APP_NAME.c_str(), 0, REG_SZ, (BYTE*)path, static_cast<DWORD>((wcslen(path) + 1) * 2));
        }
    }
    else {
        if (result == ERROR_SUCCESS) {
            RegDeleteKeyValue(HKEY_CURRENT_USER, KEY_PATH.c_str(), APP_NAME.c_str());
        }
    }
}

void Settings::setRunMinimized(bool b)
{
    _runMinimized = b;
    AfxGetApp()->WriteProfileInt(CATEGORY_SETTINGS, ENTRY_RUNMINIMIZED, _runMinimized ? 1 : 0);
}

void Settings::setCheckUpdate(bool b)
{
    _checkUpdate = b;
    AfxGetApp()->WriteProfileInt(CATEGORY_SETTINGS, ENTRY_CHECKVER_ON_START, _checkUpdate ? 1 : 0);

}

void Settings::setKakaoWaitTime(int32_t i)
{
    _kakaoWaitTime = i;
    AfxGetApp()->WriteProfileInt(CATEGORY_SETTINGS, ENTRY_KAKAOTALK_WAIT_SECONDS, _kakaoWaitTime);
}
