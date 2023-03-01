#include "common-task.h"

Task::Task(callback_t cb, void *data) : m_callback(cb), m_data(data) {}

Task::~Task() {}

void Task::invokeCallback() {
    if (m_callback) {
        m_callback(m_data);
    }
}

void Task::setCallback(callback_t cb, void *data) {
    this->m_data = data;
    m_callback = cb;
}

void Task::setData(void *data) {
    this->m_data = data;
}
