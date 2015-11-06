#pragma once
#include "RenderFish.hpp"
#include <windows.h>

typedef volatile LONG AtomicInt32;
typedef volatile LONGLONG AtomicInt64;
//#ifdef HAS_64_BIT_ATOMICS
////typedef volatile int64_t AtomicInt64;
//#endif

inline int32_t atomic_add(AtomicInt32 *v, int32_t delta) {
	return InterlockedAdd(v, delta);
}

inline int64_t atomic_add(AtomicInt64 *v, int64_t delta) {
	return InterlockedAdd64(v, delta);
}

inline int32_t atomic_compare_and_swap(AtomicInt32 *v, int32_t new_value, int32_t old_value) {
	return InterlockedCompareExchange(v, new_value, old_value);
}

inline int64_t atomic_compare_and_swap(AtomicInt64 *v, int64_t new_value, int64_t old_value) {
	return InterlockedCompareExchange64(v, new_value, old_value);
}


template<typename T>
inline T* atomic_compare_and_swap_pointer(T *v, T new_value, T old_value) {
	return InterlockedCompareExchange(v, new_value, old_value);
}

inline float atomic_add(volatile float *val, float delta) {
	union bits { float f; int32_t i; };
	bits old_val, new_val;
	do {
		// On IA32/x64, adding a PAUSE instruction in compare/exchange loops
		// is recommended to improve performance.  (And it does!)
#if (defined(__i386__) || defined(__amd64__))
		__asm__ __volatile__("pause\n");
#endif
		old_val.f = *val;
		new_val.f = old_val.f + delta;
	} while (atomic_compare_and_swap((AtomicInt32 *)val, new_val.i, old_val.i) != old_val.i);
	return new_val.f;
}

class Mutex {
private:

public:
	static Mutex* create();
	static void destroy(Mutex *m);

private:
};

struct MutexLock {
private:
	Mutex &_mutex;
public:
	MutexLock(Mutex &m);
	~MutexLock();
};


//reader-writer
class RWMutex {

public:
	static RWMutex *create();
	static void destroy();

private:

};


enum RWMutexLockType { READ, WRITE };

struct RWMutexLock {
private:
	RWMutexLockType _type;
	RWMutex &_mutex;

public:
	RWMutexLock(RWMutex &m, RWMutexLockType t);
	~RWMutexLock();
	void upgrade_to_write();
	void downgrade_to_read();
};

void tasks_init();
void tasks_cleanup();

class Task
{
public:
	virtual ~Task() {};
	virtual void run() = 0;
};

void enqueue_tasks(const vector<Task *>& tasks);
void wait_for_all_tasks();
int num_system_cores();