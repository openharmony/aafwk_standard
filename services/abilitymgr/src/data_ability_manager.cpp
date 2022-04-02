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

#include "data_ability_manager.h"

#include <chrono>
#include <thread>

#include "ability_manager_service.h"
#include "ability_util.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
using namespace std::chrono;
using namespace std::placeholders;

namespace {
constexpr bool DEBUG_ENABLED = false;
constexpr system_clock::duration DATA_ABILITY_LOAD_TIMEOUT = 11000ms;
}  // namespace

DataAbilityManager::DataAbilityManager()
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);
}

DataAbilityManager::~DataAbilityManager()
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);
}

sptr<IAbilityScheduler> DataAbilityManager::Acquire(
    const AbilityRequest &abilityRequest, bool tryBind, const sptr<IRemoteObject> &client, bool isSystem)
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);

    if (abilityRequest.abilityInfo.type != AppExecFwk::AbilityType::DATA) {
        HILOG_ERROR("Data ability manager acquire: not a data ability.");
        return nullptr;
    }

    if (abilityRequest.abilityInfo.bundleName.empty() || abilityRequest.abilityInfo.name.empty()) {
        HILOG_ERROR("Data ability manager acquire: invalid name.");
        return nullptr;
    }

    std::shared_ptr<AbilityRecord> clientAbilityRecord;
    const std::string dataAbilityName(abilityRequest.abilityInfo.bundleName + '.' + abilityRequest.abilityInfo.name);

    if (client && !isSystem) {
        clientAbilityRecord = Token::GetAbilityRecordByToken(client);
        if (!clientAbilityRecord) {
            HILOG_ERROR("Data ability manager acquire: invalid client token.");
            return nullptr;
        }
        HILOG_INFO("Ability '%{public}s' acquiring data ability '%{public}s'...",
            clientAbilityRecord->GetAbilityInfo().name.c_str(), dataAbilityName.c_str());
    } else {
        HILOG_INFO("Loading data ability '%{public}s'...", dataAbilityName.c_str());
    }

    std::lock_guard<std::mutex> locker(mutex_);

    if (DEBUG_ENABLED) {
        DumpLocked(__func__, __LINE__);
    }

    DataAbilityRecordPtr dataAbilityRecord;

    auto it = dataAbilityRecordsLoaded_.find(dataAbilityName);
    if (it == dataAbilityRecordsLoaded_.end()) {
        HILOG_DEBUG("Acquiring data ability is not existed, loading...");
        dataAbilityRecord = LoadLocked(dataAbilityName, abilityRequest);
    } else {
        HILOG_DEBUG("Acquiring data ability is existed .");
        dataAbilityRecord = it->second;
    }

    if (!dataAbilityRecord) {
        HILOG_ERROR("Failed to load data ability '%{public}s'.", dataAbilityName.c_str());
        return nullptr;
    }

    auto scheduler = dataAbilityRecord->GetScheduler();
    if (!scheduler) {
        if (DEBUG_ENABLED) {
            HILOG_ERROR("BUG: data ability '%{public}s' is not loaded, removing it...", dataAbilityName.c_str());
        }
        auto it = dataAbilityRecordsLoaded_.find(dataAbilityName);
        if (it != dataAbilityRecordsLoaded_.end()) {
            dataAbilityRecordsLoaded_.erase(it);
        }
        return nullptr;
    }

    if (client) {
        dataAbilityRecord->AddClient(client, tryBind, isSystem);
    }

    if (DEBUG_ENABLED) {
        DumpLocked(__func__, __LINE__);
    }

    return scheduler;
}

