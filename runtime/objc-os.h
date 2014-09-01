/*
 * Copyright (c) 2007 Apple Inc.  All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/***********************************************************************
* objc-os.h
* OS portability layer.
**********************************************************************/

#ifndef _OBJC_OS_H
#define _OBJC_OS_H

#include <TargetConditionals.h>

#if TARGET_OS_MAC

#   ifndef __STDC_LIMIT_MACROS
#       define __STDC_LIMIT_MACROS
#   endif

#   include <stdio.h>
#   include <stdlib.h>
#   include <stdint.h>
#   include <stdarg.h>
#   include <string.h>
#   include <ctype.h>
#   include <errno.h>
#   include <dlfcn.h>
#   include <fcntl.h>
#   include <assert.h>
#   include <limits.h>
#   include <syslog.h>
#   include <unistd.h>
#   include <pthread.h>
#   include <crt_externs.h>
#   include <AssertMacros.h>
#   undef check
#   include <AvailabilityMacros.h>
#   include <TargetConditionals.h>
#   include <sys/mman.h>
#   include <sys/time.h>
#   include <sys/stat.h>
#   include <sys/param.h>
#   include <mach/mach.h>
#   include <mach-o/dyld.h>
#   include <mach-o/ldsyms.h>
#   include <mach-o/loader.h>
#   include <mach-o/getsect.h>
#   include <mach-o/dyld_priv.h>
#   include <malloc/malloc.h>
#   include <libkern/OSAtomic.h>
#   include <libkern/OSCacheControl.h>
#   include <System/pthread_machdep.h>
#   include "objc-probes.h"  // generated dtrace probe definitions.


#if defined(__i386__) || defined(__x86_64__)

// Inlined spinlock.
// Not for arm on iOS because it hurts uniprocessor performance.

#define ARR_SPINLOCK_INIT 0
// XXX -- Careful: OSSpinLock isn't volatile, but should be
typedef volatile int ARRSpinLock;
__attribute__((always_inline))
static inline void ARRSpinLockLock(ARRSpinLock *l)
{
    unsigned y;
again:
    if (__builtin_expect(__sync_lock_test_and_set(l, 1), 0) == 0) {
        return;
    }
    for (y = 1000; y; y--) {
#if defined(__i386__) || defined(__x86_64__)
        asm("pause");
#endif
        if (*l == 0) goto again;
    }
    thread_switch(THREAD_NULL, SWITCH_OPTION_DEPRESS, 1);
    goto again;
}
__attribute__((always_inline))
static inline void ARRSpinLockUnlock(ARRSpinLock *l)
{
    __sync_lock_release(l);
}
__attribute__((always_inline))
static inline int ARRSpinLockTry(ARRSpinLock *l)
{
    return __sync_bool_compare_and_swap(l, 0, 1);
}

#define OSSpinLock ARRSpinLock
#define OSSpinLockTry(l) ARRSpinLockTry(l)
#define OSSpinLockLock(l) ARRSpinLockLock(l)
#define OSSpinLockUnlock(l) ARRSpinLockUnlock(l)
#undef OS_SPINLOCK_INIT
#define OS_SPINLOCK_INIT ARR_SPINLOCK_INIT 

#endif


#if !TARGET_OS_IPHONE
#   include <CrashReporterClient.h>
#else
    // CrashReporterClient not yet available on iOS
    __BEGIN_DECLS
    extern const char *CRSetCrashLogMessage(const char *msg);
    extern const char *CRGetCrashLogMessage(void);
    extern const char *CRSetCrashLogMessage2(const char *msg);
    __END_DECLS
#endif

#if TARGET_IPHONE_SIMULATOR
    // getsectiondata() and getsegmentdata() are unavailable
    __BEGIN_DECLS
#   define getsectiondata(m, s, n, c) objc_getsectiondata(m, s, n, c)
#   define getsegmentdata(m, s, c) objc_getsegmentdata(m, s, c)
    extern uint8_t *objc_getsectiondata(const struct mach_header *mh, const char *segname, const char *sectname, unsigned long *outSize);
    extern uint8_t * objc_getsegmentdata(const struct mach_header *mh, const char *segname, unsigned long *outSize);
    __END_DECLS
#endif

