//
// Created by anarion on 2020/2/21.
//

#ifndef MYCPPBASE_OUTCHANNEL_H
#define MYCPPBASE_OUTCHANNEL_H

#include "Channel.h"

namespace anarion {
    class OutChannel : virtual public Channel {
    protected:
        bool o_valid;
    public:

        explicit OutChannel(bool is_valid) : Channel(), o_valid(is_valid) {}
        OutChannel(OutChannel &&rhs) noexcept : Channel(forward<OutChannel>(rhs)) { rhs.o_valid = false; }

        virtual size_type out(char *p, size_type nbytes) = 0;
        virtual Buffer out(size_type nbytes) = 0;
        virtual Buffer out() = 0;

        virtual void closeo() = 0;

        bool outOn() const { return o_valid; }
    };
}

#endif //MYCPPBASE_OUTCHANNEL_H
