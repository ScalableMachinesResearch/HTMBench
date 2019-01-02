/*************************************************************************************************
 * Threading devices
 *                                                               Copyright (C) 2009-2012 FAL Labs
 * This file is part of Kyoto Cabinet.
 * This program is free software: you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation, either version
 * 3 of the License, or any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************************************/


#include "kcthread.h"
#include "myconf.h"

namespace kyotocabinet {                 // common namespace


/**
 * Constants for implementation.
 */
namespace {
const uint32_t LOCKBUSYLOOP = 8192;      ///< threshold of busy loop and sleep for locking
const size_t LOCKSEMNUM = 256;           ///< number of semaphores for locking
}


/**
 * Thread internal.
 */
struct ThreadCore {
#if defined(_SYS_MSVC_) || defined(_SYS_MINGW_)
  ::HANDLE th;                           ///< handle
#else
  ::pthread_t th;                        ///< identifier
  bool alive;                            ///< alive flag
#endif
};



/**
 * Call the running thread.
 * @param arg the thread.
 * @return always NULL.
 */
#if defined(_SYS_MSVC_) || defined(_SYS_MINGW_)
static ::DWORD threadrun(::LPVOID arg);
#else
static void* threadrun(void* arg);
#endif


/**
 * Default constructor.
 */
Thread::Thread() : opq_(NULL) {
#if defined(_SYS_MSVC_) || defined(_SYS_MINGW_)
  _assert_(true);
  ThreadCore* core = new ThreadCore;
  core->th = NULL;
  opq_ = (void*)core;
#else
  _assert_(true);
  ThreadCore* core = new ThreadCore;
  core->alive = false;
  opq_ = (void*)core;
#endif
}


/**
 * Destructor.
 */
Thread::~Thread() {
#if defined(_SYS_MSVC_) || defined(_SYS_MINGW_)
  _assert_(true);
  ThreadCore* core = (ThreadCore*)opq_;
  if (core->th) join();
  delete core;
#else
  _assert_(true);
  ThreadCore* core = (ThreadCore*)opq_;
  if (core->alive) join();
  delete core;
#endif
}


/**
 * Start the thread.
 */
void Thread::start() {
#if defined(_SYS_MSVC_) || defined(_SYS_MINGW_)
  _assert_(true);
  ThreadCore* core = (ThreadCore*)opq_;
  if (core->th) throw std::invalid_argument("already started");
  ::DWORD id;
  core->th = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadrun, this, 0, &id);
  if (!core->th) throw std::runtime_error("CreateThread");
#else
  _assert_(true);
  ThreadCore* core = (ThreadCore*)opq_;
  if (core->alive) throw std::invalid_argument("already started");
  if (::pthread_create(&core->th, NULL, threadrun, this) != 0)
    throw std::runtime_error("pthread_create");
  core->alive = true;
#endif
}


/**
 * Wait for the thread to finish.
 */
void Thread::join() {
#if defined(_SYS_MSVC_) || defined(_SYS_MINGW_)
  _assert_(true);
  ThreadCore* core = (ThreadCore*)opq_;
  if (!core->th) throw std::invalid_argument("not alive");
  if (::WaitForSingleObject(core->th, INFINITE) == WAIT_FAILED)
    throw std::runtime_error("WaitForSingleObject");
  ::CloseHandle(core->th);
  core->th = NULL;
#else
  _assert_(true);
  ThreadCore* core = (ThreadCore*)opq_;
  if (!core->alive) throw std::invalid_argument("not alive");
  core->alive = false;
  if (::pthread_join(core->th, NULL) != 0) throw std::runtime_error("pthread_join");
#endif
}


/**
 * Put the thread in the detached state.
 */
void Thread::detach() {
#if defined(_SYS_MSVC_) || defined(_SYS_MINGW_)
  _assert_(true);
#else
  _assert_(true);
  ThreadCore* core = (ThreadCore*)opq_;
  if (!core->alive) throw std::invalid_argument("not alive");
  core->alive = false;
  if (::pthread_detach(core->th) != 0) throw std::runtime_error("pthread_detach");
#endif
}


