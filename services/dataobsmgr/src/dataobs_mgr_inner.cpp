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
#include "dataobs_mgr_inner.h"
#include "dataobs_mgr_errors.h"
#include "hilog_wrapper.h"
#include "data_ability_observer_stub.h"

namespace OHOS {
namespace AAFwk {

std::mutex DataObsMgrInner::innerMutex_;

DataObsMgrInner::DataObsMgrInner()
{
    taskCount_.store(0);
}

DataObsMgrInner::~DataObsMgrInner()
{
    taskCount_.store(0);
}

void DataObsMgrInner::SetHandler(const std::shared_ptr<EventHandler> &handler)
{
    handler_ = handler;
}

int DataObsMgrInner::HandleRegisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver)
{
    HILOG_INFO("DataObsMgrInner::HandleRegisterObserver called start");
    std::lock_guard<std::mutex> lock_l(innerMutex_);

    ObsListType obslist;
    bool exist = GetObsListFromMap(uri, obslist);

    auto obs = std::find(obslist.begin(), obslist.end(), dataObserver);
    if (obs != obslist.end()) {
        HILOG_ERROR("DataObsMgrInner::HandleRegisterObserver the obs exist. no need to register.");
        return OBS_EXIST;
    }

    obslist.push_back(dataObserver);

    AddObsDeathRecipient(dataObserver);

    if (exist) {
        obsmap_.erase(uri.ToString());
    }

    obsmap_.emplace(uri.ToString(), obslist);

    AtomicSubTaskCount();

    HILOG_INFO("DataObsMgrInner::HandleRegisterObserver called end");
    return NO_ERROR;
}

int DataObsMgrInner::HandleUnregisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver)
{
    HILOG_INFO("DataObsMgrInner::HandleUnregisterObserver called start");
    std::lock_guard<std::mutex> lock_l(innerMutex_);

    ObsListType obslist;
    if (!GetObsListFromMap(uri, obslist)) {
        AtomicSubTaskCount();
        HILOG_ERROR("DataObsMgrInner::HandleUnregisterObserver there is no obs in the uri.");
        return NO_OBS_FOR_URI;
    }

    HILOG_INFO("DataObsMgrInner::HandleUnregisterObserver obslist size is %{public}zu", obslist.size());
    auto obs = obslist.begin();
    for (; obs != obslist.end(); obs++) {
        if ((*obs)->AsObject() == dataObserver->AsObject()) {
            break;
        }
    }
    if (obs == obslist.end()) {
        AtomicSubTaskCount();
        HILOG_ERROR("DataObsMgrInner::HandleUnregisterObserver the obs is not registered to the uri.");
        return NO_OBS_FOR_URI;
    }
    sptr<IDataAbilityObserver> removeObs = *obs;
    obslist.remove(removeObs);
    obsmap_.erase(uri.ToString());
    if (!obslist.empty()) {
        obsmap_.emplace(uri.ToString(), obslist);
    }

    if (!ObsExistInMap(removeObs)) {
        RemoveObsDeathRecipient(removeObs);
    }

    AtomicSubTaskCount();
    HILOG_INFO("DataObsMgrInner::HandleUnregisterObserver called end");
    return NO_ERROR;
}

int DataObsMgrInner::HandleNotifyChange(const Uri &uri)
{
    HILOG_INFO("DataObsMgrInner::HandleNotifyChange called start");
    std::lock_guard<std::mutex> lock_l(innerMutex_);

    ObsListType obslist;
    if (!GetObsListFromMap(uri, obslist)) {
        AtomicSubTaskCount();
        HILOG_INFO("DataObsMgrInner::HandleNotifyChange there is no obs in the uri.");
        return NO_OBS_FOR_URI;
    }

    for (auto obs : obslist) {
        if (obs != nullptr) {
            obs->OnChange();
        }
    }

    AtomicSubTaskCount();
    HILOG_INFO("DataObsMgrInner::HandleNotifyChange called end %{public}zu", obslist.size());
    return NO_ERROR;
}

bool DataObsMgrInner::CheckNeedLimmit()
{
    return (taskCount_.load() >= taskCount_max_) ? true : false;
}

