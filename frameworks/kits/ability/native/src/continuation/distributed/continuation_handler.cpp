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
#include "continuation_handler.h"

#include "ability_manager_client.h"
#include "distributed_errors.h"
#include "element_name.h"
#include "hilog_wrapper.h"

using OHOS::AAFwk::WantParams;
namespace OHOS {
namespace AppExecFwk {
const std::string ContinuationHandler::ORIGINAL_DEVICE_ID("deviceId");
const std::string VERSION_CODE_KEY = "version";
ContinuationHandler::ContinuationHandler(
    std::weak_ptr<ContinuationManager> &continuationManager, std::weak_ptr<Ability> &ability)
{
    ability_ = ability;
    continuationManager_ = continuationManager;
}

bool ContinuationHandler::HandleStartContinuationWithStack(const sptr<IRemoteObject> &token,
    const std::string &deviceId, uint32_t versionCode)
{
    HILOG_INFO("%{public}s called begin", __func__);
    if (token == nullptr) {
        HILOG_ERROR("HandleStartContinuationWithStack token is null.");
        return false;
    }
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("HandleStartContinuationWithStack abilityInfo is null.");
        return false;
    }

    abilityInfo_->deviceId = deviceId;

    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        HILOG_ERROR("HandleStartContinuationWithStack: get continuationManagerTmp is nullptr");
        return false;
    }

    // decided to start continuation. Callback to ability.
    Want want;
    want.SetParam(VERSION_CODE_KEY, static_cast<int32_t>(versionCode));
    WantParams wantParams = want.GetParams();
    int32_t status = continuationManagerTmp->OnContinue(wantParams);
    if (status != ERR_OK) {
        HILOG_INFO("OnContinue failed, BundleName = %{public}s, ClassName= %{public}s, status: %{public}d",
            abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str(),
            status);
    }

    want.SetParams(wantParams);
    want.AddFlags(want.FLAG_ABILITY_CONTINUATION);
    want.SetElementName(deviceId, abilityInfo_->bundleName, abilityInfo_->name);

    int result = AAFwk::AbilityManagerClient::GetInstance()->StartContinuation(want, token, status);
    if (result != ERR_OK) {
        HILOG_ERROR("startContinuation failed.");
        return false;
    }
    HILOG_INFO("%{public}s called end", __func__);
    return true;
}

