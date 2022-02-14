#pragma once
#include "pch.h"

/* Note: This class is only thread safe when
removing items from the queue and checking its 
size, since it will be preloaded with items by one thread and then 
they will be removed by multiple threads that will end on empty*/
class threadSafeQueue
{
public:

	threadSafeQueue();
	~threadSafeQueue();
	
	void push(std::string);
	std::string pop();
	int size();
	bool empty();

private:
	
	std::vector<std::string> q;
	std::mutex m;

};

//default constructor and deconstructor
threadSafeQueue::threadSafeQueue(){}

threadSafeQueue::~threadSafeQueue(){}

//throw items in queue haphazardly
void threadSafeQueue::push(std::string _s) {
	q.push_back(_s);
}

std::string threadSafeQueue::pop() {
	std::string ret = "";
	std::unique_lock<std::mutex> lock(m);
	if(q.size() > 0){
		ret = q.back();
		q.pop_back();
		return ret;
	}
	return "";
}

int threadSafeQueue::size() {
	std::unique_lock<std::mutex> lock(m);
	int size = q.size();
	lock.unlock();
	return size;
}

bool threadSafeQueue::empty() {
	std::unique_lock<std::mutex> lock(m);
	bool f = false;
	if (q.size() == 0) {
		f = true;
	}
	lock.unlock();
	return f;
}
