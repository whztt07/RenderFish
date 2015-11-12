#include "Parallel.hpp"

static HANDLE				*threads;
static Mutex				*task_queue_mutex = Mutex::create();
static vector<Task *>		task_queue;
static Semaphore			*worker_semaphore;
static uint32_t				num_unfinished_tasks;
static ConditionVariable	*tasks_running_condition;

DWORD WINAPI taskEntry(LPVOID arg);

void tasks_init()
{
	static const int n_threads = num_system_cores();
	worker_semaphore = new Semaphore;
	tasks_running_condition = new ConditionVariable;
	
	threads = new HANDLE[n_threads];
	for (int i = 0; i < n_threads; ++i) {
		threads[i] = CreateThread(NULL, 0, taskEntry, reinterpret_cast<void *>(i), 0, NULL);
		if (nullptr == threads[i])
			error("Error from CreateThread\n");
	}
	info("Create %d threads.\n", n_threads);
}

void tasks_cleanup()
{
	if (!task_queue_mutex || !worker_semaphore)
		return;
	{
		MutexLock lock(*task_queue_mutex);
		Assert(task_queue.size() == 0);
	}

	static const size_t n_threads = num_system_cores();
	if (nullptr != worker_semaphore)
		worker_semaphore->post(n_threads);

	if (nullptr != threads) {
		WaitForMultipleObjects(n_threads, threads, TRUE, DWORD_MAX);
		for (int i = 0; i < n_threads; ++i) {
			CloseHandle(threads[i]);
		}
		delete[] threads;
		threads = nullptr;
	}
}

void enqueue_tasks(const vector<Task *>& tasks)
{
	if (nullptr == threads)
		tasks_init();
	{
		MutexLock lock(*task_queue_mutex);
		for (auto& task : tasks)
			task_queue.push_back(task);
	}
	tasks_running_condition->lock();
	num_unfinished_tasks += (uint32_t)tasks.size();
	tasks_running_condition->unlock();

	worker_semaphore->post(tasks.size());
}

static DWORD WINAPI taskEntry(LPVOID arg) {
	while (true) {
		worker_semaphore->wait();
		// Try to get task from task queue
		Task *myTask = nullptr;
		{
			MutexLock lock(*task_queue_mutex);
			if (task_queue.size() == 0)
				break;
			myTask = task_queue.back();
			task_queue.pop_back();
		}

		// Do work for _myTask_
		myTask->run();
		tasks_running_condition->lock();
		if (--num_unfinished_tasks == 0)
			tasks_running_condition->signal();
		tasks_running_condition->unlock();
	}
	return 0;
}


void wait_for_all_tasks()
{
	if (!tasks_running_condition)
		return;
	tasks_running_condition->lock();
	while (num_unfinished_tasks > 0)
		tasks_running_condition->wait();
	tasks_running_condition->unlock();
}

int num_system_cores()
{
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);
	int cores = sys_info.dwNumberOfProcessors;
	//info("Number of system cores: %d\n", cores);
	return cores;
}

Mutex* Mutex::create()
{
	return new Mutex;
}

void Mutex::destroy(Mutex *m)
{
	delete m;
}

Mutex::Mutex()
{
	InitializeCriticalSection(&m_critical_section);
}

Mutex::~Mutex()
{
	DeleteCriticalSection(&m_critical_section);
}

MutexLock::MutexLock(Mutex &m) : m_mutex(m)
{
	EnterCriticalSection(&m_mutex.m_critical_section);
}

MutexLock::~MutexLock()
{
	LeaveCriticalSection(&m_mutex.m_critical_section);
}

ReadWriteMutex * ReadWriteMutex::create()
{
	return new ReadWriteMutex;
}

void ReadWriteMutex::destroy(ReadWriteMutex *m)
{
	delete m;
}

ReadWriteMutex::ReadWriteMutex()
{
	// TODO: use SRW instead
	InitializeCriticalSection(&m_critical_section);

	m_h_ready_to_read = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (nullptr == m_h_ready_to_read) {
		error("Error creating event for RMMutex: %d\n", GetLastError());
	}

	m_h_ready_to_write = CreateSemaphore(NULL, 0, 1, NULL);
	if (nullptr == m_h_ready_to_write) {
		auto lase_error = GetLastError();
		CloseHandle(m_h_ready_to_read);
		error("Error creating semaphore for RWMutex: %d\n", lase_error);
	}
}

