
// logger.cpp
#include "logger.h"
#include <windows.h>
#include <ctime>
#include <iostream>
#include <mutex>

namespace Logger 
{

namespace 
{
    inline std::string filename = "debug.log";
    inline std::ofstream log_file;
    inline LogType current_type = LogType::None;
    inline std::mutex log_mutex;
    inline bool initialized = false;

    std::string get_timestamp() 
    {
        std::time_t now = std::time(nullptr);
        char buf[20];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        return buf;
    }

    void write_line(const std::string& line) 
    {
        std::lock_guard<std::mutex> lock(log_mutex);

        if (current_type == LogType::None || !initialized) 
        {
            return;
        }

        //std::string prefixed = "[" + get_timestamp() + "] " + line + "\n";
        std::string prefixed = line + "\n";

        if (current_type == LogType::Console || current_type == LogType::Both) 
        {
            std::cout << prefixed;
            std::cout.flush();
        }

        if ((current_type == LogType::File || current_type == LogType::Both) && log_file.is_open()) 
        {
            log_file << prefixed;
            log_file.flush();
        }
    }
} // anonymous namespace


bool Init(LogType type) 
{
    std::lock_guard<std::mutex> lock(log_mutex);

    if (initialized) 
    {
        Close();
    }

    current_type = type;

    if (type == LogType::None) 
    {
        initialized = true;
        return true;
    }

    bool ok = true;

    if (type == LogType::Console || type == LogType::Both) 
    {
        if (!AttachConsole(ATTACH_PARENT_PROCESS)) 
        {
            AllocConsole();
        }
        
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        
        std::cout << "[Logger] Console initialized \n";
    }

    if (type == LogType::File || type == LogType::Both) 
    {
        log_file.open(filename, std::ios::out | std::ios::trunc);
    
        if (!log_file.is_open()) 
        {
            std::cerr << "Cant open log file: " << filename << "\n";
            ok = false;
        } 
        else 
        {
            log_file << "[Logger] Init on " << get_timestamp() << "\n";
        }
    }

    initialized = true;

    return ok;
}

void Msg(const char* msg) 
{
    if (!msg) return;
    write_line(msg);
}

void Msg(const std::string& msg) 
{
    write_line(msg);
}

void Format(const char* fmt, ...) 
{
    if (!fmt) return;

    char buffer[4096];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    write_line(buffer);
}

void Close() 
{
    std::lock_guard<std::mutex> lock(log_mutex);

    if (log_file.is_open()) 
    {
        log_file << "[Logger] Closed on " << get_timestamp() << "\n";
        log_file.close();
    }

    initialized = false;
    current_type = LogType::None;
}

} // namespace Logger