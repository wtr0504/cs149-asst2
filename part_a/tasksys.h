#ifndef _TASKSYS_H
#define _TASKSYS_H

#include "itasksys.h"
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <future>
#include <condition_variable>
#include <queue>
class ThreadState{
    public:
        std::condition_variable* condition_variable_;
        std::mutex* mutex_;
        int num_total_tasks;
        std::atomic_int done_task_;
        IRunnable *runnable;
    ThreadState(){
            condition_variable_ = new std::condition_variable();
            mutex_ = new std::mutex();
            done_task_ = -1;
            num_total_tasks = -1;
        }
        ~ThreadState(){
            delete condition_variable_;
            delete mutex_;
        }
};
/*
 * TaskSystemSerial: This class is the student's implementation of a
 * serial task execution engine.  See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemSerial: public ITaskSystem {
    public:
        TaskSystemSerial(int num_threads);
        ~TaskSystemSerial();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelSpawn: This class is the student's implementation of a
 * parallel task execution engine that spawns threads in every run()
 * call.  See definition of ITaskSystem in itasksys.h for documentation
 * of the ITaskSystem interface.
 */
class TaskSystemParallelSpawn: public ITaskSystem {
    public:
        TaskSystemParallelSpawn(int num_threads);
        ~TaskSystemParallelSpawn();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();


};

/*
 * TaskSystemParallelThreadPoolSpinning: This class is the student's
 * implementation of a parallel task execution engine that uses a
 * thread pool. See definition of ITaskSystem in itasksys.h for
 * documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSpinning: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSpinning(int num_threads);
        ~TaskSystemParallelThreadPoolSpinning();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
        void threadRunTask();
    private:
        std::vector<std::thread> threads_;
        std::queue<int> task_queue_;
        ThreadState *thread_state_;
        bool killed;
};

/*
 * TaskSystemParallelThreadPoolSleeping: This class is the student's
 * optimized implementation of a parallel task execution engine that uses
 * a thread pool. See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class sleepState{
    public:
        std::mutex *mt_;
        std::condition_variable *cv_;
        int num_total_tasks;
        int done_task_;
        IRunnable *runnable;
        sleepState(){
            mt_ = new std::mutex();
            cv_ = new std::condition_variable();
            num_total_tasks = -1;
            done_task_ = -1;
        }
        ~sleepState(){
            delete cv_;
            delete mt_;
        }
};

class TaskSystemParallelThreadPoolSleeping: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSleeping(int num_threads);
        ~TaskSystemParallelThreadPoolSleeping();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
        void threadRunTask();
    private:
            std::condition_variable queue_cv_;
            std::mutex queue_mt_;
            std::vector<std::thread> threads_;
            std::queue<int> task_queue_;
            sleepState* sleepstate_;
            bool killed_;
};

#endif
