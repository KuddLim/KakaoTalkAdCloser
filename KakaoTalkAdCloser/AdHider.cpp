#include "stdafx.h"
#include "AdHider.h"

#include <set>

using namespace kudd;

namespace {
    bool sameSize(const RECT& r1, const RECT& r2)
    {
        return (r1.top - r1.bottom == r2.top - r2.bottom) &&
            (r1.left - r1.right == r2.left - r2.right);
    };

    BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
    {
        auto* ctx = reinterpret_cast<AdCloseContext*>(lParam);

        static std::set<std::wstring> captions;
        wchar_t className[256] = { 0, };
        wchar_t caption[256] = { 0, };
        wchar_t prevCaption[256] = { 0, };

        GetClassName(hwnd, className, 256);
        GetWindowText(hwnd, caption, 256);

        //CString cname;
        //cname.Format(L"[%s] %s\n", caption, className);
        //TRACE(cname);

        const std::wstring classNameStr(className);
        const std::wstring captionStr(caption);

        if (classNameStr == L"EVA_Window" && captionStr.empty()) {
            ctx->ad.found = true;
            ctx->ad.hwnd = hwnd;
        }

        auto findView = [&](const std::wstring& name, auto& state) {
            if (!state.found && std::wstring(caption).find(name) != std::wstring::npos) {
                state.found = true;
                state.hwnd = hwnd;
            }
        };

        findView(L"LockModeView_", ctx->lockModeView);
        findView(L"OnlineMainView", ctx->onlineView);

        return TRUE;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////

AdHider::~AdHider()
{
    cleanup();
}

void AdHider::startup()
{
    if (_hiderThread.get() == nullptr) {
        _hiderThread.reset(new std::thread([&] {
            _running = true;
            threadProc();
        }));
    }
}

void AdHider::cleanup()
{
    if (_hiderThread.get() && _running) {
        _running = false;
        _hiderThread->join();
    }
}

void AdHider::checkKakaoTalkAd()
{
    HWND handle = FindWindow(nullptr, L"카카오톡");
    if (handle == nullptr) {
        handle = FindWindow(nullptr, L"KakaoTalk");
    }

    if (handle == nullptr) {
        _adCloseCtx.reset();
        return;
    }

    // 카카오톡이 더 나중에 실행되거나, Kakaotalk 핸들이 변경되는 경우,
    // 초기화가 완료 될 때까지 잠시간 대기한다.
    // Kakaotalk 핸들이 변경되는 경우는 우선 현재까지 알아낸 순서는 다음과 같다:
    //  1. 카카오톡 실행, 로그인.
    //  2. 카카오톡 로그아웃.
    //  3. AdCloser 실행.
    //  4. 카카오톡 로그인.
    if (_adCloseCtx.get() == nullptr || _adCloseCtx->kakaoTalkHwnd != handle) {
        int32_t sleepCount = 10 * _kakaoWaitSeconds;

        while (_running && --sleepCount) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        _adCloseCtx = std::make_unique<AdCloseContext>(handle);
    }

    EnumChildWindows(handle, EnumWindowsProc, reinterpret_cast<LPARAM>(_adCloseCtx.get()));

    closeAndResize();

    ++_adCloseCtx->loopCount;
}

void AdHider::closeAndResize()
{
    AdCloseContext* ctx = _adCloseCtx.get();

    const int32_t REDUCE_WIDTH = 100;

    auto resizeView = [REDUCE_WIDTH](HWND parent, auto& state, bool addSize = true) {
        RECT rect;
        GetWindowRect(state.hwnd, &rect);
        MapWindowPoints(HWND_DESKTOP, parent, (LPPOINT)&rect, 2);

        bool underneath = rect.bottom == rect.top;

        if (!underneath && (state.lastKnownSize.bottom == 0 || !sameSize(rect, state.lastKnownSize))) {
            int32_t width = std::abs(rect.right - rect.left);
            int32_t height = std::abs(rect.bottom - rect.top) + (addSize ? REDUCE_WIDTH : 0);

            state.lastKnownSize.left = rect.left;
            state.lastKnownSize.right = rect.right;
            state.lastKnownSize.top = rect.top;
            state.lastKnownSize.bottom = rect.top + height;

            MoveWindow(state.hwnd, rect.left, rect.top, width, height, TRUE);
        }
    };

    if (ctx->ad.found && ctx->loopCount == 0) {
        RECT rect;
        memset(&rect, 0, sizeof(rect));
        GetWindowRect(ctx->ad.hwnd, &rect);
        MapWindowPoints(HWND_DESKTOP, ctx->kakaoTalkHwnd, (LPPOINT)&rect, 2);

        if (rect.top != 0) {
            _adCloseCtx->adRect = rect;

            int32_t width = std::abs(rect.right - rect.left);
            int32_t height = std::abs(rect.bottom - rect.top) - REDUCE_WIDTH;

            SetWindowPos(ctx->ad.hwnd, nullptr, _adCloseCtx->adRect.left, _adCloseCtx->adRect.bottom - REDUCE_WIDTH, width, height, SWP_SHOWWINDOW);
            SendMessage(ctx->ad.hwnd, WM_CLOSE, 0, 0);
        }

        resizeView(ctx->kakaoTalkHwnd, ctx->lockModeView);
        resizeView(ctx->kakaoTalkHwnd, ctx->onlineView);
    }
    else {
        resizeView(ctx->kakaoTalkHwnd, ctx->lockModeView, false);
        resizeView(ctx->kakaoTalkHwnd, ctx->onlineView);
    }
}

void AdHider::threadProc()
{
    while (_running) {
        checkKakaoTalkAd();

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}