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

#include "data_ability_record.h"

#include <algorithm>
#include "hilog_wrapper.h"
#include "ability_util.h"
#include "app_scheduler.h"

namespace OHOS {
namespace AAFwk {
DataAbilityRecord::DataAbilityRecord(const AbilityRequest &req) : request_(req)
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);

    if (request_.abilityInfo.type != AppExecFwk::AbilityType::DATA) {
        HILOG_ERROR("BUG: Construct a data ability with wrong ability type.");
    }
}

DataAbilityRecord::~DataAbilityRecord()
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);
}

int DataAbilityRecord::StartLoading()
{
    HILOG_INFO("Start data ability loading...");

    if (ability_ || scheduler_) {
        HILOG_ERROR("Data ability already started.");
        return ERR_ALREADY_EXISTS;
    }

    if (request_.abilityInfo.type != AppExecFwk::AbilityType::DATA) {
        HILOG_ERROR("Start a data ability with wrong ability type.");
        return ERR_INVALID_VALUE;
    }

    auto ability = AbilityRecord::CreateAbilityRecord(request_);
    if (!ability) {
        HILOG_ERROR("Failed to allocate ability for DataAbilityRecord.");
        return ERR_NO_MEMORY;
    }

    int ret = ability->LoadAbility();
    if (ret != ERR_OK) {
        HILOG_ERROR("Failed to start data ability loading.");
        return ret;
    }

    ability_ = ability;

    // Ability state is 'INITIAL' now.

    return ERR_OK;
}

int DataAbilityRecord::WaitForLoaded(std::mutex &mutex, const std::chrono::system_clock::duration &timeout)
{
    CHECK_POINTER_AND_RETURN(ability_, ERR_INVALID_STATE);

    // Data ability uses 'ACTIVATE' as loaded state.
    if (ability_->GetAbilityState() == ACTIVE) {
        return ERR_OK;
    }

    auto ret = loadedCond_.wait_for(mutex, timeout, [this] { return ability_->GetAbilityState() == ACTIVE; });

    if (!ret) {
        return ERR_TIMED_OUT;
    }

    if (!scheduler_ || ability_->GetAbilityState() != ACTIVE) {
        return ERR_INVALID_STATE;
    }

    return ERR_OK;
}

sptr<IAbilityScheduler> DataAbilityRecord::GetScheduler()
{
    // Check if data ability is attached.
    CHECK_POINTER_AND_RETURN(ability_, nullptr);
    CHECK_POINTER_AND_RETURN(scheduler_, nullptr);

    // Check if data ability is loaded.
    if (ability_->GetAbilityState() != ACTIVE) {
        return nullptr;
    }

    return scheduler_;
}

int DataAbilityRecord::Attach(const sptr<IAbilityScheduler> &scheduler)
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);

    if (!scheduler) {
        HILOG_ERROR("Attach data ability: invalid scheduler.");
        return ERR_INVALID_DATA;
    }

    if (!ability_) {
        HILOG_ERROR("Data ability attach: not startloading.");
        return ERR_INVALID_STATE;
    }

    if (scheduler_) {
        HILOG_ERROR("Attach data ability: already attached.");
        return ERR_INVALID_STATE;
    }

    // INITIAL => ACTIVATING

    if (ability_->GetAbilityState() != INITIAL) {
        HILOG_ERROR("Attaching data ability: not in 'INITIAL' state.");
        return ERR_INVALID_STATE;
    }

    HILOG_DEBUG("Attaching data ability...");
    ability_->SetScheduler(scheduler);
    scheduler_ = scheduler;

    HILOG_INFO("Scheduling 'OnStart' for data ability '%{public}s|%{public}s|%{public}d'...",
        ability_->GetApplicationInfo().bundleName.c_str(),
        ability_->GetAbilityInfo().name.c_str(),
        ability_->GetAbilityInfo().applicationInfo.uid);

    ability_->SetAbilityState(ACTIVATING);

    LifeCycleStateInfo state;
    state.state = AbilityLifeCycleState::ABILITY_STATE_ACTIVE;

    scheduler->ScheduleAbilityTransaction(ability_->GetWant(), state);

    return ERR_OK;
}

int DataAbilityRecord::OnTransitionDone(int state)
{
    CHECK_POINTER_AND_RETURN(ability_, ERR_INVALID_STATE);
    CHECK_POINTER_AND_RETURN(scheduler_, ERR_INVALID_STATE);

    if (ability_->GetAbilityState() != ACTIVATING) {
        HILOG_ERROR("Data ability on transition done: not in 'ACTIVATING' state.");
        return ERR_INVALID_STATE;
    }

    if (state != AbilityLifeCycleState::ABILITY_STATE_ACTIVE) {
        HILOG_ERROR("Data ability on transition done: not ACTIVE.");
        ability_->SetAbilityState(INITIAL);
        loadedCond_.notify_all();
        return ERR_INVALID_STATE;
    }

    // ACTIVATING => ACTIVE(loaded):
    // Set loaded state, data ability uses 'ACTIVE' as loaded state.

    ability_->SetAbilityState(ACTIVE);
    loadedCond_.notify_all();

    HILOG_INFO("Data ability '%{public}s|%{public}s|%{public}d' is loaded.",
        ability_->GetApplicationInfo().bundleName.c_str(),
        ability_->GetAbilityInfo().name.c_str(),
        ability_->GetAbilityInfo().applicationInfo.uid);

    return ERR_OK;
}

int DataAbilityRecord::AddClient(const sptr<IRemoteObject> &client, bool tryBind, bool isSystem)
{
    HILOG_INFO("Adding data ability client...");

    if (!client) {
        HILOG_ERROR("Data ability add client: invalid param.");
        return ERR_INVALID_STATE;
    }

    if (!ability_ || !scheduler_) {
        HILOG_ERROR("Data ability add client: not attached.");
        return ERR_INVALID_STATE;
    }

    if (ability_->GetAbilityState() != ACTIVE) {
        HILOG_ERROR("Data ability add client: not loaded.");
        return ERR_INVALID_STATE;
    }

    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (!appScheduler) {
        HILOG_ERROR("Data ability add client: failed to get app scheduler.");
        return ERR_NULL_OBJECT;
    }

    if (isSystem) {
        HILOG_ERROR("When the caller is system,add death monitoring");
        if (client != nullptr && callerDeathRecipient_ != nullptr) {
            client->RemoveDeathRecipient(callerDeathRecipient_);
        }
        if (callerDeathRecipient_ == nullptr) {
            callerDeathRecipient_ = new DataAbilityCallerRecipient(
                std::bind(&DataAbilityRecord::OnSchedulerDied, this, std::placeholders::_1));
        }
        if (client != nullptr) {
            client->AddDeathRecipient(callerDeathRecipient_);
        }
    }

    // One client can be added multi-times, so 'RemoveClient()' must be called in corresponding times.
    auto &clientInfo = clients_.emplace_back();
    clientInfo.client = client;
    clientInfo.tryBind = tryBind;
    clientInfo.isSystem = isSystem;
    if (!isSystem) {
        auto clientAbilityRecord = Token::GetAbilityRecordByToken(client);
        CHECK_POINTER_AND_RETURN(clientAbilityRecord, ERR_UNKNOWN_OBJECT);
        appScheduler->AbilityBehaviorAnalysis(ability_->GetToken(), clientAbilityRecord->GetToken(), 0, 0, 1);
        HILOG_INFO("Ability ability '%{public}s|%{public}s|%{public}d'.",
            clientAbilityRecord->GetApplicationInfo().bundleName.c_str(),
            clientAbilityRecord->GetAbilityInfo().name.c_str(),
            clientAbilityRecord->GetAbilityInfo().applicationInfo.uid);
    }

    if (clients_.size() == 1) {
        HILOG_INFO("Moving data ability app to foreground...");
        appScheduler->MoveToForground(ability_->GetToken());
    }

    HILOG_INFO("Data ability '%{public}s|%{public}s|%{public}d'.",
        ability_->GetApplicationInfo().bundleName.c_str(),
        ability_->GetAbilityInfo().name.c_str(),
        ability_->GetAbilityInfo().applicationInfo.uid);

    return ERR_OK;
}

int DataAbilityRecord::RemoveClient(const sptr<IRemoteObject> &client, bool isSystem)
{
    HILOG_INFO("Removing data ability client...");

    if (!client) {
        HILOG_ERROR("Data ability remove client: invalid client.");
        return ERR_INVALID_STATE;
    }

    if (!ability_ || !scheduler_) {
        HILOG_ERROR("Data ability remove clients: not attached.");
        return ERR_INVALID_STATE;
    }

    if (ability_->GetAbilityState() != ACTIVE) {
        HILOG_ERROR("Data ability remove client: not loaded.");
        return ERR_INVALID_STATE;
    }

    if (clients_.empty()) {
        HILOG_DEBUG("BUG: Data ability record has no clients.");
        return ERR_OK;
    }

    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (!appScheduler) {
        HILOG_ERROR("Data ability record remove client: invalid app scheduler.");
        return ERR_NULL_OBJECT;
    }

    for (auto it(clients_.begin()); it != clients_.end(); ++it) {
        if (it->client == client) {
            if (!isSystem) {
                auto clientAbilityRecord = Token::GetAbilityRecordByToken(client);
                CHECK_POINTER_AND_RETURN(clientAbilityRecord, ERR_UNKNOWN_OBJECT);
                appScheduler->AbilityBehaviorAnalysis(ability_->GetToken(), clientAbilityRecord->GetToken(), 0, 0, 0);
                HILOG_INFO("Ability ability '%{public}s|%{public}s|%{public}d'.",
                    clientAbilityRecord->GetApplicationInfo().bundleName.c_str(),
                    clientAbilityRecord->GetAbilityInfo().name.c_str(),
                    clientAbilityRecord->GetAbilityInfo().applicationInfo.uid);
            }
            clients_.erase(it);
            HILOG_INFO("Data ability '%{public}s|%{public}s|%{public}d'.",
                ability_->GetApplicationInfo().bundleName.c_str(),
                ability_->GetAbilityInfo().name.c_str(),
                ability_->GetAbilityInfo().applicationInfo.uid);
            break;
        }
    }

    if (clients_.empty()) {
        HILOG_INFO("Moving data ability to background...");
        appScheduler->MoveToBackground(ability_->GetToken());
    }

    return ERR_OK;
}

int DataAbilityRecord::RemoveClients(const std::shared_ptr<AbilityRecord> &client)
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);

    if (!ability_ || !scheduler_) {
        HILOG_ERROR("Data ability remove clients: not attached.");
        return ERR_INVALID_STATE;
    }

    if (ability_->GetAbilityState() != ACTIVE) {
        HILOG_ERROR("Data ability remove clients: not loaded.");
        return ERR_INVALID_STATE;
    }

    if (clients_.empty()) {
        HILOG_DEBUG("Data ability remove clients: no clients.");
        return ERR_OK;
    }

    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (!appScheduler) {
        HILOG_ERROR("Data ability remove clients: invalid app scheduler.");
        return ERR_NULL_OBJECT;
    }

    if (client) {
        HILOG_DEBUG("Removing data ability clients with filter...");
        auto it = clients_.begin();
        while (it != clients_.end()) {
            if (!it->isSystem) {
                auto clientAbilityRecord = Token::GetAbilityRecordByToken(it->client);
                if (!clientAbilityRecord) {
                    HILOG_ERROR("clientAbilityRecord is nullptr, continue.");
                    ++it;
                    continue;
                }
                if (clientAbilityRecord == client) {
                    appScheduler->AbilityBehaviorAnalysis(
                        ability_->GetToken(), clientAbilityRecord->GetToken(), 0, 0, 0);
                    it = clients_.erase(it);
                    HILOG_INFO("Ability '%{public}s|%{public}s|%{public}d' --X-> Data ability "
                        "'%{public}s|%{public}s|%{public}d'.",
                        client->GetApplicationInfo().bundleName.c_str(),
                        client->GetAbilityInfo().name.c_str(),
                        client->GetAbilityInfo().applicationInfo.uid,
                        ability_->GetApplicationInfo().bundleName.c_str(),
                        ability_->GetAbilityInfo().name.c_str(),
                        ability_->GetAbilityInfo().applicationInfo.uid);
                } else {
                    ++it;
                }
            } else {
                ++it;
            }
        }
    } else {
        HILOG_DEBUG("Removing data ability clients...");
        auto it = clients_.begin();
        while (it != clients_.end()) {
            if (!it->isSystem) {
                auto clientAbilityRecord = Token::GetAbilityRecordByToken(it->client);
                if (!clientAbilityRecord) {
                    HILOG_DEBUG("clientAbilityRecord is null,clear record");
                    it = clients_.erase(it);
                    continue;
                }
                appScheduler->AbilityBehaviorAnalysis(ability_->GetToken(), clientAbilityRecord->GetToken(), 0, 0, 0);
                it = clients_.erase(it);
                HILOG_INFO(
                    "Ability '%{public}s|%{public}s|%{public}d' --X-> Data ability '%{public}s|%{public}s|%{public}d'.",
                    client->GetApplicationInfo().bundleName.c_str(),
                    client->GetAbilityInfo().name.c_str(),
                    client->GetAbilityInfo().applicationInfo.uid,
                    ability_->GetApplicationInfo().bundleName.c_str(),
                    ability_->GetAbilityInfo().name.c_str(),
                    ability_->GetAbilityInfo().applicationInfo.uid);
            } else {
                ++it;
            }
        }
    }

    if (clients_.empty()) {
        HILOG_INFO("Moving data ability to background...");
        appScheduler->MoveToBackground(ability_->GetToken());
    }

    return ERR_OK;
}

