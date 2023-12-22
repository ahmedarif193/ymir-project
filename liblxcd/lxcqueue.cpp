#include "lxcqueue.h"

namespace lxcd {

LxcQueue::LxcQueue() : stopped(false) {
    pthread_mutex_init(&_mutex, NULL);
    pthread_cond_init(&_cond, NULL);
}

LxcQueue::~LxcQueue() {
    pthread_mutex_destroy(&_mutex);
    pthread_cond_destroy(&_cond);
}

void LxcQueue::addTask(Task* task) {
    pthread_mutex_lock(&_mutex);
    _tasks.enqueue(task);
    pthread_cond_signal(&_cond);
    pthread_mutex_unlock(&_mutex);
}

int LxcQueue::start() {
    if (pthread_create(&thid, NULL, &LxcQueue::runHelper, this) != 0) {
        return -1;
    }
    if (pthread_detach(thid) != 0) {
        return -2;
    }
    return 0;
}

int LxcQueue::stop() {
    pthread_mutex_lock(&_mutex);
    stopped = true;
    pthread_cond_signal(&_cond);
    pthread_mutex_unlock(&_mutex);
    return 0;
}

bool LxcQueue::isFinished() {
    pthread_mutex_lock(&_mutex);
    bool isEmpty = _tasks.isEmpty();
    pthread_mutex_unlock(&_mutex);
    return isEmpty;
}

void* LxcQueue::runHelper(void* arg) {
    LxcQueue* queue = static_cast<LxcQueue*>(arg);
    queue->run();
    return nullptr;
}

void LxcQueue::run() {
    while(true) {
        pthread_mutex_lock(&_mutex);
        while (_tasks.isEmpty() && !stopped) {
            pthread_cond_wait(&_cond, &_mutex);
        }
        if(stopped && _tasks.isEmpty()) {
            pthread_mutex_unlock(&_mutex);
            break;
        }
        Task* task = _tasks.dequeue();
        pthread_mutex_unlock(&_mutex);

        int retCode = task->run();
        task->invokeCallback(retCode);

        delete task;
    }
}

template<typename T>
Queue<T>::Queue() : head(nullptr), tail(nullptr) {}

template<typename T>
Queue<T>::~Queue() {
    while (!isEmpty()) {
        dequeue();
    }
}

template<typename T>
void Queue<T>::enqueue(T value) {
    QueueNode<T>* newNode = new QueueNode<T>(value);
    if (tail) {
        tail->next = newNode;
    }
    tail = newNode;
    if (!head) {
        head = newNode;
    }
}

template<typename T>
T Queue<T>::dequeue() {
    if (isEmpty()) {
        return T();
    }
    QueueNode<T>* temp = head;
    T value = head->value;
    head = head->next;
    if (head == nullptr) {
        tail = nullptr;
    }
    delete temp;
    return value;
}

template<typename T>
bool Queue<T>::isEmpty() const {
    return head == nullptr;
}

// Explicit template instantiations
template class Queue<Task*>;

} // namespace lxcd
