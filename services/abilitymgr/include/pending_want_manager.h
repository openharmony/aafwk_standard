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

#ifndef OHOS_AAFWK_PENDING_WANT_MANAGER_H
#define OHOS_AAFWK_PENDING_WANT_MANAGER_H

#include <mutex>
#include <memory>
#include <map>
#include <vector>
#include <string>

#include "ability_manager_errors.h"
#include "ability_record.h"
#include "common_event.h"
#include "nocopyable.h"
#include "pending_want_key.h"
#include "pending_want_record.h"
#include "pending_want_common_event.h"
#include "sender_info.h"
#include "want_sender_info.h"

namespace OHOS {
namespace AAFwk {
enum class OperationType {
    /**
     * Unknown operation.
     */
    UNKNOWN_TYPE,

    /**
     * Starts an ability with a UI.
     */
    START_ABILITY,

    /**
     * Starts multiple abilities.
     */
    START_ABILITIES,

    /**
     * Starts an ability without a UI.
     */
    START_SERVICE,

    /**
     * Sends a common event.
     */
    SEND_COMMON_EVENT,

    /**
     * Starts a foreground ability without a UI.
     */
    START_FOREGROUND_SERVICE
};

enum class Flags {
    /**
     * Indicates that the {@link WantAgent} can be used only once.
     */
    ONE_TIME_FLAG = 1 << 30,

    /**
     * Indicates that {@code null} is returned if the {@link WantAgent} does not exist.
     */
    NO_BUILD_FLAG = 1 << 29,

    /**
     * Indicates that the existing {@link WantAgent} should be canceled before the new object is generated.
     */
    CANCEL_PRESENT_FLAG = 1 << 28,

    /**
     * Indicates that the system only replaces the extra data of the existing {@link WantAgent}
     * with that of the new object.
     */
    UPDATE_PRESENT_FLAG = 1 << 27,

    /**
     * Indicates that the created {@link WantAgent} should be immutable.
     */
    CONSTANT_FLAG = 1 << 26,

    /**
     * Indicates that the current value of {@code element} can be replaced
     * when the {@link WantAgent} is triggered.
     */
    REPLACE_ELEMENT,

    /**
     * Indicates that the current value of {@code action} can be replaced
     * when the {@link WantAgent} is triggered.
     */
    REPLACE_ACTION,

    /**
     * Indicates that the current value of {@code uri} can be replaced when the {@link WantAgent} is triggered.
     */
    REPLACE_URI,

    /**
     * Indicates that the current value of {@code entities} can be replaced
     * when the {@link WantAgent} is triggered.
     */
    REPLACE_ENTITIES,

    /**
     * Indicates that the current value of {@code bundleName} can be replaced
     * when the {@link WantAgent} is triggered.
     */
    REPLACE_BUNDLE
};

constexpr int32_t SYSTEM_UID = 1000;

class PendingWantManager : public std::enable_shared_from_this<PendingWantManager>, public NoCopyable {
public:
    PendingWantManager();
    explicit PendingWantManager(const std::shared_ptr<PendingWantManager> &manager) {};
    virtual ~PendingWantManager();

public:
    sptr<IWantSender> GetWantSender(const int32_t callingUid, const int32_t uid, const bool isSystemApp,
        const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken);
    int32_t SendWantSender(const sptr<IWantSender> &target, const SenderInfo &senderInfo);
    void CancelWantSender(
        const int32_t callingUid, const int32_t uid, const bool isSystemApp, const sptr<IWantSender> &sender);

    int32_t GetPendingWantUid(const sptr<IWantSender> &target);
    int32_t GetPendingWantUserId(const sptr<IWantSender> &target);
    std::string GetPendingWantBundleName(const sptr<IWantSender> &target);
    int32_t GetPendingWantCode(const sptr<IWantSender> &target);
    int32_t GetPendingWantType(const sptr<IWantSender> &target);
    void RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &recevier);
    void UnregisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &recevier);
    int32_t GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want);
    int32_t GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info);

    void CancelWantSenderLocked(PendingWantRecord &record, bool cleanAbility);
    int32_t PendingWantStartAbility(
        const Want &want, const sptr<IRemoteObject> &callerToken, int32_t requestCode, const int32_t callerUid);
    int32_t PendingWantStartAbilitys(const std::vector<WantsInfo> wnatsInfo, const sptr<IRemoteObject> &callerToken,
        int32_t requestCode, const int32_t callerUid);
    int32_t DeviceIdDetermine(
        const Want &want, const sptr<IRemoteObject> &callerToken, int32_t requestCode, const int32_t callerUid);
    int32_t PendingWantPublishCommonEvent(const Want &want, const SenderInfo &senderInfo, int32_t callerUid);
    void ClearPendingWantRecord(const std::string &bundleName);

private:
    sptr<IWantSender> GetWantSenderLocked(const int32_t callingUid, const int32_t uid, const int32_t userId,
        WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken);
    void MakeWantSenderCanceledLocked(PendingWantRecord &record);

    sptr<PendingWantRecord> GetPendingWantRecordByKey(const std::shared_ptr<PendingWantKey> &key);
    bool CheckPendingWantRecordByKey(
        const std::shared_ptr<PendingWantKey> &inputKey, const std::shared_ptr<PendingWantKey> &key);

    sptr<PendingWantRecord> GetPendingWantRecordByCode(int32_t code);
    static int32_t PendingRecordIdCreate();
    void ClearPendingWantRecordTask(const std::string &bundleName);

private:
    std::map<std::shared_ptr<PendingWantKey>, sptr<PendingWantRecord>> wantRecords_;
    std::recursive_mutex mutex_;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_AAFWK_PENDING_WANT_MANAGER_H
