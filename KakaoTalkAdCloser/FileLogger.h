#pragma once

#include <fstream>
#include <memory>
#include <string>

namespace kudd {
    class FileLogger {
    private:
        FileLogger();

    public:
        virtual ~FileLogger();
        static FileLogger& get();

    public:
        void writeLog(const std::string& text);
        void writeLog(const std::wstring& text);
        void setUse(bool use) { _use = use; }

        bool use() const { return _use; }

    private:
        std::unique_ptr<std::ofstream> _ofs;
        std::string _logFile;
        bool _use;
    };
}