int DataAbilityManager::Release(
    const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &client, bool isSystem)
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);

    CHECK_POINTER_AND_RETURN(scheduler, ERR_NULL_OBJECT);
    CHECK_POINTER_AND_RETURN(client, ERR_NULL_OBJECT);

    std::lock_guard<std::mutex> locker(mutex_);

    if (DEBUG_ENABLED) {
        DumpLocked(__func__, __LINE__);
    }

    DataAbilityRecordPtr dataAbilityRecord;

    for (auto it = dataAbilityRecordsLoaded_.begin(); it != dataAbilityRecordsLoaded_.end(); ++it) {
        if (it->second && it->second->GetScheduler() &&
            it->second->GetScheduler()->AsObject() == scheduler->AsObject()) {
            dataAbilityRecord = it->second;
            HILOG_INFO("Releasing data ability '%{public}s'...", it->first.c_str());
            break;
        }
    }

    if (!dataAbilityRecord) {
        HILOG_ERROR("Releasing not existed data ability.");
        return ERR_UNKNOWN_OBJECT;
    }

    auto abilityRecord = dataAbilityRecord->GetAbilityRecord();
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_UNKNOWN_OBJECT);
    int result = AbilityUtil::JudgeAbilityVisibleControl(abilityRecord->GetAbilityInfo());
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return result;
    }

    if (dataAbilityRecord->GetClientCount(client) == 0) {
        HILOG_ERROR("Release data ability with wrong client.");
        return ERR_UNKNOWN_OBJECT;
    }

    dataAbilityRecord->RemoveClient(client, isSystem);

    if (DEBUG_ENABLED) {
        DumpLocked(__func__, __LINE__);
    }

    return ERR_OK;
}

bool DataAbilityManager::ContainsDataAbility(const sptr<IAbilityScheduler> &scheduler)
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);

    CHECK_POINTER_AND_RETURN(scheduler, ERR_NULL_OBJECT);

    std::lock_guard<std::mutex> locker(mutex_);
    for (auto it = dataAbilityRecordsLoaded_.begin(); it != dataAbilityRecordsLoaded_.end(); ++it) {
        if (it->second && it->second->GetScheduler() &&
            it->second->GetScheduler()->AsObject() == scheduler->AsObject()) {
            return true;
        }
    }

    return false;
}

int DataAbilityManager::AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);

    CHECK_POINTER_AND_RETURN(scheduler, ERR_NULL_OBJECT);
    CHECK_POINTER_AND_RETURN(token, ERR_NULL_OBJECT);

    std::lock_guard<std::mutex> locker(mutex_);

    if (DEBUG_ENABLED) {
        DumpLocked(__func__, __LINE__);
    }

    HILOG_INFO("Attaching data ability...");

    auto record = Token::GetAbilityRecordByToken(token);
    std::string abilityName = "";
    if (record != nullptr) {
        abilityName = record->GetAbilityInfo().name;
    }

    DataAbilityRecordPtr dataAbilityRecord;
    auto it = dataAbilityRecordsLoading_.begin();
    for (; it != dataAbilityRecordsLoading_.end(); ++it) {
        if (it->second && it->second->GetToken() == token) {
            dataAbilityRecord = it->second;
            break;
        }
    }

    if (!dataAbilityRecord) {
        HILOG_ERROR("Attaching data ability '%{public}s' is not in loading state.", abilityName.c_str());
        return ERR_UNKNOWN_OBJECT;
    }

    if (DEBUG_ENABLED && dataAbilityRecord->GetClientCount() > 0) {
        HILOG_ERROR("BUG: Attaching data ability '%{public}s' has clients.", abilityName.c_str());
    }

    if (DEBUG_ENABLED && dataAbilityRecord->GetScheduler()) {
        HILOG_ERROR("BUG: Attaching data ability '%{public}s' has ready.", abilityName.c_str());
    }

    if (DEBUG_ENABLED && dataAbilityRecordsLoaded_.count(it->first) != 0) {
        HILOG_ERROR("BUG: The attaching data ability '%{public}s' has already existed.", abilityName.c_str());
    }

    return dataAbilityRecord->Attach(scheduler);
}

