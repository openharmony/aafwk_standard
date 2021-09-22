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

#include "dataobs_mgr_service.h"

#include <functional>
#include <memory>
#include <string>
#include <unistd.h>
#include "string_ex.h"

#include "dataobs_mgr_errors.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
// #include "sa_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<DataObsMgrService>::GetInstance().get());

DataObsMgrService::DataObsMgrService()
    : SystemAbility(DATAOBS_MGR_SERVICE_SA_ID, true),
      eventLoop_(nullptr),
      handler_(nullptr),
      state_(DataObsServiceRunningState::STATE_NOT_START)
{
    dataObsMgrInner_ = std::make_shared<DataObsMgrInner>();
}

DataObsMgrService::~DataObsMgrService()
{}

void DataObsMgrService::OnStart()
{
    if (state_ == DataObsServiceRunningState::STATE_RUNNING) {
        HILOG_INFO("Dataobs Manager Service has already started.");
        return;
    }
    HILOG_INFO("Dataobs Manager Service started.");
    if (!Init()) {
        HILOG_ERROR("failed to init service.");
        return;
    }
    state_ = DataObsServiceRunningState::STATE_RUNNING;
    eventLoop_->Run();
    /* Publish service maybe failed, so we need call this function at the last,
     * so it can't affect the TDD test program */
    bool ret = Publish(DelayedSingleton<DataObsMgrService>::GetInstance().get());
    if (!ret) {
        HILOG_ERROR("DataObsMgrService::Init Publish failed!");
        return;
    }

    HILOG_INFO("Ability Manager Service start success.");
}

bool DataObsMgrService::Init()
{
    eventLoop_ = AppExecFwk::EventRunner::Create("DataObsMgrService");
    if (eventLoop_ == nullptr) {
        return false;
    }

    handler_ = std::make_shared<AppExecFwk::EventHandler>(eventLoop_);
    if (handler_ == nullptr) {
        return false;
    }

    HILOG_INFO("init success");
    return true;
}

void DataObsMgrService::OnStop()
{
    HILOG_INFO("stop service");
    eventLoop_.reset();
    handler_.reset();
    state_ = DataObsServiceRunningState::STATE_NOT_START;
}

DataObsServiceRunningState DataObsMgrService::QueryServiceState() const
{
    return state_;
}

int DataObsMgrService::RegisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver)
{
    HILOG_INFO("DataObsMgrService::RegisterObserver called start");
    if (dataObserver == nullptr) {
        HILOG_ERROR("DataObsMgrService::RegisterObserver failed!. dataObserver is nullptr");
        return DATA_OBSERVER_IS_NULL;
    }

    if (handler_ == nullptr) {
        HILOG_ERROR("DataObsMgrService::RegisterObserver failed!. handler is nullptr");
        return DATAOBS_SERVICE_HANDLER_IS_NULL;
    }

    if (dataObsMgrInner_ == nullptr) {
        HILOG_ERROR("DataObsMgrService::RegisterObserver failed!. dataObsMgrInner_ is nullptr");
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }

    if (dataObsMgrInner_->CheckNeedLimmit()) {
        return DATAOBS_SERVICE_TASK_LIMMIT;
    }

    if (dataObsMgrInner_->CheckRegisteFull(uri)) {
        return DATAOBS_SERVICE_OBS_LIMMIT;
    }

    std::function<void()> registerObserverFunc =
        std::bind(&DataObsMgrInner::HandleRegisterObserver, dataObsMgrInner_, uri, dataObserver);

    dataObsMgrInner_->AtomicAddTaskCount();
    bool ret = handler_->PostTask(registerObserverFunc);
    if (!ret) {
        dataObsMgrInner_->AtomicSubTaskCount();
        HILOG_ERROR("DataObsMgrService::RegisterObserver PostTask error");
        return DATAOBS_SERVICE_POST_TASK_FAILED;
    }
    HILOG_INFO("DataObsMgrService::RegisterObserver called end");
    return NO_ERROR;
}

int DataObsMgrService::UnregisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver)
{
    HILOG_INFO("DataObsMgrService::UnregisterObserver called start");
    if (dataObserver == nullptr) {
        HILOG_ERROR("DataObsMgrService::UnregisterObserver failed!. dataObserver is nullptr");
        return DATA_OBSERVER_IS_NULL;
    }

    if (handler_ == nullptr) {
        HILOG_ERROR("DataObsMgrService::UnregisterObserver failed!. handler is nullptr");
        return DATAOBS_SERVICE_HANDLER_IS_NULL;
    }

    if (dataObsMgrInner_ == nullptr) {
        HILOG_ERROR("DataObsMgrService::UnregisterObserver failed!. dataObsMgrInner_ is nullptr");
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }

    if (dataObsMgrInner_->CheckNeedLimmit()) {
        return DATAOBS_SERVICE_TASK_LIMMIT;
    }

    std::function<void()> unregisterObserverFunc =
        std::bind(&DataObsMgrInner::HandleUnregisterObserver, dataObsMgrInner_, uri, dataObserver);

    dataObsMgrInner_->AtomicAddTaskCount();
    bool ret = handler_->PostTask(unregisterObserverFunc);
    if (!ret) {
        dataObsMgrInner_->AtomicSubTaskCount();
        HILOG_ERROR("DataObsMgrService::UnregisterObserver PostTask error");
        return DATAOBS_SERVICE_POST_TASK_FAILED;
    }
    HILOG_INFO("DataObsMgrService::UnregisterObserver called end");
    return NO_ERROR;
}

int DataObsMgrService::NotifyChange(const Uri &uri)
{
    HILOG_INFO("DataObsMgrService::NotifyChange called start");
    if (handler_ == nullptr) {
        HILOG_ERROR("DataObsMgrService::NotifyChange failed!. handler is nullptr");
        return DATAOBS_SERVICE_HANDLER_IS_NULL;
    }

    if (dataObsMgrInner_ == nullptr) {
        HILOG_ERROR("DataObsMgrService::NotifyChange failed!. dataObsMgrInner_ is nullptr");
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }

    if (dataObsMgrInner_->CheckNeedLimmit()) {
        return DATAOBS_SERVICE_TASK_LIMMIT;
    }

    std::function<void()> notifyChangeFunc = std::bind(&DataObsMgrInner::HandleNotifyChange, dataObsMgrInner_, uri);

    dataObsMgrInner_->AtomicAddTaskCount();
    bool ret = handler_->PostTask(notifyChangeFunc);
    if (!ret) {
        dataObsMgrInner_->AtomicSubTaskCount();
        HILOG_ERROR("DataObsMgrService::NotifyChange PostTask error");
        return DATAOBS_SERVICE_POST_TASK_FAILED;
    }
    return NO_ERROR;
    HILOG_INFO("DataObsMgrService::NotifyChange called end");
}

std::shared_ptr<EventHandler> DataObsMgrService::GetEventHandler()
{
    return handler_;
}

}  // namespace AAFwk
}  // namespace OHOS
