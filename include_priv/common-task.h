#ifndef COMMONTASK_H
#define COMMONTASK_H

typedef void (*callback_t)(void*);
enum class Method { ENABLE, DISABLE, RECONFIGURE, RESET, DESTROY };

class Task {
public:
    typedef void (*callback_t)(void*);

    Task(callback_t cb = nullptr, void* data = nullptr);

    virtual ~Task();

    virtual void run() = 0;

    void invokeCallback();
    void setCallback(callback_t cb,void* data = nullptr);

    void setData(void* data);
protected:
    callback_t m_callback;
    void* m_data;
};

#endif // COMMON-TASK_H
