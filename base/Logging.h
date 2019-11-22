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

#define LOG Logger(__FILE__, __LINE__).stream()

#endif // end _BASE_LOGGING_H