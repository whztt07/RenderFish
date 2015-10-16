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