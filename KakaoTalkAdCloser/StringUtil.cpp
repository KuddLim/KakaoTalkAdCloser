#include "stdafx.h"
#include "StringUtil.h"

#include <boost/locale/encoding_utf.hpp>
#include <boost/locale.hpp>

namespace kudd {
    // wstring_convert 가 C++17 이후 deprecate 되었고, 아직(2025) 까지 표준적인 대안이 없다.
    std::wstring StringUtil::utf8ToWstr(const std::string& utf)
    {
        //std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        //return converter.from_bytes(utf);
        return boost::locale::conv::to_utf<wchar_t>(utf, "UTF-8");
    }

    std::string StringUtil::wstrToUtf8(const std::wstring& wstr)
    {
        return boost::locale::conv::from_utf<wchar_t>(wstr, "UTF-8");
    }
}

