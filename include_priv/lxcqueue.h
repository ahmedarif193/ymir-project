#ifndef LXCQUEUE_H
#define LXCQUEUE_H

#include <pthread.h>

#include "common-task.h"
#include "utils/vector.h"
namespace lxcd {

template<typename T>
class QueueNode {
public:
T value;
QueueNode* next;
QueueNode(T val) : value(val), next(nullptr) {
}
};

template<typename T>
class Queue {
public:
Queue();
~Queue();
void enqueue(T value);
T dequeue();
bool isEmpty() const;

private:
QueueNode<T>* head;
QueueNode<T>* tail;
};

class LxcQueue {
public:
LxcQueue();
~LxcQueue();
void addTask(Task* task);
int start();
int stop();
bool isFinished();

private:
static void* runHelper(void* arg);
void run();

Queue<Task*> _tasks;
pthread_mutex_t _mutex;
pthread_cond_t _cond;
bool stopped;
pthread_t thid;
};

} // namespace lxcd

#endif // LXCQUEUE_H