size_t DataAbilityRecord::GetClientCount(const sptr<IRemoteObject> &client) const
{
    CHECK_POINTER_AND_RETURN(ability_, 0);
    CHECK_POINTER_AND_RETURN(scheduler_, 0);

    if (ability_->GetAbilityState() != ACTIVE) {
        HILOG_ERROR("Data ability get client count: not loaded.");
        return 0;
    }

    if (client) {
        return std::count_if(
            clients_.begin(), clients_.end(), [client](const ClientInfo &ci) { return ci.client == client; });
    }

    return clients_.size();
}

int DataAbilityRecord::KillBoundClientProcesses()
{
    CHECK_POINTER_AND_RETURN(ability_, ERR_INVALID_STATE);
    CHECK_POINTER_AND_RETURN(scheduler_, ERR_INVALID_STATE);

    if (ability_->GetAbilityState() != ACTIVE) {
        HILOG_ERROR("Data ability kill bound clients: not loaded.");
        return ERR_INVALID_STATE;
    }

    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (!appScheduler) {
        HILOG_ERROR("Data ability kill bound clients: invalid app scheduler.");
        return ERR_INVALID_STATE;
    }

    for (auto it = clients_.begin(); it != clients_.end(); ++it) {
        if (it->tryBind && false == it->isSystem) {
            auto clientAbilityRecord = Token::GetAbilityRecordByToken(it->client);
            CHECK_POINTER_CONTINUE(clientAbilityRecord);
            HILOG_INFO("Killing bound client '%{public}s|%{public}s|%{public}d' of data ability "
                "'%{public}s|%{public}s|%{public}d'...",
                clientAbilityRecord->GetApplicationInfo().bundleName.c_str(),
                clientAbilityRecord->GetAbilityInfo().name.c_str(),
                clientAbilityRecord->GetAbilityInfo().applicationInfo.uid,
                ability_->GetApplicationInfo().bundleName.c_str(),
                ability_->GetAbilityInfo().name.c_str(),
                ability_->GetAbilityInfo().applicationInfo.uid);
            appScheduler->KillProcessByAbilityToken(clientAbilityRecord->GetToken());
        }
    }

    return ERR_OK;
}

const AbilityRequest &DataAbilityRecord::GetRequest() const
{
    return request_;
}

std::shared_ptr<AbilityRecord> DataAbilityRecord::GetAbilityRecord()
{
    return ability_;
}

sptr<IRemoteObject> DataAbilityRecord::GetToken()
{
    if (!ability_) {
        return nullptr;
    }

    return ability_->GetToken();
}

