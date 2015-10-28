#include "Parallel.hpp"

void tasks_init()
{
	// ncores == 1;
	return;
}

void tasks_cleanup()
{
	return;
}

void enqueue_tasks(const vector<Task *>& tasks)
{
	for (auto task : tasks)
		task->run();
	return;
}

void wait_for_all_tasks()
{
	return;
}

int num_system_cores()
{
	return 1;
}

