#ifndef FUTEX_HPP
#define FUTEX_HPP

#include <atomic>
/*
 * Convenience functions around the futex API inspired by Ulrich Drepper's "Futexes Are Tricky"
 */
namespace xsync {

namespace futex {

/*
 * If the value of the futex is still val, wait until woken by a signal or a call to futexWake.
 */
int wait(int *addr, int val);

/*
 * Wake up at most thread_count threads from the wait queue of the given futex.
 */
int wake(int *addr, int thread_count);

} // namespace futex

} // namespace xsync

#endif