void DataAbilityRecord::Dump() const
{
    CHECK_POINTER(ability_);

    HILOG_INFO("attached: %{public}s, clients: %{public}zu, refcnt: %{public}d, state: %{public}s",
        scheduler_ ? "true" : "false",
        clients_.size(),
        scheduler_ ? scheduler_->GetSptrRefCount() : 0,
        AbilityRecord::ConvertAbilityState(ability_->GetAbilityState()).c_str());

    int i = 0;

    for (auto it = clients_.begin(); it != clients_.end(); ++it) {
        if (false == it->isSystem) {
            auto clientAbilityRecord = Token::GetAbilityRecordByToken(it->client);
            CHECK_POINTER_CONTINUE(clientAbilityRecord);
            HILOG_INFO("  %{public}2d '%{public}s|%{public}s' - tryBind: %{public}s",
                i++,
                clientAbilityRecord->GetApplicationInfo().bundleName.c_str(),
                clientAbilityRecord->GetAbilityInfo().name.c_str(),
                it->tryBind ? "true" : "false");
        } else {
            HILOG_INFO("  %{public}2d '%{public}s' - tryBind: %{public}s",
                i++,
                "caller is system",
                it->tryBind ? "true" : "false");
        }
    }
}

void DataAbilityRecord::Dump(std::vector<std::string> &info) const
{
    CHECK_POINTER(ability_);
    info.emplace_back("    AbilityRecord ID #" + std::to_string(ability_->GetRecordId()) + "   state #" +
                      AbilityRecord::ConvertAbilityState(ability_->GetAbilityState()) + "   start time [" +
                      std::to_string(ability_->GetStartTime()) + "]");
    info.emplace_back("    main name [" + ability_->GetAbilityInfo().name + "]");
    info.emplace_back("    bundle name [" + ability_->GetAbilityInfo().bundleName + "]");
    info.emplace_back("    ability type [DATA]");
    info.emplace_back("    app state #" + AbilityRecord::ConvertAppState(ability_->GetAppState()));
    info.emplace_back("    Clients: " + std::to_string(clients_.size()));

    for (auto &&client : clients_) {
        if (false == client.isSystem) {
            auto clientAbilityRecord = Token::GetAbilityRecordByToken(client.client);
            CHECK_POINTER_CONTINUE(clientAbilityRecord);
            info.emplace_back("     > " + clientAbilityRecord->GetAbilityInfo().bundleName + "/" +
                              clientAbilityRecord->GetAbilityInfo().name + "  tryBind #" +
                              (client.tryBind ? "true" : "false") + "  isSystem  # " +
                              (client.isSystem ? "true" : "false"));
        } else {
            info.emplace_back(std::string("     > Caller is System /  tryBind # ") +
                              (client.tryBind ? "true" : "false") + "  isSystem  # " +
                              (client.isSystem ? "true" : "false"));
        }
    }
}

void DataAbilityRecord::OnSchedulerDied(const wptr<IRemoteObject> &remote)
{
    HILOG_INFO("'%{public}s':", __func__);
    auto object = remote.promote();

    if (clients_.empty()) {
        HILOG_DEBUG("BUG: Data ability record has no clients.");
        return;
    }

    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (!appScheduler) {
        HILOG_ERROR("Data ability remove clients: invalid app scheduler.");
        return;
    }

    if (object) {
        auto it = clients_.begin();
        while (it != clients_.end()) {
            if (it->client == object) {
                HILOG_DEBUG("remove system caller record with filter...");
                it = clients_.erase(it);
                HILOG_INFO("Data ability '%{public}s|%{public}s|%{public}d'.",
                    ability_->GetApplicationInfo().bundleName.c_str(),
                    ability_->GetAbilityInfo().name.c_str(),
                    ability_->GetAbilityInfo().applicationInfo.uid);
            } else {
                ++it;
            }
        }
    } else {
        auto it = clients_.begin();
        while (it != clients_.end()) {
            if (it->isSystem) {
                HILOG_DEBUG("remove system caller record...");
                it = clients_.erase(it);
                HILOG_INFO("Data ability '%{public}s|%{public}s|%{public}d'.",
                    ability_->GetApplicationInfo().bundleName.c_str(),
                    ability_->GetAbilityInfo().name.c_str(),
                    ability_->GetAbilityInfo().applicationInfo.uid);
            } else {
                ++it;
            }
        }
    }

    if (clients_.empty()) {
        HILOG_INFO("Moving data ability to background...");
        appScheduler->MoveToBackground(ability_->GetToken());
    }
}
}  // namespace AAFwk
}  // namespace OHOS
