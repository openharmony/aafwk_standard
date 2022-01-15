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
#include "continuation_handler.h"

#include "ability_manager_client.h"
#include "app_log_wrapper.h"
#include "distributed_errors.h"
#include "element_name.h"

using OHOS::AAFwk::WantParams;
namespace OHOS {
namespace AppExecFwk {
const std::string ContinuationHandler::ORIGINAL_DEVICE_ID("deviceId");
ContinuationHandler::ContinuationHandler(
    std::weak_ptr<ContinuationManager> &continuationManager, std::weak_ptr<Ability> &ability)
{
    ability_ = ability;
    continuationManager_ = continuationManager;
}

bool ContinuationHandler::HandleStartContinuationWithStack(const sptr<IRemoteObject> &token,
    const std::string &deviceId)
{
    APP_LOGI("%{public}s called begin", __func__);
    if (token == nullptr) {
        APP_LOGE("HandleStartContinuationWithStack token is null.");
        return false;
    }
    if (abilityInfo_ == nullptr) {
        APP_LOGE("HandleStartContinuationWithStack abilityInfo is null.");
        return false;
    }

    abilityInfo_->deviceId = deviceId;

    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        APP_LOGE("HandleStartContinuationWithStack: get continuationManagerTmp is nullptr");
        return false;
    }

    // decided to start continuation. Callback to ability.
    WantParams wantParams;
    int32_t status = continuationManagerTmp->OnContinue(wantParams);
    if (status != ERR_OK) {
        APP_LOGI("OnContinue failed, BundleName = %{public}s, ClassName= %{public}s, status: %{public}d",
            abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str(),
            status);
    }

    Want want = SetWantParams(wantParams);
    want.SetElementName(deviceId, abilityInfo_->bundleName, abilityInfo_->name);

    int result = AAFwk::AbilityManagerClient::GetInstance()->StartContinuation(want, token, status);
    if (result != ERR_OK) {
        APP_LOGE("startContinuation failed.");
        return false;
    }
    APP_LOGI("%{public}s called end", __func__);
    return true;
}

bool ContinuationHandler::HandleStartContinuation(const sptr<IRemoteObject> &token, const std::string &deviceId)
{
    APP_LOGI("%{public}s called begin", __func__);
    if (token == nullptr) {
        APP_LOGE("ContinuationHandler::HandleStartContinuation token is null.");
        return false;
    }
    if (abilityInfo_ == nullptr) {
        APP_LOGE("ContinuationHandler::HandleStartContinuation abilityInfo is null.");
        return false;
    }

    abilityInfo_->deviceId = deviceId;

    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        APP_LOGE("handleStartContinuation: get continuationManagerTmp is nullptr");
        return false;
    }

