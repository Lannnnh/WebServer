#ifndef _BASE_FILEUTIL_H
#define _BASE_FILEUTIL_H

#include "nocopyable.h"

#include <string>
#include <sys/types.h>

class AppendFile : nocopyable
{
    public:
        explicit AppendFile(std::string filename);
        ~AppendFile();
        // append 向文件写
        void append(const char* logline, const size_t len);
        void flush();

    private:
        size_t write(const char* logline, size_t len);

        FILE* fp_;
        char buffer_[64*1024];
};

#endif // end _BASE_FILEUTIL_H