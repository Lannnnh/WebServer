#ifndef _BASE_NOCOPYABLE_
#define _BASE_NOCOPYABLE_

class nocopyable 
{
    public:
        nocopyable(const nocopyable&) = delete;
        void operator=(const nocopyable&) = delete;
    protected:
        nocopyable() = default;
        ~nocopyable() = default;
};

#endif