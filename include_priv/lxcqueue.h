#ifndef LXCQUEUE_H
#define LXCQUEUE_H



#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "common-task.h"

class LxcQueue {
public:
    LxcQueue();
    ~LxcQueue();

    void addTask(Task* task);

    void start();

    void stop();

    bool isFinished();

private:
    void run();

private:
    std::queue<Task*> _tasks;
    std::mutex _mutex;
    std::condition_variable _cond;
    bool stopped;
};

#endif // LXCQUEUE_H
