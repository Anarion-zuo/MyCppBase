//
// Created by anarion on 5/1/20.
//

#include <io/base/sys_utility.h>
#include <cstdlib>
#include "time/Time.h"

void anarion::Time::setCurrent() {
    clock_gettime(clockid, &this->kernelTime);
}

anarion::Time anarion::Time::now() {
    Time time;
    time.setCurrent();
    return time;
}

bool anarion::Time::operator<(const anarion::Time &rhs) const {
    if (kernelTime.tv_sec < rhs.kernelTime.tv_sec) {
        return true;
    }
    if (kernelTime.tv_sec > rhs.kernelTime.tv_sec) {
        return false;
    }
    return kernelTime.tv_nsec < rhs.kernelTime.tv_nsec;
}

bool anarion::Time::operator>(const anarion::Time &rhs) const {
    return rhs < *this;
}

bool anarion::Time::operator<=(const anarion::Time &rhs) const {
    return !(rhs < *this);
}

bool anarion::Time::operator>=(const anarion::Time &rhs) const {
    return !(*this < rhs);
}

timespec anarion::Time::difference(const anarion::Time &left, const anarion::Time &right) {
    timespec ret;
    if (left > right) {
        ret.tv_sec = left.kernelTime.tv_sec - right.kernelTime.tv_sec;
        ret.tv_nsec = left.kernelTime.tv_nsec - right.kernelTime.tv_nsec;
        return ret;
    }
    ret.tv_sec = right.kernelTime.tv_sec - left.kernelTime.tv_sec;
    ret.tv_nsec = right.kernelTime.tv_nsec - left.kernelTime.tv_nsec;
    return ret;
}



anarion::Time::Time(anarion::size_type sec, anarion::size_type nsec)  : kernelTime({static_cast<__time_t>(sec), static_cast<__syscall_slong_t>(nsec)}) {
    if (nsec > 999999999) {
        throw NanoSecondRangeError();
    }
}

