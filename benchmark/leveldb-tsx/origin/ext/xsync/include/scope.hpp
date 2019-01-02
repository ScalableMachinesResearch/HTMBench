#ifndef SCOPE_HPP
#define SCOPE_HPP

// PThreads
#include <pthread.h>

// Intel intrinsics
//#include <immintrin.h>

// STL
#include <functional>
#include <vector>

// xsync
#include "lock.hpp"
#include "tm.h"

namespace xsync {

/*
 * Provides an RAII style wrapper for transactional execution.
 */
template <typename LockType>
class XScope {
public:

    XScope(LockType& fallback){ enter(); }
    ~XScope() { exit(); }

    // No copy
    XScope(const XScope& other) = delete;

    void enter(){
        TM_BEGIN();
    }
    void exit(){
        TM_END();

        // Execute callbacks
        for (std::function<void()> &cb : cbs_) {
            cb();
        }
    }

    // Callback must not throw an exception since it is executed in the destructor
    void registerCommitCallback(const std::function<void()> &cb) {
        cbs_.push_back(cb);
    }


private:
    //friend class XCondVar;
    std::vector<std::function<void()>> cbs_;
    //LockType &fallback_;
};

// These are lock-specific hacks to check the state without modifying memory.
// Usually this state is private (for good reason), but we need to access it

// template<>
// bool XScope<spinlock_t>::isFallbackLocked() {
//     return *(reinterpret_cast<int*>(&fallback_)) == 0;
//}

/*template<>
bool XScope<pthread_mutex_t>::isFallbackLocked() {
    return fallback_.__data.__lock != 0;
}*/


} // namespace xsync

#endif