int DataAbilityManager::AbilityTransitionDone(const sptr<IRemoteObject> &token, int state)
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);

    CHECK_POINTER_AND_RETURN(token, ERR_NULL_OBJECT);

    std::lock_guard<std::mutex> locker(mutex_);

    if (DEBUG_ENABLED) {
        DumpLocked(__func__, __LINE__);
    }

    HILOG_INFO("Handling data ability transition done %{public}d...", state);

    DataAbilityRecordPtrMap::iterator it;
    DataAbilityRecordPtr dataAbilityRecord;
    auto record = Token::GetAbilityRecordByToken(token);
    std::string abilityName = "";
    if (record != nullptr) {
        abilityName = record->GetAbilityInfo().name;
    }
    for (it = dataAbilityRecordsLoading_.begin(); it != dataAbilityRecordsLoading_.end(); ++it) {
        if (it->second && it->second->GetToken() == token) {
            dataAbilityRecord = it->second;
            break;
        }
    }
    if (!dataAbilityRecord) {
        HILOG_ERROR("Attaching data ability '%{public}s' is not existed.", abilityName.c_str());
        return ERR_UNKNOWN_OBJECT;
    }

    int ret = dataAbilityRecord->OnTransitionDone(state);
    if (ret == ERR_OK) {
        dataAbilityRecordsLoaded_[it->first] = dataAbilityRecord;
        dataAbilityRecordsLoading_.erase(it);
    }

    return ret;
}

void DataAbilityManager::OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state)
{
    /* Do nothing now. */
}

void DataAbilityManager::OnAbilityDied(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);
    CHECK_POINTER(abilityRecord);

    {
        std::lock_guard<std::mutex> locker(mutex_);
        if (DEBUG_ENABLED) {
            DumpLocked(__func__, __LINE__);
        }
        if (abilityRecord->GetAbilityInfo().type == AppExecFwk::AbilityType::DATA) {
            // If 'abilityRecord' is a data ability server, trying to remove it from 'dataAbilityRecords_'.
            for (auto it = dataAbilityRecordsLoaded_.begin(); it != dataAbilityRecordsLoaded_.end();) {
                if (it->second && it->second->GetAbilityRecord() == abilityRecord) {
                    it->second->KillBoundClientProcesses();
                    HILOG_DEBUG("Removing died data ability record...");
                    it = dataAbilityRecordsLoaded_.erase(it);
                    break;
                } else {
                    ++it;
                }
            }
        }
        if (DEBUG_ENABLED) {
            DumpLocked(__func__, __LINE__);
        }
        // If 'abilityRecord' is a data ability client, tring to remove it from all servers.
        for (auto it = dataAbilityRecordsLoaded_.begin(); it != dataAbilityRecordsLoaded_.end(); ++it) {
            if (it->second) {
                it->second->RemoveClients(abilityRecord);
            }
        }
        if (DEBUG_ENABLED) {
            DumpLocked(__func__, __LINE__);
        }
    }

    RestartDataAbility(abilityRecord);
}

void DataAbilityManager::OnAppStateChanged(const AppInfo &info)
{
    std::lock_guard<std::mutex> locker(mutex_);

    for (auto it = dataAbilityRecordsLoaded_.begin(); it != dataAbilityRecordsLoaded_.end(); ++it) {
        if (!it->second) {
            continue;
        }
        auto abilityRecord = it->second->GetAbilityRecord();
        if (abilityRecord && (info.processName == abilityRecord->GetAbilityInfo().process ||
                                 info.processName == abilityRecord->GetApplicationInfo().bundleName)) {
            auto appName = abilityRecord->GetApplicationInfo().name;
            auto uid = abilityRecord->GetAbilityInfo().applicationInfo.uid;
            auto isExist = [&appName, &uid](
                               const AppData &appData) { return appData.appName == appName && appData.uid == uid; };
            auto iter = std::find_if(info.appData.begin(), info.appData.end(), isExist);
            if (iter != info.appData.end()) {
                abilityRecord->SetAppState(info.state);
            }
        }
    }

    for (auto it = dataAbilityRecordsLoading_.begin(); it != dataAbilityRecordsLoading_.end(); ++it) {
        if (!it->second) {
            continue;
        }
        auto abilityRecord = it->second->GetAbilityRecord();
        if (abilityRecord && (info.processName == abilityRecord->GetAbilityInfo().process ||
                                 info.processName == abilityRecord->GetApplicationInfo().bundleName)) {
            auto appName = abilityRecord->GetApplicationInfo().name;
            auto uid = abilityRecord->GetAbilityInfo().applicationInfo.uid;
            auto isExist = [&appName, &uid](
                               const AppData &appData) { return appData.appName == appName && appData.uid == uid; };
            auto iter = std::find_if(info.appData.begin(), info.appData.end(), isExist);
            if (iter != info.appData.end()) {
                abilityRecord->SetAppState(info.state);
            }
        }
    }
}