/**
 * Terminate the running thread.
 */
void Thread::exit() {
#if defined(_SYS_MSVC_) || defined(_SYS_MINGW_)
  _assert_(true);
  ::ExitThread(0);
#else
  _assert_(true);
  ::pthread_exit(NULL);
#endif
}


/**
 * Yield the processor from the current thread.
 */
void Thread::yield() {
#if defined(_SYS_MSVC_) || defined(_SYS_MINGW_)
  _assert_(true);
  ::Sleep(0);
#else
  _assert_(true);
  ::sched_yield();
#endif
}


/**
 * Chill the processor by suspending execution for a quick moment.
 */
void Thread::chill() {
#if defined(_SYS_MSVC_) || defined(_SYS_MINGW_)
  _assert_(true);
  ::Sleep(21);
#else
  _assert_(true);
  struct ::timespec req;
  req.tv_sec = 0;
  req.tv_nsec = 21 * 1000 * 1000;
  ::nanosleep(&req, NULL);
#endif
}



/**
 * Suspend execution of the current thread.
 */
bool Thread::sleep(double sec) {
#if defined(_SYS_MSVC_) || defined(_SYS_MINGW_)
  _assert_(sec >= 0.0);
  if (sec <= 0.0) {
    yield();
    return true;
  }
  if (sec > INT32MAX) sec = INT32MAX;
  ::Sleep(sec * 1000);
  return true;
#else
  _assert_(sec >= 0.0);
  if (sec <= 0.0) {
    yield();
    return true;
  }
  if (sec > INT32MAX) sec = INT32MAX;
  double integ, fract;
  fract = std::modf(sec, &integ);
  struct ::timespec req, rem;
  req.tv_sec = (time_t)integ;
  req.tv_nsec = (long)(fract * 999999000);
  while (::nanosleep(&req, &rem) != 0) {
    if (errno != EINTR) return false;
    req = rem;
  }
  return true;
#endif
}


/**
 * Get the hash value of the current thread.
 */
int64_t Thread::hash() {
#if defined(_SYS_MSVC_) || defined(_SYS_MINGW_)
  _assert_(true);
  return ::GetCurrentThreadId();
#else
  _assert_(true);
  pthread_t tid = pthread_self();
  int64_t num;
  if (sizeof(tid) == sizeof(num)) {
    std::memcpy(&num, &tid, sizeof(num));
  } else if (sizeof(tid) == sizeof(int32_t)) {
    uint32_t inum;
    std::memcpy(&inum, &tid, sizeof(inum));
    num = inum;
  } else {
    num = hashmurmur(&tid, sizeof(tid));
  }
  return num & INT64MAX;
#endif
}


/**
 * Call the running thread.
 */
#if defined(_SYS_MSVC_) || defined(_SYS_MINGW_)
static ::DWORD threadrun(::LPVOID arg) {
  _assert_(true);
  Thread* thread = (Thread*)arg;
  thread->run();
  return NULL;
}
#else
static void* threadrun(void* arg) {
  _assert_(true);
  Thread* thread = (Thread*)arg;
  thread->run();
  return NULL;
}
#endif


/**
 * Default constructor.
 */
Mutex::Mutex() {}


/**
 * Constructor with the specifications.
 */
Mutex::Mutex(Type type) {
}


/**
 * Destructor.
 */
Mutex::~Mutex() {}


/**
 * Get the lock.
 */
void Mutex::lock() {
  TM_BEGIN();
}


/**
 * Try to get the lock.
 */
bool Mutex::lock_try() {
  TM_BEGIN();
  return true;
}


/**
 * Try to get the lock.
 */
bool Mutex::lock_try(double sec){
  TM_BEGIN();
  return true; 
}

/**
 * Release the lock.
 */
void Mutex::unlock() {
  TM_END();
}

/**
 * Constructor.
 */
SlottedMutex::SlottedMutex(size_t slotnum) {}


/**
 * Destructor.
 */
SlottedMutex::~SlottedMutex() {}


/**
 * Get the lock of a slot.
 */
void SlottedMutex::lock(size_t idx) {
  TM_BEGIN();
}


/**
 * Release the lock of a slot.
 */
void SlottedMutex::unlock(size_t idx) {
  TM_END();
}


/**
 * Get the locks of all slots.
 */
void SlottedMutex::lock_all() {
  TM_BEGIN();  
}


/**
 * Release the locks of all slots.
 */
void SlottedMutex::unlock_all() {
  TM_END();
}


/**
 * Default constructor.
 */
SpinLock::SpinLock() : opq_(NULL) {
#if defined(_SYS_MSVC_) || defined(_SYS_MINGW_)
  _assert_(true);
#elif _KC_GCCATOMIC
  _assert_(true);
#else
  _assert_(true);
  ::pthread_spinlock_t* spin = new ::pthread_spinlock_t;
  if (::pthread_spin_init(spin, PTHREAD_PROCESS_PRIVATE) != 0)
    throw std::runtime_error("pthread_spin_init");
  opq_ = (void*)spin;
#endif
}


/**
 * Destructor.
 */
SpinLock::~SpinLock() {
#if defined(_SYS_MSVC_) || defined(_SYS_MINGW_)
  _assert_(true);
#elif _KC_GCCATOMIC
  _assert_(true);
#else
  _assert_(true);
  ::pthread_spinlock_t* spin = (::pthread_spinlock_t*)opq_;
  ::pthread_spin_destroy(spin);
  delete spin;
#endif
}


/**
 * Get the lock.
 */
void SpinLock::lock() {
  TM_BEGIN(); 
}


/**
 * Try to get the lock.
 */
bool SpinLock::lock_try() {
  TM_BEGIN();   
  return true;
}


/**
 * Release the lock.
 */
void SpinLock::unlock() {
  TM_END();
}

/**
 * Constructor.
 */
SlottedSpinLock::SlottedSpinLock(size_t slotnum) {}


/**
 * Destructor.
 */
SlottedSpinLock::~SlottedSpinLock() {}


/**
 * Get the lock of a slot.
 */
void SlottedSpinLock::lock(size_t idx) {
  TM_BEGIN();
}


/**
 * Release the lock of a slot.
 */
void SlottedSpinLock::unlock(size_t idx) {
  TM_END(); 
}


/**
 * Get the locks of all slots.
 */
void SlottedSpinLock::lock_all() {
  TM_BEGIN();
}


/**
 * Release the locks of all slots.
 */
void SlottedSpinLock::unlock_all() {
  TM_END();
}


/**
 * Default constructor.
 */
RWLock::RWLock() {
}

/*
 * Destructor.
 */
RWLock::~RWLock() {}


/**
 * Get the writer lock.
 */
void RWLock::lock_writer() {
  TM_BEGIN();
}


/**
 * Try to get the writer lock.
 */
bool RWLock::lock_writer_try() {
  TM_BEGIN();
  return true; 
}


/**
 * Get a reader lock.
 */
void RWLock::lock_reader() {
  TM_BEGIN();
}


/**
 * Try to get a reader lock.
 */
bool RWLock::lock_reader_try() {
  TM_BEGIN();
  return true;
}


/**
 * Release the lock.
 */
void RWLock::unlock() {
  TM_END();
}


/**
 * Constructor.
 */
SlottedRWLock::SlottedRWLock(size_t slotnum) {}


/**
 * Destructor.
 */
SlottedRWLock::~SlottedRWLock() {
}


/**
 * Get the writer lock of a slot.
 */
void SlottedRWLock::lock_writer(size_t idx) {
  TM_BEGIN();
}


/**
 * Get the reader lock of a slot.
 */
void SlottedRWLock::lock_reader(size_t idx) {
  TM_BEGIN();
}


/**
 * Release the lock of a slot.
 */
void SlottedRWLock::unlock(size_t idx) {
  TM_END();
}


/**
 * Get the writer locks of all slots.
 */
void SlottedRWLock::lock_writer_all() {
  TM_BEGIN();
}


/**
 * Get the reader locks of all slots.
 */
void SlottedRWLock::lock_reader_all() {
  TM_BEGIN();
}


/**
 * Release the locks of all slots.
 */
void SlottedRWLock::unlock_all() {
  TM_END();
}




/**
 * Default constructor.
 */
SpinRWLock::SpinRWLock() {
}


/**
 * Destructor.
 */
SpinRWLock::~SpinRWLock() {
}


/**
 * Get the writer lock.
 */
void SpinRWLock::lock_writer() {
  TM_BEGIN();
}


/**
 * Try to get the writer lock.
 */
bool SpinRWLock::lock_writer_try() {
  TM_BEGIN();
  return true;
}

/**
 * Get a reader lock.
 */
void SpinRWLock::lock_reader() {
  TM_BEGIN();
}


/**
 * Try to get a reader lock.
 */
bool SpinRWLock::lock_reader_try() {
  TM_BEGIN();
  return true;
}


/**
 * Release the lock.
 */
void SpinRWLock::unlock() {
  TM_END();
}


/**
 * Promote a reader lock to the writer lock.
 */
bool SpinRWLock::promote() {
  //nothing to do
  return true;
}


/**
 * Demote the writer lock to a reader lock.
 */
void SpinRWLock::demote() {
  //nothing to do
}


/**
 * Constructor.
 */
SlottedSpinRWLock::SlottedSpinRWLock(size_t slotnum) {
}


/**
 * Destructor.
 */
SlottedSpinRWLock::~SlottedSpinRWLock() {
}


/**
 * Get the writer lock of a slot.
 */
void SlottedSpinRWLock::lock_writer(size_t idx) {
  TM_BEGIN();
}


/**
 * Get the reader lock of a slot.
 */
void SlottedSpinRWLock::lock_reader(size_t idx) {
  TM_BEGIN();
}


/**
 * Release the lock of a slot.
 */
void SlottedSpinRWLock::unlock(size_t idx) {
  TM_END();
}


/**
 * Get the writer locks of all slots.
 */
void SlottedSpinRWLock::lock_writer_all() {
  TM_BEGIN();
}


/**
 * Get the reader locks of all slots.
 */
void SlottedSpinRWLock::lock_reader_all() {
  TM_BEGIN();
}


/**
 * Release the locks of all slots.
 */
void SlottedSpinRWLock::unlock_all() {
  TM_END();
}



/**
 * Default constructor.
 */
CondVar::CondVar() {
}


/**
 * Destructor.
 */
CondVar::~CondVar() {
}


/**
 * Wait for the signal.
 */
void CondVar::wait(Mutex* mutex) {
    TM_COND_WAIT_SPIN(cond);  
}


/**
 * Wait for the signal.
 */
bool CondVar::wait(Mutex* mutex, double sec) {
  TM_COND_WAIT_SPIN(cond);
  return true; 
}


/**
 * Send the wake-up signal to another waiting thread.
 */
void CondVar::signal() {
}


/**
 * Send the wake-up signals to all waiting threads.
 */
void CondVar::broadcast() {
}


/**
 * Default constructor.
 */
TSDKey::TSDKey() : opq_(NULL) {
#if defined(_SYS_MSVC_) || defined(_SYS_MINGW_)
  _assert_(true);
  ::DWORD key = ::TlsAlloc();
  if (key == 0xFFFFFFFF) throw std::runtime_error("TlsAlloc");
  opq_ = (void*)key;
#else
  _assert_(true);
  ::pthread_key_t* key = new ::pthread_key_t;
  if (::pthread_key_create(key, NULL) != 0)
    throw std::runtime_error("pthread_key_create");
  opq_ = (void*)key;
#endif
}


/**
 * Constructor with the specifications.
 */
TSDKey::TSDKey(void (*dstr)(void*)) : opq_(NULL) {
#if defined(_SYS_MSVC_) || defined(_SYS_MINGW_)
  _assert_(true);
  ::DWORD key = ::TlsAlloc();
  if (key == 0xFFFFFFFF) throw std::runtime_error("TlsAlloc");
  opq_ = (void*)key;
#else
  _assert_(true);
  ::pthread_key_t* key = new ::pthread_key_t;
  if (::pthread_key_create(key, dstr) != 0)
    throw std::runtime_error("pthread_key_create");
  opq_ = (void*)key;
#endif
}


/**
 * Destructor.
 */
TSDKey::~TSDKey() {
#if defined(_SYS_MSVC_) || defined(_SYS_MINGW_)
  _assert_(true);
  ::DWORD key = (::DWORD)opq_;
  ::TlsFree(key);
#else
  _assert_(true);
  ::pthread_key_t* key = (::pthread_key_t*)opq_;
  ::pthread_key_delete(*key);
  delete key;
#endif
}


/**
 * Set the value.
 */
void TSDKey::set(void* ptr) {
#if defined(_SYS_MSVC_) || defined(_SYS_MINGW_)
  _assert_(true);
  ::DWORD key = (::DWORD)opq_;
  if (!::TlsSetValue(key, ptr)) std::runtime_error("TlsSetValue");
#else
  _assert_(true);
  ::pthread_key_t* key = (::pthread_key_t*)opq_;
  if (::pthread_setspecific(*key, ptr) != 0)
    throw std::runtime_error("pthread_setspecific");
#endif
}


/**
 * Get the value.
 */
void* TSDKey::get() const {
#if defined(_SYS_MSVC_) || defined(_SYS_MINGW_)
  _assert_(true);
  ::DWORD key = (::DWORD)opq_;
  return ::TlsGetValue(key);
#else
  _assert_(true);
  ::pthread_key_t* key = (::pthread_key_t*)opq_;
  return ::pthread_getspecific(*key);
#endif
}


/**
 * Set the new value.
 */
int64_t AtomicInt64::set(int64_t val) {
#if (defined(_SYS_MSVC_) || defined(_SYS_MINGW_)) && defined(_SYS_WIN64_)
  _assert_(true);
  return ::InterlockedExchange((uint64_t*)&value_, val);
#elif _KC_GCCATOMIC
  _assert_(true);
  int64_t oval = __sync_lock_test_and_set(&value_, val);
  __sync_synchronize();
  return oval;
#else
  _assert_(true);
  lock_.lock();
  int64_t oval = value_;
  value_ = val;
  lock_.unlock();
  return oval;
#endif
}


/**
 * Add a value.
 */
int64_t AtomicInt64::add(int64_t val) {
#if (defined(_SYS_MSVC_) || defined(_SYS_MINGW_)) && defined(_SYS_WIN64_)
  _assert_(true);
  return ::InterlockedExchangeAdd((uint64_t*)&value_, val);
#elif _KC_GCCATOMIC
  _assert_(true);
  int64_t oval = __sync_fetch_and_add(&value_, val);
  __sync_synchronize();
  return oval;
#else
  _assert_(true);
  lock_.lock();
  int64_t oval = value_;
  value_ += val;
  lock_.unlock();
  return oval;
#endif
}


/**
 * Perform compare-and-swap.
 */
bool AtomicInt64::cas(int64_t oval, int64_t nval) {
#if (defined(_SYS_MSVC_) || defined(_SYS_MINGW_)) && defined(_SYS_WIN64_)
  _assert_(true);
  return ::InterlockedCompareExchange((uint64_t*)&value_, nval, oval) == oval;
#elif _KC_GCCATOMIC
  _assert_(true);
  bool rv = __sync_bool_compare_and_swap(&value_, oval, nval);
  __sync_synchronize();
  return rv;
#else
  _assert_(true);
  bool rv = false;
  lock_.lock();
  if (value_ == oval) {
    value_ = nval;
    rv = true;
  }
  lock_.unlock();
  return rv;
#endif
}


/**
 * Get the current value.
 */
int64_t AtomicInt64::get() const {
#if (defined(_SYS_MSVC_) || defined(_SYS_MINGW_)) && defined(_SYS_WIN64_)
  _assert_(true);
  return ::InterlockedExchangeAdd((uint64_t*)&value_, 0);
#elif _KC_GCCATOMIC
  _assert_(true);
  return __sync_fetch_and_add((int64_t*)&value_, 0);
#else
  _assert_(true);
  lock_.lock();
  int64_t oval = value_;
  lock_.unlock();
  return oval;
#endif
}


}                                        // common namespace

// END OF FILE
