#ifndef TASKQUEUE_H_
#define TASKQUEUE_H_

#include <functional>
#include "utime.h"


/**
 * Provide task execution handling the async aspect
 * of the tasks. On PICO, the task are ran on the other
 * core to be allowed to mask interrupts, bitbang, ...
 *
 * The typical use it to have a TaskExecutor member in
 * the scheduled task.
 *
 * From the tick method, the executor must be given priority
 * for execution if it is active:
 *
 * if (executor.handleTick()) {
 *     return
 * }
 *
 */

enum TaskExecutorState {
    notProgrammed = 0,
    asyncPending = 1,
    asyncStarted = 2,
};

class TaskExecutorBase {
    friend class TaskExecutorScheduler;
private:
    TaskExecutorBase * next;
    TaskExecutorBase * prev;
    
    volatile TaskExecutorState state;

    static TaskExecutorBase * first;
    static TaskExecutorBase * last;

    void setState(TaskExecutorState state);
protected:
    TaskExecutorBase();
    virtual ~TaskExecutorBase();

    virtual void runAsync() = 0;
    virtual void runSync() = 0;

    void schedule();
public:
    bool isPending() const;

    // Will the schedule fire immediatly ?
    bool isSync() const;

};

template<class Output>
class TaskExecutor : public TaskExecutorBase{
    // The lambda to call when pending
    std::function<Output(void)> task;
    std::function<void(Output)> done;
    Output result;

    virtual void runAsync() {
        result = task();
    }

    virtual void runSync() {
        done(result);
    }

public:
    TaskExecutor(): TaskExecutorBase() {
        task = nullptr;
        done = nullptr;
    }

    virtual ~TaskExecutor() {
    }

    // Schedule the given task.
    // Receives two lambda functions, one to execute the task and one to be called when the task is done.
    // This function must be called from main loop only and cannot be called when pending() is true
    void schedule(std::function<Output(void)> task, std::function<void(Output)> done, const LongDuration & expectedDuration) {
        if (isPending()) {
            return;
        }
        this->task = task;
        this->done = done;
        TaskExecutorBase::schedule();
    }
};



#endif /* TASKQUEUE_H_ */