#   if __cplusplus
#       include <vector>
#       include <algorithm>
#       include <functional>
        using namespace std;
#   endif

#   define PRIVATE_EXTERN __attribute__((visibility("hidden")))
#   undef __private_extern__
#   define __private_extern__ use_PRIVATE_EXTERN_instead
#   undef private_extern
#   define private_extern use_PRIVATE_EXTERN_instead

/* Use this for functions that are intended to be breakpoint hooks.
   If you do not, the compiler may optimize them away.
   BREAKPOINT_FUNCTION( void stop_on_error(void) ); */
#   define BREAKPOINT_FUNCTION(prototype)                            \
    OBJC_EXTERN __attribute__((noinline, visibility("hidden")))      \
    prototype { asm(""); }

#elif TARGET_OS_WIN32

#   define WINVER 0x0501		// target Windows XP and later
#   define _WIN32_WINNT 0x0501	// target Windows XP and later
#   define WIN32_LEAN_AND_MEAN
    // hack: windef.h typedefs BOOL as int
#   define BOOL WINBOOL
#   include <windows.h>
#   undef BOOL

#   include <stdio.h>
#   include <stdlib.h>
#   include <stdint.h>
#   include <stdarg.h>
#   include <string.h>
#   include <assert.h>
#   include <malloc.h>
#   include <AvailabilityMacros.h>

#   if __cplusplus
#       include <vector>
#       include <algorithm>
#       include <functional>
        using namespace std;
#       define __BEGIN_DECLS extern "C" {
#       define __END_DECLS   }
#   else
#       define __BEGIN_DECLS /*empty*/
#       define __END_DECLS   /*empty*/
#   endif

#   define PRIVATE_EXTERN
#   define __attribute__(x)
#   define inline __inline

/* Use this for functions that are intended to be breakpoint hooks.
   If you do not, the compiler may optimize them away.
   BREAKPOINT_FUNCTION( void MyBreakpointFunction(void) ); */
#   define BREAKPOINT_FUNCTION(prototype) \
    __declspec(noinline) prototype { __asm { } }

/* stub out dtrace probes */
#   define OBJC_RUNTIME_OBJC_EXCEPTION_RETHROW() do {} while(0)  
#   define OBJC_RUNTIME_OBJC_EXCEPTION_THROW(arg0) do {} while(0)

#else
#   error unknown OS
#endif


#include <objc/objc.h>
#include <objc/objc-api.h>

__BEGIN_DECLS

extern void _objc_fatal(const char *fmt, ...) __attribute__((noreturn, format (printf, 1, 2)));

#define INIT_ONCE_PTR(var, create, delete)                              \
    do {                                                                \
        if (var) break;                                                 \
        typeof(var) v = create;                                         \
        while (!var) {                                                  \
            if (OSAtomicCompareAndSwapPtrBarrier(0, (void*)v, (void**)&var)){ \
                goto done;                                              \
            }                                                           \
        }                                                               \
        delete;                                                         \
    done:;                                                              \
    } while (0)

#define INIT_ONCE_32(var, create, delete)                               \
    do {                                                                \
        if (var) break;                                                 \
        typeof(var) v = create;                                         \
        while (!var) {                                                  \
            if (OSAtomicCompareAndSwap32Barrier(0, v, (volatile int32_t *)&var)) { \
                goto done;                                              \
            }                                                           \
        }                                                               \
        delete;                                                         \
    done:;                                                              \
    } while (0)


// Thread keys reserved by libc for our use.
// Keys [0..4] are used by autozone.
#if defined(__PTK_FRAMEWORK_OBJC_KEY5)
#   define SUPPORT_DIRECT_THREAD_KEYS 1
#   define TLS_DIRECT_KEY        ((tls_key_t)__PTK_FRAMEWORK_OBJC_KEY5)
#   define SYNC_DATA_DIRECT_KEY  ((tls_key_t)__PTK_FRAMEWORK_OBJC_KEY6)
#   define SYNC_COUNT_DIRECT_KEY ((tls_key_t)__PTK_FRAMEWORK_OBJC_KEY7)
#   define AUTORELEASE_POOL_KEY  ((tls_key_t)__PTK_FRAMEWORK_OBJC_KEY8)
# if SUPPORT_RETURN_AUTORELEASE
#   define AUTORELEASE_POOL_RECLAIM_KEY ((tls_key_t)__PTK_FRAMEWORK_OBJC_KEY9)
# endif
#else
#   define SUPPORT_DIRECT_THREAD_KEYS 0
#endif


