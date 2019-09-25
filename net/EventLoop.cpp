#include "EventLoop.h"
#include <unistd.h>

__thread EventLoop* t_loopInThisThread = 0;

// EventLoop::EventLoop()
//     : looping_(false), 
//       //threadId_(CurrentThread)





