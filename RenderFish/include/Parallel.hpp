#pragma once
#include "RenderFish.hpp"

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