#if TARGET_OS_WIN32

// Compiler compatibility

// OS compatibility

#define strdup _strdup

#define issetugid() 0

#define MIN(x, y) ((x) < (y) ? (x) : (y))

static __inline void bcopy(const void *src, void *dst, size_t size) { memcpy(dst, src, size); }
static __inline void bzero(void *dst, size_t size) { memset(dst, 0, size); }

int asprintf(char **dstp, const char *format, ...);

typedef void * malloc_zone_t;

static __inline malloc_zone_t malloc_default_zone(void) { return (malloc_zone_t)-1; }
static __inline void *malloc_zone_malloc(malloc_zone_t z, size_t size) { return malloc(size); }
static __inline void *malloc_zone_calloc(malloc_zone_t z, size_t size, size_t count) { return calloc(size, count); }
static __inline void *malloc_zone_realloc(malloc_zone_t z, void *p, size_t size) { return realloc(p, size); }
static __inline void malloc_zone_free(malloc_zone_t z, void *p) { free(p); }
static __inline malloc_zone_t malloc_zone_from_ptr(const void *p) { return (malloc_zone_t)-1; }
static __inline size_t malloc_size(const void *p) { return _msize((void*)p); /* fixme invalid pointer check? */ }


// AssertMacros

#define require_action_string(cond, dest, act, msg) do { if (!(cond)) { { act; } goto dest; } } while (0)
#define require_noerr_string(err, dest, msg) do { if (err) goto dest; } while (0)
#define require_string(cond, dest, msg) do { if (!(cond)) goto dest; } while (0)


// OSAtomic

static __inline BOOL OSAtomicCompareAndSwapLong(long oldl, long newl, long volatile *dst) 
{ 
    // fixme barrier is overkill
    long original = InterlockedCompareExchange(dst, newl, oldl);
    return (original == oldl);
}

static __inline BOOL OSAtomicCompareAndSwapPtrBarrier(void *oldp, void *newp, void * volatile *dst) 
{ 
    void *original = InterlockedCompareExchangePointer(dst, newp, oldp);
    return (original == oldp);
}

static __inline BOOL OSAtomicCompareAndSwap32Barrier(int32_t oldl, int32_t newl, int32_t volatile *dst) 
{ 
    long original = InterlockedCompareExchange((volatile long *)dst, newl, oldl);
    return (original == oldl);
}

static __inline int32_t OSAtomicDecrement32Barrier(volatile int32_t *dst)
{
    return InterlockedDecrement((volatile long *)dst);
}

static __inline int32_t OSAtomicIncrement32Barrier(volatile int32_t *dst)
{
    return InterlockedIncrement((volatile long *)dst);
}


// Internal data types

typedef DWORD objc_thread_t;  // thread ID
static __inline int thread_equal(objc_thread_t t1, objc_thread_t t2) { 
    return t1 == t2; 
}
static __inline objc_thread_t thread_self(void) { 
    return GetCurrentThreadId(); 
}

typedef struct {
    DWORD key;
    void (*dtor)(void *);
} tls_key_t;
static __inline tls_key_t tls_create(void (*dtor)(void*)) { 
    // fixme need dtor registry for DllMain to call on thread detach
    tls_key_t k;
    k.key = TlsAlloc();
    k.dtor = dtor;
    return k;
}
static __inline void *tls_get(tls_key_t k) { 
    return TlsGetValue(k.key); 
}
static __inline void tls_set(tls_key_t k, void *value) { 
    TlsSetValue(k.key, value); 
}

typedef struct {
    CRITICAL_SECTION *lock;
} mutex_t;
#define MUTEX_INITIALIZER {0};
extern void mutex_init(mutex_t *m);
static __inline int _mutex_lock_nodebug(mutex_t *m) { 
    // fixme error check
    if (!m->lock) {
        mutex_init(m);
    }
    EnterCriticalSection(m->lock); 
    return 0;
}
static __inline bool _mutex_try_lock_nodebug(mutex_t *m) { 
    // fixme error check
    if (!m->lock) {
        mutex_init(m);
    }
    return TryEnterCriticalSection(m->lock); 
}
static __inline int _mutex_unlock_nodebug(mutex_t *m) { 
    // fixme error check
    LeaveCriticalSection(m->lock); 
    return 0;
}


