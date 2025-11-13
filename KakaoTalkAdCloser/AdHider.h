#pragma once

#include <thread>
#include <memory>
#include <atomic>
#include <map>
#include <string>

namespace kudd {
    struct ChildWindowInfo {
        std::wstring className;
        std::wstring caption;

        bool operator ==(const ChildWindowInfo& rhs) const
        {
            return className == rhs.className && caption == rhs.caption;
        }
        bool operator !=(const ChildWindowInfo& rhs) const
        {
            return !operator ==(rhs);
        }
    };

    struct AdCloseContext {
        struct WindowShownState {
            WindowShownState()
                : hwnd(nullptr)
                , found(false)
            {
                memset(&lastKnownSize, 0, sizeof(lastKnownSize));
            }

            HWND hwnd;
            bool found;
            RECT lastKnownSize;
        };

        AdCloseContext(HWND hwnd)
            : kakaoTalkHwnd(hwnd)
            , loopCount(0)
        {
            memset(&adRect, 0, sizeof(adRect));
        }

        HWND kakaoTalkHwnd;
        uint64_t loopCount;

        WindowShownState ad;
        WindowShownState lockModeView;
        WindowShownState onlineView;

        RECT adRect;

        std::map<HWND, ChildWindowInfo> children;
    };

    ///////////////////////////////////////////////////////////////////////////
    class AdHider {
    public:
        AdHider(int32_t kakaoWaitSeconds)
            : _running(false)
            , _kakaoWaitSeconds(kakaoWaitSeconds)
        {}
        virtual ~AdHider();

    public:
        void startup();
        void cleanup();
        void setKakaoWaitSeconds(int32_t seconds) { _kakaoWaitSeconds = seconds; }

    private:
        void threadProc();
        void checkKakaoTalkAd();
        void closeAndResize();

    private:
        bool _running;
        std::atomic_int32_t _kakaoWaitSeconds;
        std::unique_ptr<std::thread> _hiderThread;
        std::unique_ptr<AdCloseContext> _adCloseCtx;
    };
}