bool DataObsMgrInner::CheckRegisteFull(const Uri &uri)
{
    std::lock_guard<std::mutex> lock_l(innerMutex_);

    ObsListType obslist;
    if (GetObsListFromMap(uri, obslist)) {
        // The obs size for input uri has been lager than max.
        if (obslist.size() >= obs_max_) {
            return true;
        }
    }
    return false;
}

void DataObsMgrInner::AtomicAddTaskCount()
{
    taskCount_.fetch_add(1);
}

void DataObsMgrInner::AtomicSubTaskCount()
{
    taskCount_.fetch_sub(1);
}

bool DataObsMgrInner::GetObsListFromMap(const Uri &uri, ObsListType &obslist)
{
    auto it = obsmap_.find(uri.ToString());
    if (it == obsmap_.end()) {
        return false;
    }

    obslist = it->second;
    return true;
}

void DataObsMgrInner::AddObsDeathRecipient(const sptr<IDataAbilityObserver> &dataObserver)
{
    if ((dataObserver == nullptr) || dataObserver->AsObject() == nullptr) {
        return;
    }

    auto it = recipientMap_.find(dataObserver->AsObject());
    if (it != recipientMap_.end()) {
        HILOG_ERROR("%{public}s this death recipient has been added.", __func__);
        return;
    } else {
        sptr<IRemoteObject::DeathRecipient> deathRecipient =
            new DataObsCallbackRecipient(std::bind(&DataObsMgrInner::OnCallBackDied, this, std::placeholders::_1));
        dataObserver->AsObject()->AddDeathRecipient(deathRecipient);
        recipientMap_.emplace(dataObserver->AsObject(), deathRecipient);
    }
}

void DataObsMgrInner::RemoveObsDeathRecipient(const sptr<IDataAbilityObserver> &dataObserver)
{
    if ((dataObserver == nullptr) || dataObserver->AsObject() == nullptr) {
        return;
    }

    auto it = recipientMap_.find(dataObserver->AsObject());
    if (it != recipientMap_.end()) {
        it->first->RemoveDeathRecipient(it->second);
        recipientMap_.erase(it);
        return;
    }
}

void DataObsMgrInner::OnCallBackDied(const wptr<IRemoteObject> &remote)
{
    auto object = remote.promote();
    if (object == nullptr) {
        return;
    }

    if (handler_) {
        auto task = [object, dataObsMgrInner = shared_from_this()]() {
            dataObsMgrInner->HandleCallBackDiedTask(object);
        };
        handler_->PostTask(task);
    }
}

void DataObsMgrInner::HandleCallBackDiedTask(const sptr<IRemoteObject> &dataObserver)
{
    HILOG_INFO("%{public}s,called", __func__);
    std::lock_guard<std::mutex> lock_l(innerMutex_);

    if (dataObserver == nullptr) {
        return;
    }

    sptr<IDataAbilityObserver> object = iface_cast<IDataAbilityObserver>(dataObserver);

    RemoveObsFromMap(object);
}

void DataObsMgrInner::RemoveObsFromMap(const sptr<IDataAbilityObserver> &dataObserver)
{
    for (auto &obsCallback : obsmap_) {
        auto &obsList = obsCallback.second;
        auto obs = std::find(obsList.begin(), obsList.end(), dataObserver);
        if (obs != obsList.end()) {
            obsList.remove(dataObserver);
            if (obsList.empty()) {
                HILOG_INFO("%{public}s: remove obsList from map ", __func__);
                obsmap_.erase(obsCallback.first);
            }
        }
    }
    RemoveObsDeathRecipient(dataObserver);
}

bool DataObsMgrInner::ObsExistInMap(const sptr<IDataAbilityObserver> &dataObserver)
{
    for (auto &obsCallback : obsmap_) {
        auto &obsList = obsCallback.second;
        auto obs = std::find(obsList.begin(), obsList.end(), dataObserver);
        if (obs != obsList.end()) {
            return true;
        }
    }
    return false;
}

}  // namespace AAFwk
}  // namespace OHOS
