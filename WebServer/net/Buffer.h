#ifndef _NET_BUFFER_H
#define _NET_BUFFER_H

#include "WebServer/base/copyable.h"
#include "WebServer/base/type.h"

#include <algorithm>
#include <vector>
#include <assert.h>
#include <string.h>

class Buffer : public copyable
{
    public:
        static const size_t kCheapPrepend = 0;
        static const size_t kInitialSize = 1024;

        explicit Buffer(size_t initialSize = kInitialSize)
            : buffer_(kCheapPrepend + kInitialSize),
              readerIndex_(kCheapPrepend),
              writerIndex_(kCheapPrepend)
        {
            
        }

        void swap(Buffer &rhs)
        {
            buffer_.swap(rhs.buffer_);
            std::swap(readerIndex_, rhs.readerIndex_);
            std::swap(writerIndex_, rhs.writerIndex_);
        }

        size_t readableBytes() const
        {
            return writerIndex_ - readerIndex_;
        }

        size_t writableBytes() const
        {
            return buffer_.size() - writerIndex_;
        }

        size_t prependableBytes() const
        {
            return readerIndex_;
        }

        const char* peek() const
        {
            return begin() + readerIndex_;
        }

        const char* findCRLF() const
        {
            const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
            return crlf == beginWrite() ? NULL : crlf;
        }

        void retrieveAll()
        {
            readerIndex_ = kCheapPrepend;
            writerIndex_ = kCheapPrepend;
        }

        std::string retrieveAllAsString()
        {
            return retrieveAsString(readableBytes());
        }

        std::string retrieveAsString(size_t len)
        {
            assert(len <= readableBytes());
            std::string result(peek(), len);
            retrieve(len);
            return result;
        }

        // 如果正好读完了所有可读的空间，那么readerIndex_和writerIndex_都退回到kCheapPrepend，避免空间浪费
        // 如果读不完，那么readerIndex_读取len个数据即可
        void retrieve(size_t len)
        {
            assert(len <= readableBytes());
            if (len < readableBytes())
            {
                readerIndex_ += len;
            }
            else
            {
                retrieveAll();
            }
            
        }

        void retrieveUntil(const char *end)
        {
            assert(peek() <= end);
            assert(end <= beginWrite());
            retrieve(end - peek());
        }

        void ensureWritableBytes(size_t len)
        {
            if (writableBytes() < len)
            {
                makeSpace(len);
            }
            assert(writableBytes() >= len);
        }

        char* beginWrite()
        { return begin() + writerIndex_; }

        const char* beginWrite() const
        { return begin() + writerIndex_; }

        void hasWritten(size_t len)
        {
            assert(len <= writableBytes());
            writerIndex_ += len;
        }

        void unwrite(size_t len)
        {
            assert(len <= readableBytes());
            writerIndex_ -= len;
        }
    

        void append(const char *data, size_t len)
        {
            ensureWritableBytes(len);
            std::copy(data, data+len, beginWrite());
            hasWritten(len);
        }

        void append(const void *data, size_t len)
        {
            append(static_cast<const char*> (data), len);
        }

        void append(const std::string& str)
        {
            append(str.data(), str.size());
        }

        void prepend(const void *data, size_t len)
        {
            assert(len <= prependableBytes());
            readerIndex_ -= len;
            const char *d = static_cast<const char*> (data);
            std::copy(d, d+len, begin()+readerIndex_);
        }

        void shrink(size_t reserve)
        {
            Buffer other;
            other.ensureWritableBytes(readableBytes()+reserve);
            other.append(peek(), static_cast<int> (readableBytes()));
            swap(other);
        }

        size_t internalCapcity() const
        { return buffer_.capacity(); }

        ssize_t readFd(int fd, int *savedErrno);

    private:
        char* begin()
        { return &*buffer_.begin(); }

        const char* begin() const
        { return &*buffer_.begin(); }

        void makeSpace(size_t len)
        {
            if (writableBytes() + prependableBytes() < len + kCheapPrepend)
            {
                buffer_.resize(writerIndex_+len);
            }
            else
            {
                assert(kCheapPrepend < readerIndex_);
                size_t readable = readableBytes();
                std::copy(begin()+readerIndex_, 
                          begin()+writerIndex_, 
                          begin()+kCheapPrepend);  
                readerIndex_ = kCheapPrepend;
                writerIndex_ = readerIndex_+readable;
                assert(readable == readableBytes());
            }
            
        }

        std::vector<char> buffer_;
        size_t readerIndex_;
        size_t writerIndex_;

        static const char kCRLF[];
};

#endif // end _NET_BUFFER_H