std::shared_ptr<AbilityRecord> DataAbilityManager::GetAbilityRecordById(int64_t id)
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);

    std::lock_guard<std::mutex> locker(mutex_);

    for (auto it = dataAbilityRecordsLoaded_.begin(); it != dataAbilityRecordsLoaded_.end(); ++it) {
        if (!it->second) {
            continue;
        }
        auto abilityRecord = it->second->GetAbilityRecord();
        if (abilityRecord->GetRecordId() == id) {
            return abilityRecord;
        }
    }

    return nullptr;
}

std::shared_ptr<AbilityRecord> DataAbilityManager::GetAbilityRecordByToken(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);

    CHECK_POINTER_AND_RETURN(token, nullptr);

    std::lock_guard<std::mutex> locker(mutex_);
    for (auto it = dataAbilityRecordsLoaded_.begin(); it != dataAbilityRecordsLoaded_.end(); ++it) {
        if (!it->second) {
            continue;
        }
        auto abilityRecord = it->second->GetAbilityRecord();
        if (abilityRecord == Token::GetAbilityRecordByToken(token)) {
            return abilityRecord;
        }
    }
    for (auto it = dataAbilityRecordsLoading_.begin(); it != dataAbilityRecordsLoading_.end(); ++it) {
        if (!it->second) {
            continue;
        }
        auto abilityRecord = it->second->GetAbilityRecord();
        if (abilityRecord == Token::GetAbilityRecordByToken(token)) {
            return abilityRecord;
        }
    }
    return nullptr;
}

std::shared_ptr<AbilityRecord> DataAbilityManager::GetAbilityRecordByScheduler(const sptr<IAbilityScheduler> &scheduler)
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);

    CHECK_POINTER_AND_RETURN(scheduler, nullptr);

    std::lock_guard<std::mutex> locker(mutex_);

    for (auto it = dataAbilityRecordsLoaded_.begin(); it != dataAbilityRecordsLoaded_.end(); ++it) {
        if (it->second && it->second->GetScheduler() &&
            it->second->GetScheduler()->AsObject() == scheduler->AsObject()) {
            return it->second->GetAbilityRecord();
        }
    }

    return nullptr;
}

void DataAbilityManager::Dump(const char *func, int line)
{
    HILOG_DEBUG("%{public}s(%{public}d)", __PRETTY_FUNCTION__, __LINE__);

    std::lock_guard<std::mutex> locker(mutex_);

    DumpLocked(func, line);
}

DataAbilityManager::DataAbilityRecordPtr DataAbilityManager::LoadLocked(
    const std::string &name, const AbilityRequest &req)
{
    HILOG_DEBUG("%{public}s(%{public}d) name '%{public}s'", __PRETTY_FUNCTION__, __LINE__, name.c_str());

    DataAbilityRecordPtr dataAbilityRecord;

    auto it = dataAbilityRecordsLoading_.find(name);
    if (it == dataAbilityRecordsLoading_.end()) {
        HILOG_INFO("Acquiring data ability is not in loading, trying to load it...");

        dataAbilityRecord = std::make_shared<DataAbilityRecord>(req);
        if (!dataAbilityRecord) {
            HILOG_ERROR("Failed to allocate data ability record.");
            return nullptr;
        }

        // Start data ability loading process asynchronously.
        int startResult = dataAbilityRecord->StartLoading();
        if (startResult != ERR_OK) {
            HILOG_ERROR("Failed to load data ability %{public}d", startResult);
            return nullptr;
        }

        auto insertResult = dataAbilityRecordsLoading_.insert({name, dataAbilityRecord});
        if (!insertResult.second) {
            HILOG_ERROR("Failed to insert data ability to loading map.");
            return nullptr;
        }
    } else {
        HILOG_INFO("Acquired data ability is loading...");
        dataAbilityRecord = it->second;
    }

    if (!dataAbilityRecord) {
        HILOG_ERROR("Failed to load data ability '%{public}s'.", name.c_str());
        return nullptr;
    }

    HILOG_INFO("Waiting for data ability loaded...");

    // Waiting for data ability loaded.
    int ret = dataAbilityRecord->WaitForLoaded(mutex_, DATA_ABILITY_LOAD_TIMEOUT);
    if (ret != ERR_OK) {
        HILOG_ERROR("Wait for data ability failed %{public}d.", ret);
        it = dataAbilityRecordsLoading_.find(name);
        if (it != dataAbilityRecordsLoading_.end()) {
            dataAbilityRecordsLoading_.erase(it);
        }
        return nullptr;
    }

    return dataAbilityRecord;
}

