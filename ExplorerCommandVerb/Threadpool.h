#pragma once

#include <functional>
#include <queue>
#include <mutex>
#include <atomic>
#include <fstream>

class Threadpool {
public:
	Threadpool();
	~Threadpool();

private:
	void Run(void);

public:
	std::mutex myMutex;
	std::vector<std::thread> mythreads;
	std::condition_variable condvar,cv;
	std::atomic_bool isToStop;
	std::queue<std::function<void()>> myqueue;
	int taskinprog;
	void AddTask(std::function<void()>);
	void WaitForAll();
};