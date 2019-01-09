#pragma once

#include <cstdint>
#include <string>

namespace kudd {
    const int32_t DEFAULT_KAKAO_WAIT_SECONDS = 30;
    const int32_t MAX_KAKAO_WAIT_SECONDS = 60;

    class Settings {
    public:
        static Settings& get();
        virtual ~Settings() {}

    public:
        bool runWindowsStartup() const { return _runWindowsStartup; }
        bool runMinimized()      const { return _runMinimized;      }
        bool checkUpdate()       const { return _checkUpdate;       }
        int32_t kakaoWaitTime()  const { return _kakaoWaitTime;     }

        void setRunWindowsStartup(bool b);
        void setRunMinimized(bool b);
        void setCheckUpdate(bool b);
        void setKakaoWaitTime(int32_t i);

    private:
        Settings();
        void loadAll();

    private:
        bool _runWindowsStartup;
        bool _runMinimized;
        bool _checkUpdate;
        int32_t _kakaoWaitTime;
    };
}
