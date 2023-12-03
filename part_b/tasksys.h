#ifndef _TASKSYS_H
#define _TASKSYS_H

#include "itasksys.h"
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <map>
#include <set>
#include <optional>
#include <tuple>
#include <atomic>
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
    sleepState(){
        mt_ = new std::mutex();
        cv_ = new std::condition_variable();
    }
    ~sleepState(){
        delete cv_;
        delete mt_;
    }
};
class TaskInfo{
    public:
        TaskID task_id_;

        std::atomic_int done_num_{0};
        int total_num_;
        IRunnable *runnable;
        std::set<TaskID> left_deps_;
        std::queue<int> little_task_queue_;

        TaskInfo(TaskID id,int num_total_task,IRunnable *runnable,const std::vector<TaskID>& deps):task_id_(id),
        total_num_(num_total_task),
        runnable(runnable),
        left_deps_(deps.begin(),deps.end()){
            for(int i = 0;i<num_total_task;i++){
                little_task_queue_.push(i);
            }
        }
};
template class std::map<TaskID,TaskInfo*>;
class TaskSystemParallelThreadPoolSleeping: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSleeping(int num_threads);
        ~TaskSystemParallelThreadPoolSleeping();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();


    private:
        void threadRunTask();
        void threadRunTaskWithDep();
        void taskHasDone(TaskID);
        int  current_task_id_;//现在调用runAsyncWithDeps后的taskid
        std::map<TaskID,std::shared_ptr<TaskInfo>> task_to_task_info_;
        std::vector<TaskID> waiting_tasks_;
        std::queue<TaskID> ready_tasks_;
        std::set<TaskID> finish_tasks_;
        std::atomic<int> running_tasks_;

        std::mutex *run_mt_;
        std::condition_variable *run_cv_;
        std::condition_variable *finish_cv_;

        std::vector<std::thread> threads_;
        bool killed_;

};

#endif