    // DMS decided to start continuation. Callback to ability.
    if (!continuationManagerTmp->StartContinuation()) {
        APP_LOGI("handleStartContinuation: Ability rejected.");
        APP_LOGI("ID_ABILITY_SHELL_CONTINUE_ABILITY, BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str());
        return false;
    }

    WantParams wantParams;
    if (!continuationManagerTmp->SaveData(wantParams)) {
        APP_LOGI("handleStartContinuation: ScheduleSaveData failed.");
        APP_LOGI("ID_ABILITY_SHELL_CONTINUE_ABILITY, BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str());
        return false;
    }

    Want want = SetWantParams(wantParams);
    want.SetElementName(deviceId, abilityInfo_->bundleName, abilityInfo_->name);

    int result = AAFwk::AbilityManagerClient::GetInstance()->StartContinuation(want, token, 0);
    if (result != 0) {
        APP_LOGE("distClient_.startContinuation failed.");
        return false;
    }
    APP_LOGI("%{public}s called end", __func__);
    return true;
}

void ContinuationHandler::HandleReceiveRemoteScheduler(const sptr<IRemoteObject> &remoteReplica)
{
    APP_LOGI("%{public}s called begin", __func__);
    if (remoteReplica == nullptr) {
        APP_LOGE("scheduler is nullptr");
        return;
    }

    if (remoteReplicaProxy_ != nullptr && schedulerDeathRecipient_ != nullptr) {
        auto schedulerObjectTmp = remoteReplicaProxy_->AsObject();
        if (schedulerObjectTmp != nullptr) {
            schedulerObjectTmp->RemoveDeathRecipient(schedulerDeathRecipient_);
        }
    }

    if (schedulerDeathRecipient_ == nullptr) {
        schedulerDeathRecipient_ = new (std::nothrow) ReverseContinuationSchedulerRecipient(
            std::bind(&ContinuationHandler::OnReplicaDied, this, std::placeholders::_1));
    }

    remoteReplicaProxy_ = iface_cast<IReverseContinuationSchedulerReplica>(remoteReplica);
    auto schedulerObject = remoteReplicaProxy_->AsObject();
    if (schedulerObject != nullptr) {
        schedulerObject->AddDeathRecipient(schedulerDeathRecipient_);
    }

    remoteReplicaProxy_->PassPrimary(remotePrimaryStub_);
    APP_LOGI("%{public}s called end", __func__);
}

void ContinuationHandler::HandleCompleteContinuation(int result)
{
    APP_LOGI("%{public}s called begin", __func__);
    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        APP_LOGE("ContinuationHandler::HandleCompleteContinuation: get continuationManagerTmp is nullptr");
        return;
    }

    continuationManagerTmp->CompleteContinuation(result);
    APP_LOGI("%{public}s called end", __func__);
}

void ContinuationHandler::SetReversible(bool reversible)
{
    APP_LOGI("%{public}s called", __func__);
    reversible_ = reversible;
}

void ContinuationHandler::SetAbilityInfo(std::shared_ptr<AbilityInfo> &abilityInfo)
{
    APP_LOGI("%{public}s called begin", __func__);
    abilityInfo_ = std::make_shared<AbilityInfo>(*(abilityInfo.get()));
    ClearDeviceInfo(abilityInfo_);
    APP_LOGI("%{public}s called end", __func__);
}

void ContinuationHandler::SetPrimaryStub(const sptr<IRemoteObject> &Primary)
{
    APP_LOGI("%{public}s called", __func__);
    remotePrimaryStub_ = Primary;
}

void ContinuationHandler::ClearDeviceInfo(std::shared_ptr<AbilityInfo> &abilityInfo)
{
    APP_LOGI("%{public}s called", __func__);
    abilityInfo->deviceId = "";
    abilityInfo->deviceTypes.clear();
}

void ContinuationHandler::OnReplicaDied(const wptr<IRemoteObject> &remote)
{
    APP_LOGI("%{public}s called begin", __func__);
    if (remoteReplicaProxy_ == nullptr) {
        APP_LOGE("BUG: remote death notifies to a unready replica.");
        return;
    }

    auto object = remote.promote();
    if (!object) {
        APP_LOGE("replica on remoteReplica died: null object.");
        return;
    }

    if (object != remoteReplicaProxy_->AsObject()) {
        APP_LOGE("replica on remoteReplica died: remoteReplica is not matches with remote.");
        return;
    }

    if (remoteReplicaProxy_ != nullptr && schedulerDeathRecipient_ != nullptr) {
        auto schedulerObject = remoteReplicaProxy_->AsObject();
        if (schedulerObject != nullptr) {
            schedulerObject->RemoveDeathRecipient(schedulerDeathRecipient_);
        }
    }
    remoteReplicaProxy_.clear();

    NotifyReplicaTerminated();
    APP_LOGI("%{public}s called end", __func__);
}

void ContinuationHandler::NotifyReplicaTerminated()
{
    APP_LOGI("%{public}s called begin", __func__);

    CleanUpAfterReverse();

    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        APP_LOGE("ContinuationHandler::NotifyReplicaTerminated: get continuationManagerTmp is nullptr");
        return;
    }
    APP_LOGI("%{public}s called end", __func__);
    continuationManagerTmp->NotifyRemoteTerminated();
}

Want ContinuationHandler::SetWantParams(const WantParams &wantParams)
{
    APP_LOGI("%{public}s called begin", __func__);
    Want want;
    want.SetParams(wantParams);
    want.AddFlags(want.FLAG_ABILITY_CONTINUATION);
    if (abilityInfo_->launchMode != LaunchMode::STANDARD) {
        APP_LOGI("SetWantParams: Clear task.");
    }
    if (reversible_) {
        APP_LOGI("SetWantParams: Reversible.");
        want.AddFlags(Want::FLAG_ABILITY_CONTINUATION_REVERSIBLE);
    }
    ElementName element("", abilityInfo_->bundleName, abilityInfo_->name);
    want.SetElement(element);
    APP_LOGI("%{public}s called end", __func__);
    return want;
}

