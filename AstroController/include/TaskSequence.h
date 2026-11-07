#ifndef TASKSEQUENCE_H_
#define TASKSEQUENCE_H_

#include "Scheduled.h"
#include "TaskQueue.h"
#include "utime.h"

#define  SEQUENCE_LOOP_HERE 1

template<class Scheduled>
using TaskAsyncFunction = bool (Scheduled::*)();

template<class Scheduled>
using TaskSyncFunction = void (Scheduled::*)();

template<class Scheduled>
using TaskFailureFunction = void (Scheduled::*)();

template<class Scheduled>
struct TaskSequence {
    TaskAsyncFunction<Scheduled> asyncFunc;
    uint16_t asyncMsDuration;
    TaskSyncFunction<Scheduled> syncFunc;
    uint16_t msWait;
    uint8_t flag;
};

template<class Scheduled>
class TaskSequenceScheduler {
    TaskSequence<Scheduled> * seq;
    int stepId;
    Scheduled * scheduled;
    TaskExecutor<bool> taskExecutor;
    TaskFailureFunction<Scheduled> onFailure;

    void stepFinished(bool success) {
        if (!success) {
            stepId = -1;
            scheduled->nextTick = UTime::never();
            (scheduled->*onFailure)();
            return;
        }
        auto stepDef = seq[stepId];
        auto syncFunc = stepDef.syncFunc;

        // Prepare for the loop. The sync function can stop the loop
        scheduled->nextTick = UTime::now() + MS(stepDef.msWait);
        // Then next step
        stepId++;
        if (seq[stepId].asyncFunc == nullptr && seq[stepId].syncFunc == nullptr) {
            stepId = 0;
            for(int i = 0; seq[i].asyncFunc != nullptr || seq[i].syncFunc != nullptr; i++) {
                if (seq[i].flag & SEQUENCE_LOOP_HERE) {
                    stepId = i;
                    break;
                }
            }
        }
        // Adjust the expected duration
        if (taskExecutor.isSync()) {
            scheduled->tickExpectedDuration = MS(seq[stepId].asyncMsDuration);
        } else {
            scheduled->tickExpectedDuration = US(100);
        }

        if (syncFunc != nullptr) {
            // Run the sync func
            (scheduled->*syncFunc)();
        }
    }

    void runSequence() {
        auto stepDef = seq[stepId];

        // asyncFunc or syncFunc are always set
        if (stepDef.asyncFunc != nullptr) {
            // Run the async func then the callback
            taskExecutor.schedule(std::bind(stepDef.asyncFunc, this->scheduled),
                                  [this](bool success) { this->stepFinished(success); },
                                  MS(stepDef.asyncMsDuration));
        } else {
            // Directly run the callback
            this->stepFinished(true);
        }
    }
public:
    TaskSequenceScheduler(Scheduled * scheduled, TaskSequence<Scheduled> * seq, TaskFailureFunction<Scheduled> onFailure)
        : seq(seq), stepId(-1), scheduled(scheduled), onFailure(onFailure)
    {
    }

    bool handleTick() {
        if (stepId == -1) {
            return false;
        }
        runSequence();
        return true;
    }

    // This must be called only from a sync callback between steps
    // This release the scheduler. So any adjustment to nextTick
    // must be done after this call
    // It is also safe to be called if not started
    void stop() {
        if (this->stepId != -1) {
            this->stepId = -1;
            scheduled->nextTick = UTime::never();
        }
    }

    void start(const LongDuration & within) {
        stepId = 0;
        // Adjust the expected duration
        scheduled->nextTick = UTime::now() + within;
        scheduled->tickExpectedDuration = taskExecutor.isSync()
                    ? seq[stepId].asyncMsDuration
                    : US(100);
    }
};

#endif
