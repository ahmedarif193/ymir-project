#ifndef COMMONTASK_H
#define COMMONTASK_H

typedef void (*callback_t)(void*);
enum class Method { CREATE, ENABLE = 1, DISABLE = 0, RECONFIGURE, RESET, DESTROY, IDLE };

class Task {
public:
    typedef void (*callback_t)(void*);

    Task(callback_t cb = nullptr, void* data = nullptr);

    virtual ~Task();

    virtual int run() = 0;

    void invokeCallback();
    void setCallback(callback_t cb,void* data = nullptr);

    void setData(void* data);
protected:
    callback_t m_callback;
    void* m_data;
};

#endif // COMMON-TASK_H
