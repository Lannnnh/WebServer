#ifndef _BASE_CURRENTTHREAD_H
#define _BASE_CURRENTTHREAD_H

#include <sys/types.h>
#include <string>

extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char* t_threadName;

void cacheTid();

inline int tid()
{
    return t_cachedTid;
}

inline const char* tidString()
{
    return t_tidString;
}

inline int tidStringLength()
{
    return t_tidStringLength;
}

inline const char* name()
{
    return t_threadName;
}

bool isMainThread();

void sleepUsec(int64_t usec);

std::string stackUsec(bool demangle);

#endif // end _BASE_CURRENTTHREAD_H