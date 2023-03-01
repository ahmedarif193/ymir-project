#include <iostream>
#include <string>
#include <memory>
#include <chrono>
#include <thread>
#include "lxcqueue.h"
#include "lxc-container.h"

int main()
{
    LxcQueue queue;

    auto cb = [](void* data) {
        std::cout << "Callback called with data: " << (const char*)data << std::endl;
    };

    // Create and add tasks to the queue
    std::shared_ptr<LxcContainer> task1 = std::make_shared<LxcContainer>("mycontainer1",Method::ENABLE);
    task1->setCallback(cb);

    std::shared_ptr<LxcContainer> task2 = std::make_shared<LxcContainer>("mycontainer2",Method::ENABLE);
    task2->setCallback(cb);

    std::shared_ptr<LxcContainer> task3 = std::make_shared<LxcContainer>("mycontainer3",Method::ENABLE);
    task3->setCallback(cb);

    queue.addTask(task1.get());
    queue.addTask(task2.get());
    queue.addTask(task3.get());

    // Start the queue to execute tasks
    queue.start();

    // Wait for the tasks to complete
    while (!queue.isFinished()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
