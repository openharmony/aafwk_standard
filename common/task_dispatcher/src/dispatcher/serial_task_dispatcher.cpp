/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "serial_task_dispatcher.h"
#include "hilog_wrapper.h"
#include "task_errors.h"
namespace OHOS {
namespace AppExecFwk {
std::string SerialTaskDispatcher::DISPATCHER_TAG = "SerialTaskDispatcher";
std::string SerialTaskDispatcher::ASYNC_DISPATCHER_TAG = DISPATCHER_TAG + "::asyncDispatch";
std::string SerialTaskDispatcher::SYNC_DISPATCHER_TAG = DISPATCHER_TAG + "::syncDispatch";
std::string SerialTaskDispatcher::DELAY_DISPATCHER_TAG = DISPATCHER_TAG + "::delayDispatch";

SerialTaskDispatcher::SerialTaskDispatcher(
    const std::string &dispatcherName, const TaskPriority priority, const std::shared_ptr<TaskExecutor> &executor)
    : BaseTaskDispatcher(dispatcherName, priority)
{
    running_ = false;
    executor_ = executor;
}

int SerialTaskDispatcher::GetWorkingTasksSize()
{
    return workingTasks_.Size();
}

std::string SerialTaskDispatcher::GetDispatcherName()
{
    return dispatcherName_;
}

ErrCode SerialTaskDispatcher::SyncDispatch(const std::shared_ptr<Runnable> &runnable)
{
    HILOG_INFO("SerialTaskDispatcher::SyncDispatch start");
    if (Check(runnable) != ERR_OK) {
        HILOG_ERROR("SerialTaskDispatcher::SyncDispatch check failed");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }

    std::shared_ptr<SyncTask> innerSyncTask = std::make_shared<SyncTask>(runnable, GetPriority(), shared_from_this());
    if (innerSyncTask == nullptr) {
        HILOG_ERROR("SerialTaskDispatcher::SyncDispatch innerSyncTask is nullptr");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }
    std::shared_ptr<Task> innerTask = std::static_pointer_cast<Task>(innerSyncTask);
    if (innerTask == nullptr) {
        HILOG_ERROR("SerialTaskDispatcher::SyncDispatch innerTask is nullptr");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }
    TracePointBeforePost(innerTask, false, SYNC_DISPATCHER_TAG);
    OnNewTaskIn(innerTask);
    innerSyncTask->WaitTask();
    TracePointAfterPost(innerTask, false, DISPATCHER_TAG);

    HILOG_INFO("SerialTaskDispatcher::SyncDispatch end");
    return ERR_OK;
}

std::shared_ptr<Revocable> SerialTaskDispatcher::AsyncDispatch(const std::shared_ptr<Runnable> &runnable)
{
    HILOG_INFO("SerialTaskDispatcher::AsyncDispatch start");
    if (Check(runnable) != ERR_OK) {
        HILOG_ERROR("SerialTaskDispatcher::AsyncDispatch Check failed");
        return nullptr;
    }

    std::shared_ptr<Task> innerTask = std::make_shared<Task>(runnable, GetPriority(), shared_from_this());
    if (innerTask == nullptr) {
        HILOG_ERROR("SerialTaskDispatcher::AsyncDispatch innerTask is nullptr");
        return nullptr;
    }
    TracePointBeforePost(innerTask, true, ASYNC_DISPATCHER_TAG);
    HILOG_INFO("SerialTaskDispatcher::AsyncDispatch into new async task");
    OnNewTaskIn(innerTask);
    HILOG_INFO("SerialTaskDispatcher::AsyncDispatch end");
    return innerTask;
}

std::shared_ptr<Revocable> SerialTaskDispatcher::DelayDispatch(const std::shared_ptr<Runnable> &runnable, long delayMs)
{
    HILOG_INFO("SerialTaskDispatcher::DelayDispatch start");
    if (executor_ == nullptr) {
        HILOG_ERROR("SerialTaskDispatcher::DelayDispatch executor_ is nullptr");
        return nullptr;
    }
    if (Check(runnable) != ERR_OK) {
        HILOG_ERROR("SerialTaskDispatcher::DelayDispatch Check failed");
        return nullptr;
    }

    std::shared_ptr<Task> innerTask = std::make_shared<Task>(runnable, GetPriority(), shared_from_this());
    if (innerTask == nullptr) {
        HILOG_ERROR("SerialTaskDispatcher::DelayDispatch innerTask is nullptr");
        return nullptr;
    }
    TracePointBeforePost(innerTask, true, DELAY_DISPATCHER_TAG);
    // bind parameter to avoid deconstruct.
    std::function<void()> callback = std::bind(&SerialTaskDispatcher::OnNewTaskIn, this, innerTask);
    bool executeFlag = executor_->DelayExecute(callback, delayMs);
    if (!executeFlag) {
        HILOG_ERROR("SerialTaskDispatcher::DelayDispatch execute failed");
        return nullptr;
    }
    HILOG_INFO("SerialTaskDispatcher::DelayDispatch end");
    return innerTask;
}

ErrCode SerialTaskDispatcher::OnNewTaskIn(std::shared_ptr<Task> &task)
{
    HILOG_INFO("SerialTaskDispatcher::OnNewTaskIn start");
    ErrCode code = Prepare(task);
    if (code != ERR_OK) {
        HILOG_ERROR("SerialTaskDispatcher::OnNewTaskIn Prepare failed");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (workingTasks_.Offer(task) == false) {
            HILOG_WARN("SerialTaskDispatcher.onNewTaskIn exceed the maximum capacity of Queue");
        }
    }

    Schedule();
    HILOG_INFO("SerialTaskDispatcher::OnNewTaskIn end");
    return ERR_OK;
}

ErrCode SerialTaskDispatcher::Prepare(std::shared_ptr<Task> &task)
{
    HILOG_INFO("SerialTaskDispatcher::Prepare start");
    if (task == nullptr) {
        HILOG_ERROR("SerialTaskDispatcher::Prepare task is nullptr");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }
    // inline class
    class MyTaskListener : public TaskListener {
    private:
        std::function<void()> callback_;

    public:
        void OnChanged(const TaskStage &stage)
        {
            if (stage.IsDone()) {
                callback_();
            }
        }
        // set callback function
        void Callback(const std::function<void()> &callbackFunction)
        {
            callback_ = std::move(callbackFunction);
        }
    };

    // set inline listener
    std::shared_ptr<MyTaskListener> ptrlistener = std::make_shared<MyTaskListener>();
    if (ptrlistener == nullptr) {
        HILOG_ERROR("SerialTaskDispatcher::Prepare MyTaskListener is nullptr");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }
    const std::function<void()> onTaskDone = [&]() { OnTaskDone(); };
    ptrlistener->Callback(onTaskDone);
    task->AddTaskListener(ptrlistener);
    HILOG_INFO("SerialTaskDispatcher::Prepare end");
    return ERR_OK;
}

void SerialTaskDispatcher::OnTaskDone()
{
    HILOG_INFO("SerialTaskDispatcher::OnTaskDone start");
    bool isExhausted = workingTasks_.Empty();
    DoNext(isExhausted);
    HILOG_INFO("SerialTaskDispatcher::OnTaskDone end");
}

bool SerialTaskDispatcher::Schedule()
{
    bool init = false;
    if (!running_.compare_exchange_strong(init, true)) {
        HILOG_WARN("SerialTaskDispatcher::schedule already running");
        return false;
    }
    HILOG_INFO("SerialTaskDispatcher::Schedule do next");
    return DoNext(false);
}

bool SerialTaskDispatcher::DoNext(bool isExhausted)
{
    HILOG_INFO("SerialTaskDispatcher::DoNext start");
    std::shared_ptr<Task> nextptr = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        nextptr = workingTasks_.Poll();
        if (nextptr == nullptr) {
            running_.store(false);
            HILOG_WARN("SerialTaskDispatcher::DoNext no more task");
            return false;
        }
    }

    DoWork(nextptr);
    HILOG_INFO("SerialTaskDispatcher::DoNext end");
    return true;
}

void SerialTaskDispatcher::DoWork(std::shared_ptr<Task> &task)
{
    HILOG_INFO("SerialTaskDispatcher::DoWork called.");
    // |task| mustn't be null
    executor_->Execute(task);
}
}  // namespace AppExecFwk
}  // namespace OHOS