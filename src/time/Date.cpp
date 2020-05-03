//
// Created by anarion on 5/3/20.
//

#include "time/Date.h"


void anarion::Date::setCalender(const timespec &kernelTime) {
    if (isNull()) { return; }
    sec_type s = kernelTime.tv_sec;
    sec_type ns = kernelTime.tv_nsec;
    if (ns > 1e9 - 1e6) {
        ++s;
    }
    time_t timet = s;
    tm *tp = nullptr;
    if (timezone == GMT) {
        tp = localtime(&timet);
    } else if (timeZone == LocalDependent) {
        tp = gmtime(&timet);
    } else {

    }
    memcpy(&calenderTime, tp, sizeof(tm));
}

anarion::SString anarion::Date::print(const char *format) {
    char *ret = static_cast<char *>(operator new(256));
    size_type len = strftime(ret, 256, format, &calenderTime);
    return SString::move(ret, len);
}

anarion::SString anarion::Date::print() {
    // Fir, 01 May 2020 08:55:11 GMT
    return print("%a, %d %b %Y %T %Z");
}

bool anarion::Date::isNull() {
    size_type len = sizeof(tm);
    char *p = reinterpret_cast<char *>(&calenderTime);
    for (size_type i = 0; i < len; ++i) {
        if (*p != 0) {
            return false;
        }
    }
    return true;
}

anarion::Date::Date(const anarion::Time &timeObj) {
    setCalender(timeObj.getSpecHandle());
}
