#pragma once

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