bool ContinuationHandler::HandleStartContinuation(const sptr<IRemoteObject> &token, const std::string &deviceId)
{
    HILOG_INFO("%{public}s called begin", __func__);
    if (token == nullptr) {
        HILOG_ERROR("ContinuationHandler::HandleStartContinuation token is null.");
        return false;
    }
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("ContinuationHandler::HandleStartContinuation abilityInfo is null.");
        return false;
    }

    abilityInfo_->deviceId = deviceId;

    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        HILOG_ERROR("handleStartContinuation: get continuationManagerTmp is nullptr");
        return false;
    }

    // DMS decided to start continuation. Callback to ability.
    if (!continuationManagerTmp->StartContinuation()) {
        HILOG_INFO("handleStartContinuation: Ability rejected.");
        HILOG_INFO("ID_ABILITY_SHELL_CONTINUE_ABILITY, BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str());
        return false;
    }

    WantParams wantParams;
    if (!continuationManagerTmp->SaveData(wantParams)) {
        HILOG_INFO("handleStartContinuation: ScheduleSaveData failed.");
        HILOG_INFO("ID_ABILITY_SHELL_CONTINUE_ABILITY, BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str());
        return false;
    }

    Want want = SetWantParams(wantParams);
    want.SetElementName(deviceId, abilityInfo_->bundleName, abilityInfo_->name);

    int result = AAFwk::AbilityManagerClient::GetInstance()->StartContinuation(want, token, 0);
    if (result != 0) {
        HILOG_ERROR("distClient_.startContinuation failed.");
        return false;
    }
    HILOG_INFO("%{public}s called end", __func__);
    return true;
}

void ContinuationHandler::HandleReceiveRemoteScheduler(const sptr<IRemoteObject> &remoteReplica)
{
    HILOG_INFO("%{public}s called begin", __func__);
    if (remoteReplica == nullptr) {
        HILOG_ERROR("scheduler is nullptr");
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
    HILOG_INFO("%{public}s called end", __func__);
}

void ContinuationHandler::HandleCompleteContinuation(int result)
{
    HILOG_INFO("%{public}s called begin", __func__);
    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        HILOG_ERROR("ContinuationHandler::HandleCompleteContinuation: get continuationManagerTmp is nullptr");
        return;
    }

    continuationManagerTmp->CompleteContinuation(result);
    HILOG_INFO("%{public}s called end", __func__);
}

void ContinuationHandler::SetReversible(bool reversible)
{
    HILOG_INFO("%{public}s called", __func__);
    reversible_ = reversible;
}

void ContinuationHandler::SetAbilityInfo(std::shared_ptr<AbilityInfo> &abilityInfo)
{
    HILOG_INFO("%{public}s called begin", __func__);
    abilityInfo_ = std::make_shared<AbilityInfo>(*(abilityInfo.get()));
    ClearDeviceInfo(abilityInfo_);
    HILOG_INFO("%{public}s called end", __func__);
}

void ContinuationHandler::SetPrimaryStub(const sptr<IRemoteObject> &Primary)
{
    HILOG_INFO("%{public}s called", __func__);
    remotePrimaryStub_ = Primary;
}

void ContinuationHandler::ClearDeviceInfo(std::shared_ptr<AbilityInfo> &abilityInfo)
{
    HILOG_INFO("%{public}s called", __func__);
    abilityInfo->deviceId = "";
    abilityInfo->deviceTypes.clear();
}

void ContinuationHandler::OnReplicaDied(const wptr<IRemoteObject> &remote)
{
    HILOG_INFO("%{public}s called begin", __func__);
    if (remoteReplicaProxy_ == nullptr) {
        HILOG_ERROR("BUG: remote death notifies to a unready replica.");
        return;
    }

    auto object = remote.promote();
    if (!object) {
        HILOG_ERROR("replica on remoteReplica died: null object.");
        return;
    }

    if (object != remoteReplicaProxy_->AsObject()) {
        HILOG_ERROR("replica on remoteReplica died: remoteReplica is not matches with remote.");
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
    HILOG_INFO("%{public}s called end", __func__);
}

void ContinuationHandler::NotifyReplicaTerminated()
{
    HILOG_INFO("%{public}s called begin", __func__);

    CleanUpAfterReverse();

    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        HILOG_ERROR("ContinuationHandler::NotifyReplicaTerminated: get continuationManagerTmp is nullptr");
        return;
    }
    HILOG_INFO("%{public}s called end", __func__);
    continuationManagerTmp->NotifyRemoteTerminated();
}

Want ContinuationHandler::SetWantParams(const WantParams &wantParams)
{
    HILOG_INFO("%{public}s called begin", __func__);
    Want want;
    want.SetParams(wantParams);
    want.AddFlags(want.FLAG_ABILITY_CONTINUATION);
    if (abilityInfo_->launchMode != LaunchMode::STANDARD) {
        HILOG_INFO("SetWantParams: Clear task.");
    }
    if (reversible_) {
        HILOG_INFO("SetWantParams: Reversible.");
        want.AddFlags(Want::FLAG_ABILITY_CONTINUATION_REVERSIBLE);
    }
    ElementName element("", abilityInfo_->bundleName, abilityInfo_->name);
    want.SetElement(element);
    HILOG_INFO("%{public}s called end", __func__);
    return want;
}

void ContinuationHandler::CleanUpAfterReverse()
{
    HILOG_INFO("%{public}s called", __func__);
    remoteReplicaProxy_ = nullptr;
}

void ContinuationHandler::PassPrimary(const sptr<IRemoteObject> &Primary)
{
    HILOG_INFO("%{public}s called", __func__);
    remotePrimaryProxy_ = iface_cast<IReverseContinuationSchedulerPrimary>(Primary);
}

bool ContinuationHandler::ReverseContinuation()
{
    HILOG_INFO("%{public}s called begin", __func__);

    if (remotePrimaryProxy_ == nullptr) {
        HILOG_ERROR("ReverseContinuation:remotePrimaryProxy_ not initialized, can not reverse");
        return false;
    }

    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("ReverseContinuation: abilityInfo is null");
        return false;
    }

    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        HILOG_ERROR("ReverseContinuation: get continuationManagerTmp is nullptr");
        return false;
    }

    if (!continuationManagerTmp->StartContinuation()) {
        HILOG_ERROR("ReverseContinuation: Ability rejected.");
        HILOG_INFO("ReverseContinuation, BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str());
        return false;
    }

    WantParams wantParams;
    if (!continuationManagerTmp->SaveData(wantParams)) {
        HILOG_ERROR("ReverseContinuation: SaveData failed.");
        HILOG_INFO("ReverseContinuation, BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str());
        return false;
    }

    Want want;
    want.SetParams(wantParams);
    if (remotePrimaryProxy_->ContinuationBack(want)) {
        HILOG_ERROR("reverseContinuation: ContinuationBack send failed.");
        return false;
    }
    HILOG_INFO("%{public}s called end", __func__);
    return true;
}

void ContinuationHandler::NotifyReverseResult(int reverseResult)
{
    HILOG_INFO("NotifyReverseResult: Start. result = %{public}d", reverseResult);
    if (reverseResult == 0) {
        std::shared_ptr<Ability> ability = nullptr;
        ability = ability_.lock();
        if (ability == nullptr) {
            HILOG_ERROR("ContinuationHandler::NotifyReverseResult failed. ability is nullptr");
            return;
        }
        ability->TerminateAbility();
    }
    HILOG_INFO("%{public}s called end", __func__);
}

bool ContinuationHandler::ContinuationBack(const Want &want)
{
    HILOG_INFO("%{public}s called begin", __func__);
    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        HILOG_ERROR("ContinuationBack: get continuationManagerTmp is nullptr");
        return false;
    }

    int result = 0;
    if (!continuationManagerTmp->RestoreFromRemote(want.GetParams())) {
        HILOG_INFO("ContinuationBack: RestoreFromRemote failed.");
        result = ABILITY_FAILED_RESTORE_DATA;
    }

    remoteReplicaProxy_->NotifyReverseResult(result);
    if (result == 0) {
        CleanUpAfterReverse();
    }
    HILOG_INFO("%{public}s called end", __func__);
    return true;
}

void ContinuationHandler::NotifyTerminationToPrimary()
{
    HILOG_INFO("%{public}s called begin", __func__);
    if (remotePrimaryProxy_ == nullptr) {
        HILOG_ERROR("NotifyTerminationToPrimary: remotePrimary not initialized, can not notify");
        return;
    }

    HILOG_INFO("NotifyTerminationToPrimary: Start");
    remotePrimaryProxy_->NotifyReplicaTerminated();
    HILOG_INFO("%{public}s called end", __func__);
}

bool ContinuationHandler::ReverseContinueAbility()
{
    HILOG_INFO("%{public}s called begin", __func__);
    if (remoteReplicaProxy_ == nullptr) {
        HILOG_ERROR("ReverseContinueAbility: remoteReplica not initialized, can not reverse");
        return false;
    }

    HILOG_INFO("ReverseContinueAbility: Start");
    bool requestSendSuccess = remoteReplicaProxy_->ReverseContinuation();
    HILOG_INFO("%{public}s called end", __func__);
    return requestSendSuccess;
}

}  // namespace AppExecFwk
}  // namespace OHOS