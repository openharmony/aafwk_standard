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

#ifndef OHOS_AAFWK_DATA_ABILITY_RECORD_H
#define OHOS_AAFWK_DATA_ABILITY_RECORD_H

#include <list>
#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "ability_record.h"
#include "data_ability_caller_recipient.h"

namespace OHOS {
namespace AAFwk {
class DataAbilityRecord : public std::enable_shared_from_this<DataAbilityRecord> {
public:
    explicit DataAbilityRecord(const AbilityRequest &req);
    virtual ~DataAbilityRecord();

public:
    int StartLoading();
    int WaitForLoaded(std::mutex &mutex, const std::chrono::system_clock::duration &timeout);
    sptr<IAbilityScheduler> GetScheduler();
    int Attach(const sptr<IAbilityScheduler> &scheduler);
    int OnTransitionDone(int state);
    int AddClient(const sptr<IRemoteObject> &client, bool tryBind, bool isSystem);
    int RemoveClient(const sptr<IRemoteObject> &client, bool isSystem);
    int RemoveClients(const std::shared_ptr<AbilityRecord> &client = nullptr);
    size_t GetClientCount(const sptr<IRemoteObject> &client = nullptr) const;
    int KillBoundClientProcesses();
    const AbilityRequest &GetRequest() const;
    std::shared_ptr<AbilityRecord> GetAbilityRecord();
    sptr<IRemoteObject> GetToken();
    void Dump() const;
    void Dump(std::vector<std::string> &info) const;

private:
    using IRemoteObjectPtr = sptr<IRemoteObject>;
    using AbilityRecordPtr = std::shared_ptr<AbilityRecord>;

    struct ClientInfo {
        IRemoteObjectPtr client;
        bool tryBind;
        bool isSystem;
    };
    void OnSchedulerDied(const wptr<IRemoteObject> &remote);

private:
    std::condition_variable_any loadedCond_ {};
    AbilityRequest request_ {};
    AbilityRecordPtr ability_ {};
    sptr<IAbilityScheduler> scheduler_ {};
    std::list<ClientInfo> clients_ {};
    sptr<IRemoteObject::DeathRecipient> callerDeathRecipient_;  // caller binderDied Recipient
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_DATA_ABILITY_RECORD_H
