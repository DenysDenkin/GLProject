#include "Threadpool.h"
#include <thread>
#include <vector>
#include <functional>
#include <fstream>

Threadpool::Threadpool():mythreads(1)
{
	for (int i = 0; i < 1; i++)
		mythreads[i]=std::thread(std::bind(&Threadpool::Run, this));
	isToStop = false;
	taskinprog = 0;
}

Threadpool::~Threadpool()
{
	isToStop = true;
	cv.notify_all();
	taskinprog = 0;
	condvar.notify_all();
	for (int i = 0; i < 1; i++)
		mythreads[i].join();
}

void Threadpool::Run(void)
{
	std::function<void()> curTask;
	while (!isToStop) {
		{			
			std::unique_lock<std::mutex> mylock(myMutex);
			while (myqueue.empty() && !isToStop)
				condvar.wait(mylock);
			if (!isToStop) {
				curTask = myqueue.front();
				myqueue.pop();
			}
			
		}
		if (!isToStop) {
			curTask();
			taskinprog--;
			cv.notify_one();
		}
	}

}

void Threadpool::AddTask(std::function<void()> myfunc)
{
	std::unique_lock<std::mutex> mylock(myMutex);
	taskinprog++;
	myqueue.push(myfunc);
	condvar.notify_one();
}

void Threadpool::WaitForAll()
{
	std::unique_lock<std::mutex> mylock(myMutex);
	while (taskinprog != 0)
		cv.wait(mylock);
}