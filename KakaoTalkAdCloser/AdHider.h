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

        bool kakaoTalkFound() const { return _kakaoTalkFound; }

    private:
        AdHideStatus();

    private:
        bool _kakaoTalkFound;
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