typedef mutex_t OSSpinLock;
#define OSSpinLockLock(l) mutex_lock(l)
#define OSSpinLockUnlock(l) mutex_unlock(l)
#define OS_SPINLOCK_INIT MUTEX_INITIALIZER


typedef struct {
    HANDLE mutex;
} recursive_mutex_t;
#define RECURSIVE_MUTEX_INITIALIZER {0};
#define RECURSIVE_MUTEX_NOT_LOCKED 1
extern void recursive_mutex_init(recursive_mutex_t *m);
static __inline int _recursive_mutex_lock_nodebug(recursive_mutex_t *m) { 
    assert(m->mutex);
    return WaitForSingleObject(m->mutex, INFINITE);
}
static __inline bool _recursive_mutex_try_lock_nodebug(recursive_mutex_t *m) { 
    assert(m->mutex);
    return (WAIT_OBJECT_0 == WaitForSingleObject(m->mutex, 0));
}
static __inline int _recursive_mutex_unlock_nodebug(recursive_mutex_t *m) { 
    assert(m->mutex);
    return ReleaseMutex(m->mutex) ? 0 : RECURSIVE_MUTEX_NOT_LOCKED;
}


/*
typedef HANDLE mutex_t;
static inline void mutex_init(HANDLE *m) { *m = CreateMutex(NULL, FALSE, NULL); }
static inline void _mutex_lock(mutex_t *m) { WaitForSingleObject(*m, INFINITE); }
static inline bool mutex_try_lock(mutex_t *m) { return WaitForSingleObject(*m, 0) == WAIT_OBJECT_0; }
static inline void _mutex_unlock(mutex_t *m) { ReleaseMutex(*m); }
*/

// based on http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
// Vista-only CONDITION_VARIABLE would be better
typedef struct {
    HANDLE mutex;
    HANDLE waiters;      // semaphore for those in cond_wait()
    HANDLE waitersDone;  // auto-reset event after everyone gets a broadcast
    CRITICAL_SECTION waitCountLock;  // guards waitCount and didBroadcast
    unsigned int waitCount;
    int didBroadcast; 
} monitor_t;
#define MONITOR_INITIALIZER { 0 }
#define MONITOR_NOT_ENTERED 1
extern int monitor_init(monitor_t *c);

static inline int _monitor_enter_nodebug(monitor_t *c) {
    if (!c->mutex) {
        int err = monitor_init(c);
        if (err) return err;
    }
    return WaitForSingleObject(c->mutex, INFINITE);
}
static inline int _monitor_exit_nodebug(monitor_t *c) {
    if (!ReleaseMutex(c->mutex)) return MONITOR_NOT_ENTERED;
    else return 0;
}
static inline int _monitor_wait_nodebug(monitor_t *c) { 
    int last;
    EnterCriticalSection(&c->waitCountLock);
    c->waitCount++;
    LeaveCriticalSection(&c->waitCountLock);

    SignalObjectAndWait(c->mutex, c->waiters, INFINITE, FALSE);

    EnterCriticalSection(&c->waitCountLock);
    c->waitCount--;
    last = c->didBroadcast  &&  c->waitCount == 0;
    LeaveCriticalSection(&c->waitCountLock);

    if (last) {
        // tell broadcaster that all waiters have awoken
        SignalObjectAndWait(c->waitersDone, c->mutex, INFINITE, FALSE);
    } else {
        WaitForSingleObject(c->mutex, INFINITE);
    }

    // fixme error checking
    return 0;
}
static inline int monitor_notify(monitor_t *c) { 
    int haveWaiters;

    EnterCriticalSection(&c->waitCountLock);
    haveWaiters = c->waitCount > 0;
    LeaveCriticalSection(&c->waitCountLock);

    if (haveWaiters) {
        ReleaseSemaphore(c->waiters, 1, 0);
    }

    // fixme error checking
    return 0;
}
static inline int monitor_notifyAll(monitor_t *c) { 
    EnterCriticalSection(&c->waitCountLock);
    if (c->waitCount == 0) {
        LeaveCriticalSection(&c->waitCountLock);
        return 0;
    }
    c->didBroadcast = 1;
    ReleaseSemaphore(c->waiters, c->waitCount, 0);
    LeaveCriticalSection(&c->waitCountLock);

    // fairness: wait for everyone to move from waiters to mutex
    WaitForSingleObject(c->waitersDone, INFINITE);
    // not under waitCountLock, but still under mutex
    c->didBroadcast = 0;

    // fixme error checking
    return 0;
}


