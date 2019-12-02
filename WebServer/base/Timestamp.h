#ifndef _BASE_TIMESTAMP_H
#define _BASE_TIMESTAMP_H

#include "copyable.h"
#include <sys/types.h>
#include <boost/operators.hpp>
#include <string>


// boost operator库允许用户在类里定义少量的操作符（如<），就可以方便地自动生成其他操作符重载，而且保证正确的语义实现。
class Timestamp : public copyable,
                  public boost::equality_comparable<Timestamp>,
                  public boost::less_than_comparable<Timestamp>
{
    public:
        Timestamp() : microSecondsSinceEpoch_(0)
        {   }
        
        explicit Timestamp(int64_t Arg) : microSecondsSinceEpoch_(Arg)
        {   }

        // default copy/assignment/dtor are okay
        // Timestamp(const Timestamp &b)
        // {
        //     this->microSecondsSinceEpoch_ = b.microSecondsSinceEpoch_;
        // }

        void swap(Timestamp &that)
        {
            std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
        }

        std::string toString() const;
        std::string toFormattedString(bool showMicroseconds = true) const;

        bool valid() const { return microSecondsSinceEpoch_ > 0; }

        int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
        time_t secondsSinceEpoch() const 
        {
             return static_cast<time_t> (microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
        }

        static Timestamp now();
        static Timestamp invalid()
        {
            return Timestamp();
        }
        
        static const int kMicroSecondsPerSecond = 1000 * 1000;

    private:
        int64_t microSecondsSinceEpoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

inline double timeDifference(Timestamp high, Timestamp low)
{
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double> (diff) / Timestamp::kMicroSecondsPerSecond;
}

inline Timestamp addTime(Timestamp timestamp, double seconds)
{
    int64_t dalta = static_cast<int64_t> (seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + dalta);
}

#endif // end _BASE_TIMESTAMP_H