void DataAbilityManager::DumpLocked(const char *func, int line)
{
    if (func && line >= 0) {
        HILOG_INFO("Data ability manager dump at %{public}s(%{public}d)", func, line);
    } else {
        HILOG_INFO("Data ability manager dump");
    }

    HILOG_INFO("Available data ability count: %{public}zu", dataAbilityRecordsLoaded_.size());

    for (auto it = dataAbilityRecordsLoaded_.begin(); it != dataAbilityRecordsLoaded_.end(); ++it) {
        HILOG_INFO("'%{public}s':", it->first.c_str());
        if (it->second) {
            it->second->Dump();
        }
    }

    HILOG_INFO("Loading data ability count: %{public}zu", dataAbilityRecordsLoading_.size());

    for (auto it = dataAbilityRecordsLoading_.begin(); it != dataAbilityRecordsLoading_.end(); ++it) {
        HILOG_INFO("'%{public}s':", it->first.c_str());
        if (it->second) {
            it->second->Dump();
        }
    }
}

void DataAbilityManager::DumpState(std::vector<std::string> &info, const std::string &args) const
{
    if (!args.empty()) {
        auto it = std::find_if(dataAbilityRecordsLoaded_.begin(),
            dataAbilityRecordsLoaded_.end(),
            [&args](const auto &dataAbilityRecord) { return dataAbilityRecord.first.compare(args) == 0; });
        if (it != dataAbilityRecordsLoaded_.end()) {
            info.emplace_back("AbilityName [ " + it->first + " ]");
            if (it->second) {
                it->second->Dump(info);
            }
        } else {
            info.emplace_back(args + ": Nothing to dump.");
        }
    } else {
        info.emplace_back("dataAbilityRecords:");
        for (auto &&dataAbilityRecord : dataAbilityRecordsLoaded_) {
            info.emplace_back("  uri [" + dataAbilityRecord.first + "]");
            if (dataAbilityRecord.second) {
                dataAbilityRecord.second->Dump(info);
            }
        }
    }
}