// fixme no rwlock yet

#define rwlock_t mutex_t
#define rwlock_init(r) mutex_init(r)
#define _rwlock_read_nodebug(m) _mutex_lock_nodebug(m)
#define _rwlock_write_nodebug(m) _mutex_lock_nodebug(m)
#define _rwlock_try_read_nodebug(m) _mutex_try_lock_nodebug(m)
#define _rwlock_try_write_nodebug(m) _mutex_try_lock_nodebug(m)
#define _rwlock_unlock_read_nodebug(m) _mutex_unlock_nodebug(m)
#define _rwlock_unlock_write_nodebug(m) _mutex_unlock_nodebug(m)


typedef IMAGE_DOS_HEADER headerType;
// fixme YES bundle? NO bundle? sometimes?
#define headerIsBundle(hi) YES
OBJC_EXTERN IMAGE_DOS_HEADER __ImageBase;
#define libobjc_header ((headerType *)&__ImageBase)

// Prototypes


#elif TARGET_OS_MAC


// OS headers
#include <mach-o/loader.h>
#ifndef __LP64__
#   define SEGMENT_CMD LC_SEGMENT
#else
#   define SEGMENT_CMD LC_SEGMENT_64
#endif

#ifndef VM_MEMORY_OBJC_DISPATCHERS
#   define VM_MEMORY_OBJC_DISPATCHERS 0
#endif


// Compiler compatibility

// OS compatibility

// Internal data types

typedef pthread_t objc_thread_t;

static __inline int thread_equal(objc_thread_t t1, objc_thread_t t2) { 
    return pthread_equal(t1, t2); 
}
static __inline objc_thread_t thread_self(void) { 
    return pthread_self(); 
}


typedef pthread_key_t tls_key_t;

static inline tls_key_t tls_create(void (*dtor)(void*)) { 
    tls_key_t k;
    pthread_key_create(&k, dtor); 
    return k;
}
static inline void *tls_get(tls_key_t k) { 
    return pthread_getspecific(k); 
}
static inline void tls_set(tls_key_t k, void *value) { 
    pthread_setspecific(k, value); 
}

#if SUPPORT_DIRECT_THREAD_KEYS

#if !NDEBUG
static bool is_valid_direct_key(tls_key_t k) {
    return (   k == SYNC_DATA_DIRECT_KEY
            || k == SYNC_COUNT_DIRECT_KEY
            || k == AUTORELEASE_POOL_KEY
#   if SUPPORT_RETURN_AUTORELEASE
            || k == AUTORELEASE_POOL_RECLAIM_KEY
#   endif
               );
}
#endif

#if __arm__

// rdar://9162780  _pthread_get/setspecific_direct are inefficient
// copied from libdispatch

__attribute__((always_inline)) __attribute__((const))
static inline void**
tls_base(void)
{
    uintptr_t p;
#if defined(__arm__) && defined(_ARM_ARCH_6)
    __asm__("mrc	p15, 0, %[p], c13, c0, 3" : [p] "=&r" (p));
    return (void**)(p & ~0x3ul);
#else
#error tls_base not implemented
#endif
}

__attribute__((always_inline))
static inline void
tls_set_direct(void **tsdb, tls_key_t k, void *v)
{
    assert(is_valid_direct_key(k));

    tsdb[k] = v;
}
#define tls_set_direct(k, v)                    \
        tls_set_direct(tls_base(), (k), (v))

__attribute__((always_inline))
static inline void *
tls_get_direct(void **tsdb, tls_key_t k)
{
    assert(is_valid_direct_key(k));

    return tsdb[k];
}
#define tls_get_direct(k)                       \
        tls_get_direct(tls_base(), (k))

