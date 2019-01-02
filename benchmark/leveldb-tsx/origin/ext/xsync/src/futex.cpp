#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include "../include/futex.hpp"

namespace xsync {

namespace futex {

int wait(int *addr, int val) {
    return syscall(SYS_futex, addr, FUTEX_WAIT_PRIVATE, val, nullptr, nullptr, nullptr);
}

int wake(int *addr, int thread_count) {
    return syscall(SYS_futex, addr, FUTEX_WAKE_PRIVATE, thread_count, nullptr, nullptr);
}

} // namespace futex

} // namespace xsync {
