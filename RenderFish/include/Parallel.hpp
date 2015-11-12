#pragma once
#include "RenderFish.hpp"
#include <windows.h>

#ifndef DWORD_MAX
#define DWORD_MAX       0xffffffffUL
#endif

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
public:
	static Mutex* create();
	static void destroy(Mutex *m);

private:
	Mutex();
	~Mutex();
	friend struct MutexLock;
	Mutex(Mutex &m);					// disabled
	Mutex & operator=(const Mutex &);	// disabled

	CRITICAL_SECTION m_critical_section;
};

struct MutexLock {
public:
	MutexLock(Mutex &m);
	~MutexLock();

private:
	Mutex &m_mutex;
	MutexLock(const MutexLock &);				// disabled
	MutexLock & operator=(const MutexLock &);	// disabled

};



class ReadWriteMutex {
public:
	static ReadWriteMutex *create();
	static void destroy(ReadWriteMutex *m);

private:
	ReadWriteMutex();
	~ReadWriteMutex();
	friend struct ReadWriteMutexLock;
	ReadWriteMutexLock & operator=(const ReadWriteMutex &);	// disabled

	void acquire_read();

	void release_read();
	void acquire_write();
	void release_write();

	LONG m_num_writers_waiting = 0;
	LONG m_num_readers_waiting = 0;
	DWORD m_active_writer_readers = 0;

	HANDLE m_h_ready_to_read;
	HANDLE m_h_ready_to_write;
	CRITICAL_SECTION m_critical_section;
};


enum RWMutexLockType { eREAD, eWRITE };

struct ReadWriteMutexLock {
public:
	ReadWriteMutexLock(ReadWriteMutex &m, RWMutexLockType t);
	~ReadWriteMutexLock();
	void upgrade_to_write();
	void downgrade_to_read();

private:
	RWMutexLockType m_type;
	ReadWriteMutex &m_mutex;
	ReadWriteMutexLock(const ReadWriteMutexLock &);				// disabled
	ReadWriteMutexLock &operator=(const ReadWriteMutexLock &);	// disabled

};

class Semaphore {
public:
	Semaphore();
	~Semaphore();
	void post(size_t count = 1);
	void wait();
	bool try_wait();

private:
	HANDLE m_handle;
};

class ConditionVariable {
public:
	ConditionVariable();
	~ConditionVariable();
	void lock();
	void unlock();
	void wait();
	void signal();

private:
	uint32_t m_waiters_count = 0;
	CRITICAL_SECTION m_waiters_count_mutex;
	CRITICAL_SECTION m_condition_mutex;
	enum { eSIGNAL = 0, eBROADCAST = 1, eNUM_EVENTS = 2 };
	// a event is used to send a signal to a thread indicating 
	// that a particular event has occurred.
	HANDLE m_events[eNUM_EVENTS];
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