// arm
#else
// not arm

static inline void *tls_get_direct(tls_key_t k) 
{ 
    assert(is_valid_direct_key(k));

    if (_pthread_has_direct_tsd()) {
        return _pthread_getspecific_direct(k);
    } else {
        return pthread_getspecific(k);
    }
}
static inline void tls_set_direct(tls_key_t k, void *value) 
{ 
    assert(is_valid_direct_key(k));

    if (_pthread_has_direct_tsd()) {
        _pthread_setspecific_direct(k, value);
    } else {
        pthread_setspecific(k, value);
    }
}

// not arm
#endif

// SUPPORT_DIRECT_THREAD_KEYS
#endif


typedef pthread_mutex_t mutex_t;
#define MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER;

extern int DebuggerMode;
extern void gdb_objc_debuggerModeFailure(void);
extern BOOL isManagedDuringDebugger(void *lock);
extern BOOL isLockedDuringDebugger(void *lock);

static inline int _mutex_lock_nodebug(mutex_t *m) { 
    if (DebuggerMode  &&  isManagedDuringDebugger(m)) {
        if (! isLockedDuringDebugger(m)) {
            gdb_objc_debuggerModeFailure();
        }
        return 0;
    }
    return pthread_mutex_lock(m); 
}
static inline bool _mutex_try_lock_nodebug(mutex_t *m) { 
    if (DebuggerMode  &&  isManagedDuringDebugger(m)) {
        if (! isLockedDuringDebugger(m)) {
            gdb_objc_debuggerModeFailure();
        }
        return true;
    }
    return !pthread_mutex_trylock(m); 
}
static inline int _mutex_unlock_nodebug(mutex_t *m) { 
    if (DebuggerMode  &&  isManagedDuringDebugger(m)) {
        return 0;
    }
    return pthread_mutex_unlock(m); 
}


typedef struct { 
    pthread_mutex_t *mutex; 
} recursive_mutex_t;
#define RECURSIVE_MUTEX_INITIALIZER {0};
#define RECURSIVE_MUTEX_NOT_LOCKED EPERM
extern void recursive_mutex_init(recursive_mutex_t *m);

static inline int _recursive_mutex_lock_nodebug(recursive_mutex_t *m) { 
    assert(m->mutex);
    if (DebuggerMode  &&  isManagedDuringDebugger(m)) {
        if (! isLockedDuringDebugger((mutex_t *)m)) {
            gdb_objc_debuggerModeFailure();
        }
        return 0;
    }
    return pthread_mutex_lock(m->mutex); 
}
static inline bool _recursive_mutex_try_lock_nodebug(recursive_mutex_t *m) { 
    assert(m->mutex);
    if (DebuggerMode  &&  isManagedDuringDebugger(m)) {
        if (! isLockedDuringDebugger((mutex_t *)m)) {
            gdb_objc_debuggerModeFailure();
        }
        return true;
    }
    return !pthread_mutex_trylock(m->mutex); 
}
static inline int _recursive_mutex_unlock_nodebug(recursive_mutex_t *m) { 
    assert(m->mutex);
    if (DebuggerMode  &&  isManagedDuringDebugger(m)) {
        return 0;
    }
    return pthread_mutex_unlock(m->mutex); 
}


typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} monitor_t;
#define MONITOR_INITIALIZER { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER }
#define MONITOR_NOT_ENTERED EPERM

static inline int monitor_init(monitor_t *c) {
    int err = pthread_mutex_init(&c->mutex, NULL);
    if (err) return err;
    err = pthread_cond_init(&c->cond, NULL);
    if (err) {
        pthread_mutex_destroy(&c->mutex);
        return err;
    }
    return 0;
}
static inline int _monitor_enter_nodebug(monitor_t *c) {
    assert(!isManagedDuringDebugger(c));
    return pthread_mutex_lock(&c->mutex);
}
static inline int _monitor_exit_nodebug(monitor_t *c) {
    return pthread_mutex_unlock(&c->mutex);
}
static inline int _monitor_wait_nodebug(monitor_t *c) { 
    return pthread_cond_wait(&c->cond, &c->mutex);
}
static inline int monitor_notify(monitor_t *c) { 
    return pthread_cond_signal(&c->cond);
}
static inline int monitor_notifyAll(monitor_t *c) { 
    return pthread_cond_broadcast(&c->cond);
}