void ContinuationHandler::CleanUpAfterReverse()
{
    APP_LOGI("%{public}s called", __func__);
    remoteReplicaProxy_ = nullptr;
}

void ContinuationHandler::PassPrimary(const sptr<IRemoteObject> &Primary)
{
    APP_LOGI("%{public}s called", __func__);
    remotePrimaryProxy_ = iface_cast<IReverseContinuationSchedulerPrimary>(Primary);
}

bool ContinuationHandler::ReverseContinuation()
{
    APP_LOGI("%{public}s called begin", __func__);

    if (remotePrimaryProxy_ == nullptr) {
        APP_LOGE("ReverseContinuation:remotePrimaryProxy_ not initialized, can not reverse");
        return false;
    }

    if (abilityInfo_ == nullptr) {
        APP_LOGE("ReverseContinuation: abilityInfo is null");
        return false;
    }

    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        APP_LOGE("ReverseContinuation: get continuationManagerTmp is nullptr");
        return false;
    }

    if (!continuationManagerTmp->StartContinuation()) {
        APP_LOGE("ReverseContinuation: Ability rejected.");
        APP_LOGI("ReverseContinuation, BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str());
        return false;
    }

    WantParams wantParams;
    if (!continuationManagerTmp->SaveData(wantParams)) {
        APP_LOGE("ReverseContinuation: SaveData failed.");
        APP_LOGI("ReverseContinuation, BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str());
        return false;
    }

    Want want;
    want.SetParams(wantParams);
    if (remotePrimaryProxy_->ContinuationBack(want)) {
        APP_LOGE("reverseContinuation: ContinuationBack send failed.");
        return false;
    }
    APP_LOGI("%{public}s called end", __func__);
    return true;
}

void ContinuationHandler::NotifyReverseResult(int reverseResult)
{
    APP_LOGI("NotifyReverseResult: Start. result = %{public}d", reverseResult);
    if (reverseResult == 0) {
        std::shared_ptr<Ability> ability = nullptr;
        ability = ability_.lock();
        if (ability == nullptr) {
            APP_LOGE("ContinuationHandler::NotifyReverseResult failed. ability is nullptr");
            return;
        }
        ability->TerminateAbility();
    }
    APP_LOGI("%{public}s called end", __func__);
}

bool ContinuationHandler::ContinuationBack(const Want &want)
{
    APP_LOGI("%{public}s called begin", __func__);
    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        APP_LOGE("ContinuationBack: get continuationManagerTmp is nullptr");
        return false;
    }

    int result = 0;
    if (!continuationManagerTmp->RestoreFromRemote(want.GetParams())) {
        APP_LOGI("ContinuationBack: RestoreFromRemote failed.");
        result = ABILITY_FAILED_RESTORE_DATA;
    }

    remoteReplicaProxy_->NotifyReverseResult(result);
    if (result == 0) {
        CleanUpAfterReverse();
    }
    APP_LOGI("%{public}s called end", __func__);
    return true;
}

void ContinuationHandler::NotifyTerminationToPrimary()
{
    APP_LOGI("%{public}s called begin", __func__);
    if (remotePrimaryProxy_ == nullptr) {
        APP_LOGE("NotifyTerminationToPrimary: remotePrimary not initialized, can not notify");
        return;
    }

    APP_LOGI("NotifyTerminationToPrimary: Start");
    remotePrimaryProxy_->NotifyReplicaTerminated();
    APP_LOGI("%{public}s called end", __func__);
}

bool ContinuationHandler::ReverseContinueAbility()
{
    APP_LOGI("%{public}s called begin", __func__);
    if (remoteReplicaProxy_ == nullptr) {
        APP_LOGE("ReverseContinueAbility: remoteReplica not initialized, can not reverse");
        return false;
    }

    APP_LOGI("ReverseContinueAbility: Start");
    bool requestSendSuccess = remoteReplicaProxy_->ReverseContinuation();
    APP_LOGI("%{public}s called end", __func__);
    return requestSendSuccess;
}

}  // namespace AppExecFwk
}  // namespace OHOS