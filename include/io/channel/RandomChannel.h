#ifndef RANDOMCHANNEL_H
#define RANDOMCHANNEL_H

#include "OutChannel.h"
#include "InChannel.h"

/**
 * @Name: RandomChannel
 * @Parent: Channel
 * @Utility: encapsulated file descriptor for system io with random access interface
 *          a) file
 *          b) mapped memory
 * @Interface:
 *      1) set current operating position
 *          a) reset to head
 *          b) reset to end
 *          c) move back and forth
 *          d) total block size
 *      3) 
 * 
 */

namespace anarion {
class RandomChannel : virtual public InChannel, virtual public OutChannel {
public:
    // position
    virtual void rewind() = 0;
    virtual void set_append() = 0;
    virtual void move_forth(size_type nbytes) = 0;
    virtual void move_back(size_type nbytes) = 0;
    virtual size_type size() const = 0;

};
} // namespace anarion


#endif  // RANDOMCHANNEL_H