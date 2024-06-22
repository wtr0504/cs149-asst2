#include "tasksys.h"
#include <algorithm>
#include <iostream>
IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads):num_threads_(num_threads) {}
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
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemSerial::sync() {
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
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
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

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
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
    //
    killed_ = false;
    current_task_id_ = 0;
    running_tasks_ = 0;
    run_cv_ = new std::condition_variable();
    finish_cv_ = new std::condition_variable();
    run_mt_ = new std::mutex();
    for(int i = 0;i<num_threads;i++){
        threads_.emplace_back(&TaskSystemParallelThreadPoolSleeping::threadRunTaskWithDep,this);
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
    run_cv_->notify_all();
    for(auto &th : threads_){
        th.join();
    }
//    for(auto &tmp : task_to_task_info_){
//        delete tmp.second;
//    }
    delete run_cv_;
    delete run_mt_;
    delete finish_cv_;
}

void TaskSystemParallelThreadPoolSleeping::threadRunTask() {

}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {

    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    //use runAsyncWithDeps and sync
    std::vector<TaskID> nodeps;
    runAsyncWithDeps(runnable, num_total_tasks, nodeps);
    sync();
}

void TaskSystemParallelThreadPoolSleeping::threadRunTaskWithDep(){
    while(!killed_){
        std::unique_lock<std::mutex> lk(*run_mt_);
        run_cv_->wait(lk,[this]{return !ready_tasks_.empty() || killed_ || !waiting_tasks_.empty();});
        //当ready_tasks_有元素时运行
        if(ready_tasks_.empty()){
            lk.unlock();
            continue;
        }else{
            int task_id = ready_tasks_.front();
//            TaskInfo *p = task_to_task_info_[task_id];
            int little_task_index = -1;
            if(!task_to_task_info_[task_id]->little_task_queue_.empty()){
                little_task_index = task_to_task_info_[task_id]->little_task_queue_.front();
                task_to_task_info_[task_id]->little_task_queue_.pop();
            }
            if(little_task_index != -1){
                lk.unlock();
                task_to_task_info_[task_id]->runnable->runTask(little_task_index,
                                                               task_to_task_info_[task_id]->total_num_);
                lk.lock();
                task_to_task_info_[task_id]->done_num_++;
//                std::cout<<task_to_task_info_[task_id]->done_num_<<std::endl;
                if(task_to_task_info_[task_id]->done_num_ == task_to_task_info_[task_id]->total_num_){
                    finish_tasks_.insert(task_id);
                    ready_tasks_.pop();
                    task_to_task_info_.erase(task_id);
                    for(auto it = waiting_tasks_.begin();it!=waiting_tasks_.end();){
                        task_to_task_info_[*it]->left_deps_.erase(task_id);
                        if(task_to_task_info_[*it]->left_deps_.empty()){
                            ready_tasks_.push(*it);
                            it = waiting_tasks_.erase(it);
                        }else{
                            it++;
                        }
                    }
                }else{
                    run_cv_->notify_all();
                }
            }
            if(ready_tasks_.empty() && waiting_tasks_.empty()){
                finish_cv_->notify_one();
            }
        }

    }

}

//void TaskSystemParallelThreadPoolSleeping::taskHasDone(TaskID id) {
//    task_task_num_[id].finish_ = true;
//    for(auto &tmpdep : taskDep_){
//        auto iter = std::find_if(tmpdep.second.begin(),tmpdep.second.end(),
//                     [id](const std::pair<TaskID,bool>& id_and_fin){return id == id_and_fin.first;});
//        if(iter != tmpdep.second.end()){
//            iter->second = true;
//            auto it = std::find_if(tmpdep.second.begin(),tmpdep.second.end(),
//                                   [](const std::pair<TaskID,bool>& id_and_fin){return id_and_fin.second;});
//            if(it == tmpdep.second.end()){
//                task_task_num_[tmpdep.first].start_ = true;//所有依赖的任务全部完成，此任务启动
//                ready_task_.push(tmpdep.first);
//                taskDep_.erase(tmpdep.first);
//            }
//        }
//    }
//}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {
    //
    // TODO: CS149 students will implement this method in Part B.
    //


    run_mt_->lock();
    task_to_task_info_[current_task_id_] = std::make_shared<TaskInfo>(current_task_id_,num_total_tasks,runnable,deps) ;
    for(auto & tmp : deps){
        if(finish_tasks_.find(tmp) != finish_tasks_.end()){
            task_to_task_info_[current_task_id_]->left_deps_.erase(tmp);//如果完成了就擦除
        }
    }
    if(task_to_task_info_[current_task_id_]->left_deps_.empty()){
        ready_tasks_.push(current_task_id_);
//        running_tasks_++;
    }else{
        waiting_tasks_.push_back(current_task_id_);
    }
    run_mt_->unlock();
    run_cv_->notify_all();

    return current_task_id_++;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    std::unique_lock<std::mutex> lk(*run_mt_);
    finish_cv_->wait(lk,[this]{return ready_tasks_.empty()&&waiting_tasks_.empty();});

    return;
}
