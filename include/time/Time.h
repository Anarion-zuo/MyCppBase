//
// Created by anarion on 5/1/20.
//

#ifndef ANBASE_TIME_H
#define ANBASE_TIME_H

#include <cstring>
#include <ctime>
#include <container/SString.h>

namespace anarion {
    class Time {
        using sec_type = unsigned long;
    protected:

        /*
         * support Linux high precision time mechanism
         * contains accurate measurement on nano-sec
         */
        timespec kernelTime;
        clockid_t clockid = CLOCK_REALTIME;
        /*
         * Select a timer from system definition
         *  CLOCK_REALTIME
         *  CLOCK_BOOTTIME
         *  CLOCK_PROCESS_CPUTIME_ID
         *  CLOCK_THREAD_CPUTIME_ID
         */

        void setNull() { memset(&kernelTime, 0, sizeof(timespec)); }

    public:

        constexpr const timespec &getSpecHandle() const { return kernelTime; }

        constexpr bool isNull() const { return kernelTime.tv_nsec == 0 && kernelTime.tv_sec == 0; }

        // constructors
        Time() { setNull(); }  // initialize a null object
        Time(size_type sec, size_type nsec);
        explicit Time(timespec timespec) : kernelTime(timespec) {}
        explicit Time(time_t timet) : kernelTime({timet, 0}) {}
        Time(const Time &) = default;
        Time(Time &&) noexcept = default;

        static Time now();

        // setters
        void setCurrent();
        constexpr void setClockType(clockid_t id) { clockid = id; }

        // time field
        constexpr sec_type getSecField() const { return kernelTime.tv_sec; }
        constexpr sec_type getNsecField() const { return kernelTime.tv_nsec; }
        constexpr double getDouble() const { return getSecField() + getNsecField() / 1e9; }

        /*
         * Operators
         * compare to check who is earlier
         * compute difference
         */
        bool operator<(const Time &rhs) const;
        bool operator>(const Time &rhs) const;
        bool operator<=(const Time &rhs) const;
        bool operator>=(const Time &rhs) const;

        static timespec difference(const Time &left, const Time &right);
    };

struct NanoSecondRangeError : public virtual std::exception {
    const char *what() const noexcept override {
        return "Overflowing nano-second field when setting time. Nano second should be set lower than 999999999.";
    }
};
}

#endif //ANBASE_TIME_H