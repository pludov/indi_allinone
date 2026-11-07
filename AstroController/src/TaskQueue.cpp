#include "TaskQueue.h"
#include "Scheduled.h"

#include "pico/util/queue.h"


class TaskExecutorScheduler: public Scheduled {
    static TaskExecutorScheduler * instance;
public:
    TaskExecutorScheduler(): Scheduled(Symbol(F("TASK_EXECUTOR_SCHEDULER"))) {
    }

    virtual ~TaskExecutorScheduler() {
    }

    virtual void tick() {
        TaskExecutorBase * te;
        while((te = TaskExecutorBase::first)) {
            switch(te->state) {
                case asyncPending:
                    te->setState(asyncStarted);
                    rp2040.fifo.push(uintptr_t(te));
                case asyncStarted:
                    uint32_t v;
                    if (!rp2040.fifo.pop_nb(&v)) {
                        this->nextTick = UTime::now() + MS(1);
                        this->tickExpectedDuration = US(200);
                        this->priority = 2;
                        // Awaiting result
                        return;
                    }
                    // This removes te from the list
                    te->setState(notProgrammed);
                    te->runSync();
                    break;
            }
        }
        this->nextTick = UTime::never();
    }

    void awake() {
        this->nextTick = UTime::now();
        this->tickExpectedDuration = US(200);
        this->priority = 2;
    }

    static TaskExecutorScheduler * getInstance() {
        if (!instance) {
            instance = new TaskExecutorScheduler();
        }
        return instance;
    }

    static void loop() {
        uintptr_t v;
        v = rp2040.fifo.pop();
        TaskExecutorBase * te = (TaskExecutorBase*)v;
        te->runAsync();
        rp2040.fifo.push(uintptr_t(te));
    }
};

TaskExecutorScheduler * TaskExecutorScheduler::instance = new TaskExecutorScheduler();
TaskExecutorBase * TaskExecutorBase::first = nullptr;
TaskExecutorBase * TaskExecutorBase::last = nullptr;

TaskExecutorBase::TaskExecutorBase() {
    next = nullptr;
    prev = nullptr;
    state = notProgrammed;
}

TaskExecutorBase::~TaskExecutorBase() {
    if (isPending()) {
        // Remove from the list
        if (prev) {
            prev->next = next;
        } else {
            first = next;
        }
        if (next) {
            next->prev = prev;
        } else {
            last = prev;
        }
    }
}

void TaskExecutorBase::setState(TaskExecutorState state) {
    if (this->state == state) {
        return;
    }

    bool wasPending = this->state != notProgrammed;
    bool newPending = state != notProgrammed;

    this->state = state;
    if  (wasPending != newPending) {
        if (wasPending) {
            // Remove from the list
            if (prev) {
                prev->next = next;
            } else {
                first = next;
            }
            if (next) {
                next->prev = prev;
            } else {
                last = prev;
            }
            next = prev = nullptr;
        } else {
            // Add to the list
            if (last) {
                last->next = this;
                this->prev = last;
                this->next = nullptr;
                last = this;
            } else {
                this->prev = nullptr;
                this->next = nullptr;
                first = last = this;
            }
        }
    }
    if (state == asyncPending) {
        TaskExecutorScheduler::getInstance()->awake();
    }
}

bool TaskExecutorBase::isPending() const {
    return state != notProgrammed;
}

void TaskExecutorBase::schedule() {
#ifdef SYNC_TASK_EXECUTOR
    this->runAsync();
    this->runSync();
#else
    if (isPending()) {
        return;
    }
    setState(asyncPending);
#endif
}


bool TaskExecutorBase::isSync() const {
#ifdef SYNC_TASK_EXECUTOR
    return true;
#else
    return false;
#endif
}


void setup1() {
    // TaskExecutorScheduler::getInstance();
}

void loop1() {
#ifndef SYNC_TASK_EXECUTOR
    TaskExecutorScheduler::loop();
#endif
}
