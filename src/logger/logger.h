// logger.h
#pragma once

#include <string>
#include <fstream>
#include <cstdio>
#include <cstdarg>

enum class LogType {
    None    = 0,
    Console = 1,
    File    = 2,
    Both    = 3,     // console + ficheiro
};

namespace Logger {

    bool Init(LogType type = LogType::Console);
    void Msg(const char* msg);
    void Msg(const std::string& msg);
    void Format(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
    void Close();

    // Conveniência
    inline void Info(const char* fmt, ...)   __attribute__((format(printf, 1, 2)));
    inline void Warning(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
    inline void Error(const char* fmt, ...)  __attribute__((format(printf, 1, 2)));

} // namespace Logger