#ifndef _NET_HTTP_HTTPREQUEST_H
#define _NET_HTTP_HTTPREQUEST_H

#include "base/copyable.h"
#include "base/Timestamp.h"
#include "base/type.h"

#include <map>
#include <assert.h>
#include <stdio.h>

class HttpRequest : public copyable
{
    public:
        enum Method
        {
            kInvalid, kGet, kPost, kHead, kPut, kDelete
        };

        enum Version
        {
            kUnknown, kHttp10, kHttp11
        };

        HttpRequest()
            : method_(kInvalid),
              version_(kUnknown)
        {   }

        void setVersion(Version v)
        {
            version_ = v;
        }

        Version getVersion() const
        {
            return version_;
        }

        bool setMethod(const char* start, const char* end)
        {
            assert(method_ == kInvalid);
            string m(start, end);
            if (m == "GET")
            {
                method_ = kGet;
            }
            else if (m == "POST")
            {
                method_ = kPost;
            }
            else if (m == "HEAD")
            {
                method_ = kHead;
            }
            else if (m == "PUT")
            {
                method_ = kPut;
            }
            else if (m == "DELETE")
            {
                method_ = kDelete;
            }
            else 
            {
                method_ = kInvalid;
            }
            return method_ != kInvalid;
        }

        Method method() const
        {
            return method_;
        }

        const char* methodString() const
        {
            const char* result = "UNKNOWN";
            switch(method_)
            {
                case kGet:
                    result = "GET";
                    break;
                case kPost:
                    result = "POST";
                    break;
                case kHead:
                    result = "HEAD";
                    break;
                case kPut:
                    result = "PUT";
                    break;
                case kDelete:
                    result = "DELETE";
                    break;
                default:
                    break;
            }
            return result;
        }

        void setPath(const char* start, const char* end)
        {
            path_.assign(start, end);
        }

        const std::string& path() const
        {
            return path_;
        }

        void setQuery(const char* start, const char* end)
        {
            query_.assign(start, end);
        }

        const std::string& query() const
        {
            return query_;
        }

        void setReceiveTime(Timestamp t)
        {
            receiveTime_ = t;
        }

        Timestamp receiveTime() const
        {
            return receiveTime_;
        }

        void addHeader(const char* start, const char* colon, const char* end)
        {
            std::string field(start, colon);
            ++colon;
            while (colon < end && isspace(*colon))
            {
                ++colon;
            }
            std::string value(colon, end);
            while (!value.empty() && isspace(value[value.size()-1]))
            {
                value.resize(value.size()-1);
            }
            headers_[field] = value;
        }

        std::string getHeader(const std::string& field) const
        {
            std::string result;
            auto it = headers_.find(field);
            if (it != headers_.end())
            {
                result = it->second;
            }
            return result;
        }

        const std::map<std::string, std::string>& headers() const
        {
            return headers_;
        }

        void swap(HttpRequest target)
        {
            std::swap(method_, target.method_);
            std::swap(version_, target.version_);
            path_.swap(target.path_);
            query_.swap(target.query_);
            receiveTime_.swap(target.receiveTime_);
            headers_.swap(target.headers_);
        }
    
    private:
        Method method_;
        Version version_;
        std::string path_;
        std::string query_;
        Timestamp receiveTime_;
        std::map<std::string, std::string> headers_;
};

#endif // end _NET_HTTP_HTTPREQUEST_H