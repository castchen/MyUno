#ifndef __LOG_H__
#define __LOG_H__

#include <fstream>
#include <string>

class Log {
public:
    enum LogLevel {
        LOG_MIN = 0,

        LOG_DEBUG,
        LOG_INFO,
        LOG_ERROR,

        LOG_MAX
    };

    int log_level() const { return log_level_; }

    void set_log_level(int val) { log_level_ = val; }

    std::string file_name() const { return file_name_; }

    void set_file_name(std::string val) { file_name_ = val; }
    
public:

    Log() = default;
       
    void Debug(const char* fmt, ...);

    void Info(const char* fmt, ...);

    void Error(const char* fmt, ...);

private:

    bool BeforeCheck(LogLevel log_level);

    void ChangeFile();

    void PrintTime();

private:
    static const int    ONE_LINE_LEN;
    int                 log_level_ = LOG_DEBUG;
    std::ofstream       os_;
    std::string         file_name_ = "default_log.log";
    int                 year_ = 0;
    int                 month_ = 0;
    int                 day_ = 0;
};

#endif