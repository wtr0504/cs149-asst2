#include "tasksys.h"
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <functional>
IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads): num_threads_(num_threads){}
ITaskSystem::~ITaskSystem() {}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char* TaskSystemSerial::name() {
    return "Serial";
}

TaskSystemSerial::TaskSystemSerial(int num_threads): ITaskSystem(num_threads) {
}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable* runnable, int num_total_tasks) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                          const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemSerial::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelSpawn::name() {
    return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //

}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}


void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    std::vector<std::thread> workers;
    std::atomic_int current_task{0};


    for(int i=0;i<num_threads_;i++){
        workers.push_back(std::thread([&current_task,num_total_tasks,runnable]{
            int tm;
            while((tm = current_task++) < num_total_tasks){
                runnable->runTask(tm,num_total_tasks);
            }
        }));
    }
    for(int i= 0;i<num_threads_;i++){
        workers[i].join();
    }



    // for (int i = 0; i < num_total_tasks; i++) {
    //     runnable->runTask(i, num_total_tasks);
    // }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSpinning::name() {
    return "Parallel + Thread Pool + Spin";
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads)
                                                                                             {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    killed = false;
    thread_state_ = new ThreadState();
    for (int i=0;i<num_threads;i++) {
        threads_.emplace_back(&TaskSystemParallelThreadPoolSpinning::threadRunTask, this);
    }
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {
    killed = true;
    for (int i = 0; i < num_threads_; i++) {
        threads_[i].join();
    }
    delete thread_state_;
}




void TaskSystemParallelThreadPoolSpinning::threadRunTask() {
    int task_index;
    while(!killed){
        task_index = -1;
        thread_state_->mutex_->lock();
        if(!task_queue_.empty()){
            task_index = task_queue_.front();
            task_queue_.pop();
        }
        thread_state_->mutex_->unlock();
        if(task_index != -1){
//            printf("taskid:%d\n",task_index);
            thread_state_->runnable->runTask(task_index,thread_state_->num_total_tasks);
            thread_state_->done_task_++;
            if(thread_state_->done_task_ == thread_state_->num_total_tasks){
                thread_state_->condition_variable_->notify_one();
            }
        }
    }
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {

    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    std::unique_lock<std::mutex> lk(*thread_state_->mutex_);
    this->thread_state_->runnable = runnable;
    this->thread_state_->done_task_ = 0;
    this->thread_state_->num_total_tasks = num_total_tasks;
    for(int i = 0;i<num_total_tasks;i++){
        task_queue_.push(i);
    }

    thread_state_->condition_variable_->wait(lk,[this]{
        return thread_state_->done_task_ == thread_state_->num_total_tasks;});

}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSleeping::name() {
    return "Parallel + Thread Pool + Sleep";
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).

    killed_ = false;
    sleepstate_ = new sleepState();
    for(int i = 0;i<num_threads;i++){
        threads_.emplace_back(&TaskSystemParallelThreadPoolSleeping::threadRunTask,this);
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    killed_ = true;
    queue_cv_.notify_all();
    for(int i = 0;i < num_threads_;i++){
        threads_[i].join();
    }
    delete sleepstate_;
}

void TaskSystemParallelThreadPoolSleeping::threadRunTask() {
    int task_index;
//    while(true){
//        while(true){
//            std::unique_lock<std::mutex> lk(queue_mt_);
//            queue_cv_.wait(lk,[]{return true;});
//            if(killed_) return ;
//            if(task_queue_.empty()) continue;
//            task_index = task_queue_.front();
//            task_queue_.pop();
//            break;
//        }
//        sleepstate_->runnable->runTask(task_index,sleepstate_->num_total_tasks);
//        std::unique_lock<std::mutex> lock(*sleepstate_->mt_);
//        sleepstate_->done_task_++;
//        if(sleepstate_->done_task_ < sleepstate_->num_total_tasks){
//            queue_cv_.notify_all();
//        } else{
//            sleepstate_->cv_->notify_one();
//        }
//    }
    while(!killed_){
        task_index = -1;
        {
            std::unique_lock<std::mutex> lk(queue_mt_);
            queue_cv_.wait(lk,[]{return true;});
            if(!task_queue_.empty()){
                task_index = task_queue_.front();
                task_queue_.pop();
                //printf("task_index:%d\n",(int)task_index);
            }
        }
        if(task_index != -1){
            sleepstate_->runnable->runTask(task_index,sleepstate_->num_total_tasks);
            std::unique_lock<std::mutex> lock(*sleepstate_->mt_);
            sleepstate_->done_task_++;
            if(sleepstate_->done_task_ == sleepstate_->num_total_tasks ){
                sleepstate_->cv_->notify_one();
            }else{
                queue_cv_.notify_all();
            }
        }

    }
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    this->sleepstate_->runnable = runnable;
    this->sleepstate_->num_total_tasks = num_total_tasks;
    this->sleepstate_->done_task_= 0;
    queue_mt_.lock();
    for(int i = 0;i<num_total_tasks;i++){
        task_queue_.push(i);
    }
    queue_mt_.unlock();
    queue_cv_.notify_all();

//    while(true){
//        std::unique_lock<std::mutex> lk(*sleepstate_->mt_);
//        sleepstate_->cv_->wait(lk,[]{return true;});
//        if(sleepstate_->done_task_ == sleepstate_->num_total_tasks) return;
//    }
    std::unique_lock<std::mutex> lk(*sleepstate_->mt_);
    sleepstate_->cv_->wait(lk,[this]{return sleepstate_->done_task_ == sleepstate_->num_total_tasks;});
    return ;
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
