#include "lxcqueue.h"

LxcQueue::LxcQueue() : stopped(false) {}

LxcQueue::~LxcQueue() {}

void LxcQueue::addTask(Task *task) {
    std::unique_lock<std::mutex> lock(_mutex);
    _tasks.push(task);
    _cond.notify_one();
}

void LxcQueue::start() {
    std::thread t(&LxcQueue::run, this);
    t.detach();
}

void LxcQueue::stop() {
    std::unique_lock<std::mutex> lock(_mutex);
    stopped = true;
    _cond.notify_one();
}

bool LxcQueue::isFinished() {
    std::unique_lock<std::mutex> lock(_mutex);
    return _tasks.empty();
}

void LxcQueue::run() {
    while (true) {
        std::unique_lock<std::mutex> lock(_mutex);
        _cond.wait(lock, [this](){ return !_tasks.empty() || stopped; });
        if (stopped && _tasks.empty()) {
            return;
        }
        Task* task = _tasks.front();
        _tasks.pop();
        lock.unlock();

        task->run();
        task->invokeCallback();

        delete task;
    }
}
