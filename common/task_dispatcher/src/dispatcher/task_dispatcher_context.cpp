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
#include "task_dispatcher_context.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
TaskDispatcherContext::TaskDispatcherContext()
{
    globalDispatchers_.resize(PRIORITY_COUNT);
    config_ = std::make_shared<DefaultWorkerPoolConfig>();
    if (config_ == nullptr) {
        HILOG_ERROR("TaskDispatcherContext::TaskDispatcherContext config is nullptr");
        executor_ = nullptr;
    } else {
        executor_ = std::make_shared<TaskExecutor>(config_);
        if (executor_ != nullptr) {
            executor_->CreateGuardThread();
        }
    }
}
TaskDispatcherContext::TaskDispatcherContext(const std::shared_ptr<TaskExecutor> &executor)
{
    globalDispatchers_.resize(PRIORITY_COUNT);
    config_ = std::make_shared<DefaultWorkerPoolConfig>();
    executor_ = executor;
}

TaskDispatcherContext::~TaskDispatcherContext()
{
    if (executor_) {
        HILOG_INFO("TaskDispatcherContext::~TaskDispatcherContext() terminate");
        executor_->Terminate(false);
    }
    HILOG_INFO("TaskDispatcherContext::~TaskDispatcherContext end");
}

std::shared_ptr<WorkerPoolConfig> TaskDispatcherContext::GetWorkerPoolConfig() const
{
    return config_;
}

std::map<std::string, long> TaskDispatcherContext::GetWorkerThreadsInfo() const
{
    HILOG_INFO("TaskDispatcherContext::GetWorkerThreadsInfo called");
    if (executor_ != nullptr) {
        return executor_->GetWorkerThreadsInfo();
    }
    std::map<std::string, long> map;
    HILOG_ERROR("TaskDispatcherContext::GetWorkerThreadsInfo executor is nullptr");
    return map;
}

std::map<std::shared_ptr<SerialTaskDispatcher>, std::string> TaskDispatcherContext::GetSerialDispatchers() const
{
    HILOG_INFO("TaskDispatcherContext::GetSerialDispatchers called");
    return serialDispatchers_;
}

int TaskDispatcherContext::GetWaitingTasksCount() const
{
    HILOG_INFO("TaskDispatcherContext::GetWaitingTasksCount called");
    if (executor_ != nullptr) {
        return executor_->GetPendingTasksSize();
    }
    HILOG_ERROR("TaskDispatcherContext::GetWaitingTasksCount executor is nullptr");
    return 0;
}

long TaskDispatcherContext::GetTaskCounter() const
{
    HILOG_INFO("TaskDispatcherContext::GetTaskCounter called");
    if (executor_ != nullptr) {
        return executor_->GetTaskCounter();
    }
    HILOG_ERROR("TaskDispatcherContext::GetTaskCounter executor is nullptr");
    return 0;
}

std::shared_ptr<SerialTaskDispatcher> TaskDispatcherContext::CreateSerialDispatcher(
    const std::string &name, TaskPriority priority)
{
    HILOG_INFO("TaskDispatcherContext::CreateSerialDispatcher start");
    if (executor_ == nullptr) {
        HILOG_ERROR("TaskDispatcherContext::CreateSerialDispatcher executor is nullptr");
        return nullptr;
    }
    std::shared_ptr<SerialTaskDispatcher> serialDispatcher =
        std::make_shared<SerialTaskDispatcher>(name, priority, executor_);
    serialDispatchers_.insert(std::pair<std::shared_ptr<SerialTaskDispatcher>, std::string>(serialDispatcher, name));
    HILOG_INFO("TaskDispatcherContext::CreateSerialDispatcher end");
    return serialDispatcher;
}

std::shared_ptr<ParallelTaskDispatcher> TaskDispatcherContext::CreateParallelDispatcher(
    const std::string &name, TaskPriority priority)
{
    HILOG_INFO("TaskDispatcherContext::CreateParallelDispatcher start");
    if (executor_ == nullptr) {
        HILOG_ERROR("TaskDispatcherContext::CreateParallelDispatcher executor is nullptr");
        return nullptr;
    }
    std::shared_ptr<ParallelTaskDispatcher> parallelTaskDispatcher =
        std::make_shared<ParallelTaskDispatcher>(name, priority, executor_);
    HILOG_INFO("TaskDispatcherContext::CreateParallelDispatcher end");
    return parallelTaskDispatcher;
}

int TaskDispatcherContext::MapPriorityIndex(TaskPriority priority) const
{
    switch (priority) {
        case TaskPriority::HIGH:
            return HIGH_PRIORITY_INDEX;
        case TaskPriority::DEFAULT:
            return DEFAULT_PRIORITY_INDEX;
        case TaskPriority::LOW:
            return LOW_PRIORITY_INDEX;
        default:
            return DEFAULT_PRIORITY_INDEX;
    }
    HILOG_ERROR("TaskDispatcherContext.mapPriorityIndex unhandled priority=%{public}d", priority);

    return DEFAULT_PRIORITY_INDEX;
}

std::shared_ptr<TaskDispatcher> TaskDispatcherContext::GetGlobalTaskDispatcher(TaskPriority priority)
{
    HILOG_INFO("TaskDispatcherContext::GetGlobalTaskDispatcher start");
    std::unique_lock<std::mutex> lock(mtx_);
    int index = MapPriorityIndex(priority);
    std::shared_ptr<TaskDispatcher> dispatcher = globalDispatchers_[index];
    if (dispatcher == nullptr) {
        HILOG_INFO("TaskDispatcherContext::GetGlobalTaskDispatcher dispatcher is nullptr ");
        if (executor_ == nullptr) {
            HILOG_ERROR("TaskDispatcherContext::GetGlobalTaskDispatcher executor_ is nullptr ");
            return nullptr;
        }
        dispatcher = std::make_shared<GlobalTaskDispatcher>(priority, executor_);
        if (globalDispatchers_[index] == nullptr) {
            HILOG_INFO("TaskDispatcherContext::GetGlobalTaskDispatcher dispatcher compareAndSet ");
            globalDispatchers_.insert((globalDispatchers_.begin() + index), dispatcher);
        }
    }
    HILOG_INFO("TaskDispatcherContext::GetGlobalTaskDispatcher end");
    return dispatcher;
}

ErrCode TaskDispatcherContext::Shutdown(bool force)
{
    HILOG_INFO("TaskDispatcherContext::Shutdown start");
    if (executor_ == nullptr) {
        HILOG_ERROR("TaskDispatcherContext::Shutdown executor_ is nullptr");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }
    executor_->Terminate(force);
    HILOG_INFO("TaskDispatcherContext::Shutdown end");
    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS
