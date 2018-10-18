#pragma once

#include <thread>
#include <memory>

namespace kudd {
    class AdHideStatus {
    public:
        ~AdHideStatus() {}
        static AdHideStatus& get();

    public:
        void reset();
        void setKakaoTalkFound() { _kakaoTalkFound = true; }
        void setAdHidden()       { _adHidden = true;       }

        bool kakaoTalkFound() const { return _kakaoTalkFound; }
        bool adHidden()       const { return _adHidden;       }

    private:
        AdHideStatus();

    private:
        bool _kakaoTalkFound;
        bool _adHidden;
    };

    class AdHider {
    public:
        AdHider()
            : _running(false)
        {}
        virtual ~AdHider();

    public:
        void startup();
        void cleanup();

    private:
        void threadProc();
        void checkKakaoTalkAd();

    private:
        bool _running;
        std::unique_ptr<std::thread> _hiderThread;
    };
}