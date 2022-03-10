/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "spec_task_dispatcher.h"
#include "task_handler_libevent_adapter.h"

namespace OHOS {
namespace AppExecFwk {
std::string SpecTaskDispatcher::DISPATCHER_TAG = "SpecTaskDispatcher";
std::string SpecTaskDispatcher::ASYNC_DISPATCHER_TAG = DISPATCHER_TAG + "::asyncDispatch";
std::string SpecTaskDispatcher::SYNC_DISPATCHER_TAG = DISPATCHER_TAG + "::syncDispatch";
std::string SpecTaskDispatcher::DELAY_DISPATCHER_TAG = DISPATCHER_TAG + "::delayDispatch";
SpecTaskDispatcher::SpecTaskDispatcher(
    std::shared_ptr<SpecDispatcherConfig> config, std::shared_ptr<EventRunner> runner)
    : BaseTaskDispatcher(config->GetName(), config->GetPriority())
{
    handler_ = std::make_shared<TaskHandlerLibeventAdapter>(runner);
}

ErrCode SpecTaskDispatcher::SyncDispatch(const std::shared_ptr<Runnable> &runnable)
{
    HILOG_INFO("SpecTaskDispatcher::SyncDispatch start");
    if (handler_ == nullptr) {
        HILOG_ERROR("SpecTaskDispatcher::SyncDispatch handler is nullptr");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }
    if (Check(runnable) != ERR_OK) {
        HILOG_ERROR("SpecTaskDispatcher::SyncDispatch check failed");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }

    std::shared_ptr<Task> innerTask = std::make_shared<Task>(runnable, GetPriority(), shared_from_this());
    if (innerTask == nullptr) {
        HILOG_ERROR("SpecTaskDispatcher::SyncDispatch innerTask is nullptr");
        return ERR_APPEXECFWK_CHECK_FAILED;
    }
    TracePointBeforePost(innerTask, false, SYNC_DISPATCHER_TAG);
    HILOG_INFO("SpecTaskDispatcher::SyncDispatch into new sync task");
    handler_->DispatchSync(runnable);
    TracePointAfterPost(innerTask, false, DISPATCHER_TAG);

    HILOG_INFO("SpecTaskDispatcher::SyncDispatch end");
    return ERR_OK;
}

std::shared_ptr<Revocable> SpecTaskDispatcher::AsyncDispatch(const std::shared_ptr<Runnable> &runnable)
{
    HILOG_INFO("SpecTaskDispatcher::AsyncDispatch start");
    if (handler_ == nullptr) {
        HILOG_ERROR("SpecTaskDispatcher::AsyncDispatch handler is nullptr");
        return nullptr;
    }
    if (Check(runnable) != ERR_OK) {
        HILOG_ERROR("SpecTaskDispatcher::AsyncDispatch check failed");
        return nullptr;
    }

    std::shared_ptr<Task> innerTask = std::make_shared<Task>(runnable, GetPriority(), shared_from_this());
    if (innerTask == nullptr) {
        HILOG_ERROR("SpecTaskDispatcher::AsyncDispatch innerTask is nullptr");
        return nullptr;
    }
    TracePointBeforePost(innerTask, true, ASYNC_DISPATCHER_TAG);
    HILOG_INFO("SpecTaskDispatcher::AsyncDispatch into new async task");
    handler_->Dispatch(runnable);
    HILOG_INFO("SpecTaskDispatcher::AsyncDispatch end");
    return innerTask;
}

std::shared_ptr<Revocable> SpecTaskDispatcher::DelayDispatch(const std::shared_ptr<Runnable> &runnable, long delayMs)
{
    HILOG_INFO("SpecTaskDispatcher::DelayDispatch start");
    if (handler_ == nullptr) {
        HILOG_ERROR("SpecTaskDispatcher::DelayDispatch handler is nullptr");
        return nullptr;
    }
    if (Check(runnable) != ERR_OK) {
        HILOG_ERROR("SpecTaskDispatcher::DelayDispatch check failed");
        return nullptr;
    }

    std::shared_ptr<Task> innerTask = std::make_shared<Task>(runnable, GetPriority(), shared_from_this());
    if (innerTask == nullptr) {
        HILOG_ERROR("SpecTaskDispatcher::DelayDispatch innerTask is nullptr");
        return nullptr;
    }
    TracePointBeforePost(innerTask, true, DELAY_DISPATCHER_TAG);
    handler_->Dispatch(runnable, delayMs);
    HILOG_INFO("SpecTaskDispatcher::DelayDispatch end");
    return innerTask;
}
}  // namespace AppExecFwk
}  // namespace OHOS