ReadWriteMutex::~ReadWriteMutex()
{
	if (nullptr != m_h_ready_to_read)
		CloseHandle(m_h_ready_to_read);
	if (nullptr != m_h_ready_to_write)
		CloseHandle(m_h_ready_to_write);
	DeleteCriticalSection(&m_critical_section);
}

void ReadWriteMutex::acquire_read()
{
	bool notifity_readers = false;

	EnterCriticalSection(&m_critical_section);

	// The reader is only allowed to read if there aren't
	// any writers waiting and if a writer doesn't own the lock.
	if ((m_num_writers_waiting > 0) || (HIWORD(m_active_writer_readers) > 0)) {
		++m_num_readers_waiting;

		while (true) {
			ResetEvent(m_h_ready_to_read);
			LeaveCriticalSection(&m_critical_section);
			WaitForSingleObject(m_h_ready_to_read, DWORD_MAX);
			EnterCriticalSection(&m_critical_section);

			if ((m_num_writers_waiting = 0) && (HIWORD(m_active_writer_readers) == 0))
				break;
		}
		// reader is done waiting
		--m_num_readers_waiting;
		// reader can read
		++m_active_writer_readers;
	}
	else {
		// Reader can read.
		if ((++m_active_writer_readers == 1) && (m_num_readers_waiting != 0)) {
			// Set flag to notify other waiting readers
			// outside of the critical section
			// so that they don't when the threads
			// are dispatched by the scheduler they
			// don't immediately block on the critical
			// section that this thread is holding.
			notifity_readers = true;
		}
	}

	Assert(HIWORD(m_active_writer_readers) == 0);
	LeaveCriticalSection(&m_critical_section);

	if (notifity_readers)
		SetEvent(m_h_ready_to_read);
}

void ReadWriteMutex::release_read()
{
	EnterCriticalSection(&m_critical_section);

	// Are there active readers?
	if (m_active_writer_readers != 0) {
		++m_num_writers_waiting;

		LeaveCriticalSection(&m_critical_section);
		WaitForSingleObject(m_h_ready_to_write, DWORD_MAX);

		// Upon wakeup theirs no need for the writer
		// to acquire the critical section.  It
		// already has been transfered ownership of the
		// lock by the signaler.
	}
	else {
		Assert(m_active_writer_readers == 0);

		// Set that the writer owns the lock;
		m_active_writer_readers = MAKELONG(0, 1);

		LeaveCriticalSection(&m_critical_section);
	}
}

void ReadWriteMutex::acquire_write()
{
	EnterCriticalSection(&m_critical_section);

	// Assert that the lock isn't held by a writer.
	Assert(HIWORD(m_active_writer_readers) == 0);

	// Assert that the lock is held by readers.
	Assert(LOWORD(m_active_writer_readers) > 0);

	// Decrement the number of active readers.
	if (--m_active_writer_readers == 0)
		ResetEvent(m_h_ready_to_read);

	// if writers are waiting and this is the last reader
	// hand ownership over to a writer.
	if ((m_num_writers_waiting != 0) && (m_active_writer_readers == 0)) {
		// // Decrement the number of waiting writers
		--m_num_writers_waiting;

		// Pass ownership to a writer thread.
		m_active_writer_readers = MAKELONG(0, 1);
		ReleaseSemaphore(m_h_ready_to_write, 1, NULL);
	}
	LeaveCriticalSection(&m_critical_section);
}

void ReadWriteMutex::release_write()
{
	bool notify_writer = false;
	bool notify_readers = false;

	EnterCriticalSection(&m_critical_section);

	// Assert that the lock is owned by a writer.
	Assert(HIWORD(m_active_writer_readers) == 1);
	// Assert that the lock isn't owned by one or more readers
	Assert(LOWORD(m_active_writer_readers) == 0);

	if (m_num_writers_waiting != 0) {
		// Writers waiting, decrement the number of
		// waiting writers and release the semaphore
		// which means ownership is passed to the thread
		// that has been released.
		--m_num_writers_waiting;
		notify_writer = true;
	}
	else {
		// There aren't any writers waiting
		// Release the exclusive hold on the lock.
		m_active_writer_readers = 0;

		// if readers are waiting set the flag
		// that will cause the readers to be notified
		// once the critical section is released.  This
		// is done so that an awakened reader won't immediately
		// block on the critical section which is still being
		// held by this thread.
		if (m_num_readers_waiting != 0)
			notify_readers = true;
	}

	LeaveCriticalSection(&m_critical_section);

	if (notify_writer)
		ReleaseSemaphore(m_h_ready_to_write, 1, NULL);
	else if (notify_readers)
		SetEvent(m_h_ready_to_read);
}

Semaphore::Semaphore()
{
	m_handle = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
	if (nullptr == m_handle)
		error("Error from CreateSemaphore: %d\n", GetLastError());
}

Semaphore::~Semaphore()
{
	CloseHandle(m_handle);
}

void Semaphore::post(size_t count /*= 1*/)
{
	if (!ReleaseSemaphore(m_handle, (LONG)count, NULL))
		error("Error from ReleaseSemaphore: %d\n", GetLastError());
}

void Semaphore::wait()
{
	if (WaitForSingleObject(m_handle, DWORD_MAX) == WAIT_FAILED)
		error("Error form WaitForSingleObject: %d", GetLastError());
}

bool Semaphore::try_wait()
{
	return (WaitForSingleObject(m_handle, 0L) == WAIT_OBJECT_0);
}

ReadWriteMutexLock::ReadWriteMutexLock(ReadWriteMutex &m, RWMutexLockType t) : m_type(t), m_mutex(m)
{
	m_type == eREAD ? m_mutex.acquire_read() : m_mutex.acquire_write();
}

ReadWriteMutexLock::~ReadWriteMutexLock()
{
	m_type == eREAD ? m_mutex.release_read() : m_mutex.release_write();
}

void ReadWriteMutexLock::upgrade_to_write()
{
	Assert(m_type == eREAD);
	m_mutex.release_read();
	m_mutex.acquire_write();
	m_type = eWRITE;
}

void ReadWriteMutexLock::downgrade_to_read()
{
	Assert(m_type == eWRITE);
	m_mutex.release_write();
	m_mutex.acquire_read();
	m_type = eREAD;
}

ConditionVariable::ConditionVariable() {
	InitializeCriticalSection(&m_waiters_count_mutex);
	InitializeCriticalSection(&m_condition_mutex);

	m_events[eSIGNAL] = CreateEvent(NULL,  // no security
		FALSE, // auto-reset event
		FALSE, // non-signaled initially
		NULL); // unnamed
	m_events[eBROADCAST] = CreateEvent(NULL,  // no security
		TRUE,  // manual-reset
		FALSE, // non-signaled initially
		NULL); // unnamed
}

ConditionVariable::~ConditionVariable()
{
	CloseHandle(m_events[eSIGNAL]);
	CloseHandle(m_events[eBROADCAST]);
}

void ConditionVariable::lock()
{
	EnterCriticalSection(&m_condition_mutex);
}

void ConditionVariable::unlock()
{
	LeaveCriticalSection(&m_condition_mutex);
}

void ConditionVariable::wait()
{
	// avoid race conditions
	EnterCriticalSection(&m_waiters_count_mutex);
	++m_waiters_count;
	LeaveCriticalSection(&m_waiters_count_mutex);

	// It's ok to release the <external_mutex> here since Win32
	// manual-reset events maintain state when used with
	// <SetEvent>.  This avoids the "lost wakeup" bug...
	LeaveCriticalSection(&m_condition_mutex);

	// Wait for either event to become signaled due to <pthread_cond_signal>
	// being called or <pthread_cond_broadcast> being called.
	int result = WaitForMultipleObjects(2, m_events, FALSE, DWORD_MAX);

	EnterCriticalSection(&m_waiters_count_mutex);
	--m_waiters_count;
	int last_waiter = (result == WAIT_OBJECT_0 + eBROADCAST) &&
		(m_waiters_count == 0);
	LeaveCriticalSection(&m_waiters_count_mutex);

	// Some thread called <pthread_cond_broadcast>.
	if (last_waiter > 0) {
		// We're the last waiter to be notified or to stop waiting, so
		// reset the manual event.
		ResetEvent(m_events[eBROADCAST]);
	}

	EnterCriticalSection(&m_condition_mutex);
}

void ConditionVariable::signal()
{
	EnterCriticalSection(&m_waiters_count_mutex);
	int have_waiters = (m_waiters_count > 0);
	LeaveCriticalSection(&m_waiters_count_mutex);

	if (have_waiters)
		SetEvent(m_events[eSIGNAL]);
}
