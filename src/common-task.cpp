#include "common-task.h"

Task::Task(callback_t cb, void *data) : m_callback(cb), m_data(data) {}

Task::~Task() {}

void Task::invokeCallback(int retCode) {
    if (m_callback != nullptr) {
        m_callback(m_data, retCode);
    }
}

void Task::setCallback(callback_t cb, void *data) {
    this->m_data = data;
    m_callback = cb;
}

void Task::setData(void *data) {
    this->m_data = data;
}