// semaphore_create formatted for INIT_ONCE use
static inline semaphore_t create_semaphore(void)
{
    semaphore_t sem;
    kern_return_t k;
    k = semaphore_create(mach_task_self(), &sem, SYNC_POLICY_FIFO, 0);
    if (k) _objc_fatal("semaphore_create failed (0x%x)", k);
    return sem;
}


/* Custom read-write lock
   - reader is atomic add/subtract 
   - writer is pthread mutex plus atomic add/subtract
   - fairness: new readers wait if a writer wants in
   - fairness: when writer completes, readers (probably) precede new writer

   state: xxxxxxxx xxxxxxxx yyyyyyyy yyyyyyyz
       x: blocked reader count
       y: active reader count
       z: readers allowed flag
*/
typedef struct {
    pthread_rwlock_t rwl;
} rwlock_t;

extern BOOL isReadingDuringDebugger(rwlock_t *lock);
extern BOOL isWritingDuringDebugger(rwlock_t *lock);

static inline void rwlock_init(rwlock_t *l)
{
    int err __unused = pthread_rwlock_init(&l->rwl, NULL);
    assert(err == 0);
}

static inline void _rwlock_read_nodebug(rwlock_t *l)
{
    if (DebuggerMode  &&  isManagedDuringDebugger(l)) {
        if (! isReadingDuringDebugger(l)) {
            gdb_objc_debuggerModeFailure();
        }
        return;
    }
    int err __unused = pthread_rwlock_rdlock(&l->rwl);
    assert(err == 0);
}

static inline void _rwlock_unlock_read_nodebug(rwlock_t *l)
{
    if (DebuggerMode  &&  isManagedDuringDebugger(l)) {
        return;
    }
    int err __unused = pthread_rwlock_unlock(&l->rwl);
    assert(err == 0);
}


static inline bool _rwlock_try_read_nodebug(rwlock_t *l)
{
    if (DebuggerMode  &&  isManagedDuringDebugger(l)) {
        if (! isReadingDuringDebugger(l)) {
            gdb_objc_debuggerModeFailure();
        }
        return true;
    }
    int err = pthread_rwlock_tryrdlock(&l->rwl);
    assert(err == 0  ||  err == EBUSY);
    return (err == 0);
}


static inline void _rwlock_write_nodebug(rwlock_t *l)
{
    if (DebuggerMode  &&  isManagedDuringDebugger(l)) {
        if (! isWritingDuringDebugger(l)) {
            gdb_objc_debuggerModeFailure();
        }
        return;
    }
    int err __unused = pthread_rwlock_wrlock(&l->rwl);
    assert(err == 0);
}

static inline void _rwlock_unlock_write_nodebug(rwlock_t *l)
{
    if (DebuggerMode  &&  isManagedDuringDebugger(l)) {
        return;
    }
    int err __unused = pthread_rwlock_unlock(&l->rwl);
    assert(err == 0);
}

static inline bool _rwlock_try_write_nodebug(rwlock_t *l)
{
    if (DebuggerMode  &&  isManagedDuringDebugger(l)) {
        if (! isWritingDuringDebugger(l)) {
            gdb_objc_debuggerModeFailure();
        }
        return true;
    }
    int err = pthread_rwlock_trywrlock(&l->rwl);
    assert(err == 0  ||  err == EBUSY);
    return (err == 0);
}


#ifndef __LP64__
typedef struct mach_header headerType;
typedef struct segment_command segmentType;
typedef struct section sectionType;
#else
typedef struct mach_header_64 headerType;
typedef struct segment_command_64 segmentType;
typedef struct section_64 sectionType;
#endif
#define headerIsBundle(hi) (hi->mhdr->filetype == MH_BUNDLE)
#define libobjc_header ((headerType *)&_mh_dylib_header)

// Prototypes

/* Secure /tmp usage */
extern int secure_open(const char *filename, int flags, uid_t euid);


#else


#error unknown OS


#endif

__END_DECLS

#endif
