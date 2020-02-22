#ifndef _BASE_LOGFILE_H
#define _BASE_LOGFILE_H

#include "MutexLock.h"
#include "type.h"
#include "nocopyable.h"

#include <memory>

class AppendFile;

class LogFile : nocopyable
{
    public:
        LogFile(const std::string& basename, int flushEveryN = 1024);
        ~LogFile();

        void append(const char* logline, int len);
        void flush();

    private:
        void append_unlocked(const char* logline, int len);

        const std::string basename_;
        const int flushEvertN_;

        int count_;
        std::unique_ptr<MutexLock> mutex_;
        std::unique_ptr<AppendFile> file_;
};



#endif // end _BASE_LOGFILE_H