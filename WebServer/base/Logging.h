#ifndef _BASE_LOGGING_H
#define _BASE_LOGGING_H

#include "LogStream.h"

#include <string>

class AsyncLogging;

class Logger
{
    public: 
        Logger(const char* fileName, int line);
        ~Logger();
        LogStream& stream() { return impl_.stream_; }

        static void setLogFileName(std::string fileName)
        {
            logFileName_ = fileName;
        }

        static std::string getLogFileName()
        {
            return logFileName_;
        }

    private:
        class Impl
        {
            public:
                Impl(const char* fileNane, int line);
                void formatTime();

                LogStream stream_;
                int line_;
                std::string basename_;
        };
        Impl impl_;
        static std::string logFileName_;
};

// 预定宏：__FILE__ 当前程序源文件名，__LINE__当前程序行的行号
#define LOG Logger(__FILE__, __LINE__).stream()

#endif // end _BASE_LOGGING_H