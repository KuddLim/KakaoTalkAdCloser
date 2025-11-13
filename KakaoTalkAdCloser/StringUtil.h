#pragma once

#include <string>

namespace kudd {
    class StringUtil {
    public:
        static std::wstring utf8ToWstr(const std::string& utf);
        static std::string wstrToUtf8(const std::wstring& wstr);
    };
}