void DataAbilityManager::DumpSysState(std::vector<std::string> &info, bool isClient, const std::string &args) const
{
    if (!args.empty()) {
        auto it = std::find_if(dataAbilityRecordsLoaded_.begin(),
            dataAbilityRecordsLoaded_.end(),
            [&args](const auto &dataAbilityRecord) { return dataAbilityRecord.first.compare(args) == 0; });
        if (it != dataAbilityRecordsLoaded_.end()) {
            info.emplace_back("AbilityName [ " + it->first + " ]");
            if (it->second) {
                it->second->Dump(info);
            }
            // add dump client info
            if (isClient && it->second && it->second->GetScheduler() &&
                it->second->GetAbilityRecord() && it->second->GetAbilityRecord()->IsReady()) {
                std::vector<std::string> params;
                it->second->GetScheduler()->DumpAbilityInfo(params, info);
                AppExecFwk::Configuration config;
                if (DelayedSingleton<AppScheduler>::GetInstance()->GetConfiguration(config) == ERR_OK) {
                    info.emplace_back("          configuration: " + config.GetName());
                }
            }
        } else {
            info.emplace_back(args + ": Nothing to dump.");
        }
    } else {
        info.emplace_back("  dataAbilityRecords:");
        for (auto &&dataAbilityRecord : dataAbilityRecordsLoaded_) {
            info.emplace_back("    uri [" + dataAbilityRecord.first + "]");
            if (dataAbilityRecord.second) {
                dataAbilityRecord.second->Dump(info);
                dataAbilityRecord.second->GetScheduler();
            }
            // add dump client info
            if (isClient && dataAbilityRecord.second && dataAbilityRecord.second->GetScheduler() &&
                dataAbilityRecord.second->GetAbilityRecord() &&
                dataAbilityRecord.second->GetAbilityRecord()->IsReady()) {
                std::vector<std::string> params;
                dataAbilityRecord.second->GetScheduler()->DumpAbilityInfo(params, info);
                AppExecFwk::Configuration config;
                if (DelayedSingleton<AppScheduler>::GetInstance()->GetConfiguration(config) == ERR_OK) {
                    info.emplace_back("          configuration: " + config.GetName());
                }
            }
        }
    }
    return;
}

void DataAbilityManager::GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm)
{
    HILOG_INFO("Get ability running infos");
    std::lock_guard<std::mutex> locker(mutex_);

    auto queryInfo = [&info, isPerm](DataAbilityRecordPtrMap::reference data) {
        auto dataAbilityRecord = data.second;
        if (!dataAbilityRecord) {
            return;
        }

        auto abilityRecord = dataAbilityRecord->GetAbilityRecord();
        if (!abilityRecord) {
            return;
        }

        if (isPerm) {
            DelayedSingleton<AbilityManagerService>::GetInstance()->GetAbilityRunningInfo(info, abilityRecord);
        } else {
            auto callingTokenId = IPCSkeleton::GetCallingTokenID();
            auto tokenID = abilityRecord->GetApplicationInfo().accessTokenId;
            if (callingTokenId == tokenID) {
                DelayedSingleton<AbilityManagerService>::GetInstance()->GetAbilityRunningInfo(info, abilityRecord);
            }
        }
    };

    std::for_each(dataAbilityRecordsLoading_.begin(), dataAbilityRecordsLoading_.end(), queryInfo);
    std::for_each(dataAbilityRecordsLoaded_.begin(), dataAbilityRecordsLoaded_.end(), queryInfo);
}

void DataAbilityManager::RestartDataAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    // restart data ability if necessary
    auto bms = AbilityUtil::GetBundleManager();
    CHECK_POINTER(bms);
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    bool getBundleInfos = bms->GetBundleInfos(OHOS::AppExecFwk::GET_BUNDLE_DEFAULT, bundleInfos, USER_ID_NO_HEAD);
    if (!getBundleInfos) {
        HILOG_ERROR("Handle ability died task, get bundle infos failed");
        return;
    }

    for (size_t i = 0; i < bundleInfos.size(); i++) {
        if (!bundleInfos[i].isKeepAlive) {
            continue;
        }
        for (auto hapModuleInfo : bundleInfos[i].hapModuleInfos) {
            if (hapModuleInfo.isModuleJson) {
                // new application model, it cannot be a data ability
                continue;
            }
            // old application model, it maybe a data ability
            std::string mainElement = hapModuleInfo.mainAbility;
            if (abilityRecord->GetAbilityInfo().name != mainElement) {
                continue;
            }
            std::string uriStr;
            bool getDataAbilityUri = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance()->GetDataAbilityUri(
                hapModuleInfo.abilityInfos, mainElement, uriStr);
            if (getDataAbilityUri) {
                HILOG_INFO("restart data ability: %{public}s, uri: %{public}s",
                    abilityRecord->GetAbilityInfo().name.c_str(), uriStr.c_str());
                Uri uri(uriStr);
                OHOS::DelayedSingleton<AbilityManagerService>::GetInstance()->AcquireDataAbility(uri, true, nullptr);
                return;
            }
        }
    }
}
}  // namespace AAFwk
}  // namespace OHOS
