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

#ifndef OHOS_AAFWK_KERNAL_ABILITY_MANAGER_H
#define OHOS_AAFWK_KERNAL_ABILITY_MANAGER_H

#include <mutex>
#include <queue>

#include "ability_record.h"
#include "ability_running_info.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class KernalAbilityManager
 * KernalAbilityManager provides a facility for managing systerm ability life cycle.
 */
class KernalAbilityManager : public std::enable_shared_from_this<KernalAbilityManager> {
public:
    explicit KernalAbilityManager(int userId);
    ~KernalAbilityManager();

    /**
     * init kernal ability manager.
     *
     */
    void Init();
    /**
     * StartAbility with request.
     *
     * @param abilityRequest, the request of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartAbility(const AbilityRequest &abilityRequest);
    /**
     * attach ability thread ipc object.
     *
     * @param scheduler, ability thread ipc object.
     * @param token, the token of ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token);
    /**
     * AbilityTransitionDone, ability call this interface after lift cycle was changed.
     *
     * @param token,.ability's token.
     * @param state,.the state of ability lift cycle.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state);
    /**
     * OnAbilityRequestDone, app manager service call this interface after ability request done.
     *
     * @param token,ability's token.
     * @param state,the state of ability lift cycle.
     */
    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state);

    void OnAppStateChanged(const AppInfo &info);

    /**
     * get manager's user id.
     */
    int GetManagerUserId() const;

    void DumpState(std::vector<std::string> &info);

    void DumpSysState(std::vector<std::string> &info, bool isClient);

    void OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord);

    void OnTimeOut(uint32_t msgId, int64_t eventId);

    /**
     * get the ability record by token.
     *
     * @return abilityRecord, target ability.
     */
    std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(const sptr<IRemoteObject> &token);

    void RestartAbility(const std::shared_ptr<AbilityRecord> abilityRecord);

    void GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info);

private:
    /**
     * StartAbilityLocked.
     *
     * @param abilityRequest the request of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartAbilityLocked(const AbilityRequest &abilityRequest);
    /**
     * push waitting ability to queue.
     *
     * @param abilityRequest, the request of ability.
     */
    void EnqueueWaittingAbility(const AbilityRequest &abilityRequest);
    /**
     * pop waitting ability.
     *
     */
    void DequeueWaittingAbility();
    /**
     * get current top ability of stack.
     *
     * @return top ability record.
     */
    std::shared_ptr<AbilityRecord> GetCurrentTopAbility() const;
    /**
     * get or create the target ability record of system app.
     *
     * @param abilityRequest, the request of ability.
     * @param targetAbility, target ability record.
     */
    void GetOrCreateAbilityRecord(const AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &targetAbility);
    /**
     * get the flag of the target ability record.
     *
     * @param bundleName, target bundleName.
     * @param abilityName, target ability name.
     */
    static std::string GetFlagOfAbility(const std::string &bundleName, const std::string &abilityName);
    /**
     * get the ability record by eventId.
     *
     * @return abilityRecord, target ability.
     */
    std::shared_ptr<AbilityRecord> GetAbilityRecordByEventId(const int64_t eventId) const;
    /**
     * dispatch ability life cycle .
     *
     * @param abilityRecord.
     * @param state.
     */
    int DispatchForeground(const std::shared_ptr<AbilityRecord> &abilityRecord, int state);
    /**
     * complete ability life cycle .
     *
     * @param abilityRecord.
     */
    void CompleteForeground(const std::shared_ptr<AbilityRecord> &abilityRecord);

    bool RemoveAbilityRecord(std::shared_ptr<AbilityRecord> ability);

private:
    std::recursive_mutex stackLock_;
    std::list<std::shared_ptr<AbilityRecord>> abilities_;
    std::queue<AbilityRequest> waittingAbilityQueue_;
    int userId_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_KERNAL_ABILITY_MANAGER_H
