#include "stdafx.h"
#include "FileLogger.h"

#include <iostream>
#include <iomanip>
#include <ctime>
#include <codecvt>
#include <boost/dll.hpp>
#include <boost/algorithm/string.hpp>

#pragma warning(disable:4996)

using namespace kudd;

FileLogger::FileLogger()
    : _use(true)
{
    _logFile = boost::dll::program_location().string();
    boost::algorithm::replace_last(_logFile, ".exe", ".log.txt");
    _ofs = std::make_unique<std::ofstream>();
}

FileLogger::~FileLogger()
{
}

FileLogger& FileLogger::get()
{
    static FileLogger __singleton__;
    return __singleton__;
}

void FileLogger::writeLog(const std::string& text)
{
    if (_use) {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);

        _ofs->open(_logFile, std::ios::app | std::ios::ate);
        (*_ofs) << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] " << text.c_str() << std::endl;
        _ofs->close();
    }
}

void FileLogger::writeLog(const std::wstring& text)
{
    if (_use) {
        using convert_type = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_type, wchar_t> converter;
        writeLog(converter.to_bytes(text));
    }
}