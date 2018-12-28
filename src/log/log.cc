#include <sstream>
#include <fstream>
#include <string>
#include <iomanip>

#include <ctime>
#include <cstdarg>

#include "log.h"

#define COMMON_LOG                                      \
    char line[ONE_LINE_LEN];                            \
    va_list ap;                                         \
    va_start(ap, fmt);                                  \
    int ret = vsnprintf(line, ONE_LINE_LEN, fmt, ap);   \
    va_end(ap);                                         \
    if (ret < 0) {                                      \
        os_ << std::endl;                               \
    } else {                                            \
        os_ << line;                                    \
    }                                                   \
    os_ << std::flush;

const int Log::ONE_LINE_LEN = 1024;

void Log::Debug(const char* fmt, ...) {
    if (!BeforeCheck(LOG_DEBUG)) {
        return;
    }

    os_ << "[debug] ";

    PrintTime();

    COMMON_LOG
}

void Log::Info(const char* fmt, ...) {
    if (!BeforeCheck(LOG_INFO)) {
        return;
    }

    os_ << "[info]  ";

    PrintTime();

    COMMON_LOG
}

void Log::Error(const char* fmt, ...) {
    if (!BeforeCheck(LOG_ERROR)) {
        return;
    }

    os_ << "[error] ";

    PrintTime();

    COMMON_LOG
}

bool Log::BeforeCheck(LogLevel log_level) {
    if (log_level < log_level_) {
        return false;
    }

    time_t now = time(0);
    tm* local_time = localtime(&now);

    if (local_time->tm_year != year_ || local_time->tm_mon != month_ || local_time->tm_mday != day_) {
        ChangeFile();
    }

    if (!os_.is_open()) {
        return false;
    }

    return true;
}

void Log::ChangeFile() {
    if (os_.is_open()) {
        os_.close();

        std::ostringstream new_name;
        new_name << file_name_ << "." << (year_ + 1900) << "." << (month_ + 1) << "." << day_;
        rename(file_name_.c_str(), new_name.str().c_str());
    }

    time_t now = time(0);
    tm* local_time = localtime(&now);

    year_ = local_time->tm_year; 
    month_ = local_time->tm_mon; 
    day_ = local_time->tm_mday;
    os_.open(file_name_, std::ofstream::app);
}

void Log::PrintTime() {
    time_t now = time(0);
    tm* local_time = localtime(&now);

    os_ << std::setfill('0');

    os_ << "[" << local_time->tm_year + 1900 << "-";
    os_ << std::setw(2) << local_time->tm_mon + 1 << "-";
    os_ << std::setw(2) << local_time->tm_mday << " ";
    os_ << std::setw(2) << local_time->tm_hour << ":";
    os_ << std::setw(2) << local_time->tm_min << ":";
    os_ << std::setw(2) << local_time->tm_sec << "] ";

    os_ << std::setfill(' ');
}
