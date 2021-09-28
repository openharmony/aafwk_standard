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

#include "ability_manager_service.h"

#include <functional>
#include <memory>
#include <string>
#include <unistd.h>
#include "string_ex.h"

#include "ability_util.h"
#include "ability_info.h"
#include "ability_manager_errors.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"

using OHOS::AppExecFwk::ElementName;

namespace OHOS {
namespace AAFwk {
using namespace std::chrono;
constexpr auto DATA_ABILITY_START_TIMEOUT = 5s;
const std::map<std::string, AbilityManagerService::DumpKey> AbilityManagerService::dumpMap = {
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--all", KEY_DUMP_ALL),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-a", KEY_DUMP_ALL),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--stack-list", KEY_DUMP_STACK_LIST),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-l", KEY_DUMP_STACK_LIST),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--stack", KEY_DUMP_STACK),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-s", KEY_DUMP_STACK),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--mission", KEY_DUMP_MISSION),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-m", KEY_DUMP_MISSION),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--top", KEY_DUMP_TOP_ABILITY),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-t", KEY_DUMP_TOP_ABILITY),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--waitting-queue", KEY_DUMP_WAIT_QUEUE),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-w", KEY_DUMP_WAIT_QUEUE),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--serv", KEY_DUMP_SERVICE),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-e", KEY_DUMP_SERVICE),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--data", KEY_DUMP_DATA),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-d", KEY_DUMP_DATA),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--ui", KEY_DUMP_SYSTEM_UI),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-u", KEY_DUMP_SYSTEM_UI),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-focus", KEY_DUMP_FOCUS_ABILITY),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-f", KEY_DUMP_FOCUS_ABILITY),
};
const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<AbilityManagerService>::GetInstance().get());

AbilityManagerService::AbilityManagerService()
    : SystemAbility(ABILITY_MGR_SERVICE_ID, true),
      eventLoop_(nullptr),
      handler_(nullptr),
      state_(ServiceRunningState::STATE_NOT_START),
      connectManager_(std::make_shared<AbilityConnectManager>()),
      iBundleManager_(nullptr)
{
    std::shared_ptr<AppScheduler> appScheduler(
        DelayedSingleton<AppScheduler>::GetInstance().get(), [](AppScheduler *x) { x->DecStrongRef(x); });
    appScheduler_ = appScheduler;
    DumpFuncInit();
}

AbilityManagerService::~AbilityManagerService()
{}

void AbilityManagerService::OnStart()
{
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        HILOG_INFO("Ability manager service has already started.");
        return;
    }
    HILOG_INFO("Ability manager service started.");
    if (!Init()) {
        HILOG_ERROR("Failed to init service.");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    eventLoop_->Run();
    /* Publish service maybe failed, so we need call this function at the last,
     * so it can't affect the TDD test program */
    bool ret = Publish(DelayedSingleton<AbilityManagerService>::GetInstance().get());
    if (!ret) {
        HILOG_ERROR("Init publish failed!");
        return;
    }

    HILOG_INFO("Ability manager service start success.");
}

bool AbilityManagerService::Init()
{
    eventLoop_ = AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    CHECK_POINTER_RETURN_BOOL(eventLoop_);

    handler_ = std::make_shared<AbilityEventHandler>(eventLoop_, weak_from_this());
    CHECK_POINTER_RETURN_BOOL(handler_);
    CHECK_POINTER_RETURN_BOOL(connectManager_);
    connectManager_->SetEventHandler(handler_);

    auto dataAbilityManager = std::make_shared<DataAbilityManager>();
    CHECK_POINTER_RETURN_BOOL(dataAbilityManager);

    amsConfigResolver_ = std::make_shared<AmsConfigurationParameter>();
    if (amsConfigResolver_) {
        amsConfigResolver_->Parse();
        HILOG_INFO("ams config parse");
    }

    auto pendingWantManager = std::make_shared<PendingWantManager>();
    if (!pendingWantManager) {
        HILOG_ERROR("Failed to init pending want ability manager.");
        return false;
    }

    int userId = GetUserId();
    SetStackManager(userId);
    systemAppManager_ = std::make_shared<KernalSystemAppManager>(userId);
    CHECK_POINTER_RETURN_BOOL(systemAppManager_);

    auto startLauncherAbilityTask = [aams = shared_from_this()]() { aams->StartSystemApplication(); };
    handler_->PostTask(startLauncherAbilityTask, "startLauncherAbility");
    dataAbilityManager_ = dataAbilityManager;
    pendingWantManager_ = pendingWantManager;
    HILOG_INFO("Init success.");
    return true;
}

void AbilityManagerService::OnStop()
{
    HILOG_INFO("Stop service.");
    eventLoop_.reset();
    handler_.reset();
    state_ = ServiceRunningState::STATE_NOT_START;
}

ServiceRunningState AbilityManagerService::QueryServiceState() const
{
    return state_;
}

int AbilityManagerService::StartAbility(const Want &want, int requestCode)
{
    HILOG_INFO("%{public}s", __func__);
    return StartAbility(want, nullptr, requestCode, -1);
}

int AbilityManagerService::StartAbility(const Want &want, const sptr<IRemoteObject> &callerToken, int requestCode)
{
    HILOG_INFO("%{public}s", __func__);
    return StartAbility(want, callerToken, requestCode, -1);
}

int AbilityManagerService::StartAbility(
    const Want &want, const sptr<IRemoteObject> &callerToken, int requestCode, int callerUid)
{
    HILOG_INFO("%{public}s", __func__);
    if (callerToken != nullptr && !VerificationToken(callerToken)) {
        return ERR_INVALID_VALUE;
    }

    AbilityRequest abilityRequest;
    int result = GenerateAbilityRequest(want, requestCode, abilityRequest, callerToken);
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request error.");
        return result;
    }
    auto abilityInfo = abilityRequest.abilityInfo;
    result = AbilityUtil::JudgeAbilityVisibleControl(abilityInfo, callerUid);
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return result;
    }
    auto type = abilityInfo.type;
    if (type == AppExecFwk::AbilityType::DATA) {
        HILOG_ERROR("Cannot start data ability, use 'AcquireDataAbility()' instead.");
        return ERR_INVALID_VALUE;
    }

    if (!AbilityUtil::IsSystemDialogAbility(abilityInfo.bundleName, abilityInfo.name)) {
        result = PreLoadAppDataAbilities(abilityInfo.bundleName);
        if (result != ERR_OK) {
            HILOG_ERROR("StartAbility: App data ability preloading failed, '%{public}s', %{public}d",
                abilityInfo.bundleName.c_str(),
                result);
            return result;
        }
    }

    if (type == AppExecFwk::AbilityType::SERVICE) {
        return connectManager_->StartAbility(abilityRequest);
    }

    if (IsSystemUiApp(abilityRequest.abilityInfo)) {
        return systemAppManager_->StartAbility(abilityRequest);
    }

    return currentStackManager_->StartAbility(abilityRequest);
}

int AbilityManagerService::StartAbility(const Want &want, const AbilityStartSetting &abilityStartSetting,
    const sptr<IRemoteObject> &callerToken, int requestCode)
{
    HILOG_INFO("Start ability setting.");
    if (callerToken != nullptr && !VerificationToken(callerToken)) {
        return ERR_INVALID_VALUE;
    }

    AbilityRequest abilityRequest;
    int result = GenerateAbilityRequest(want, requestCode, abilityRequest, callerToken);
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request error.");
        return result;
    }

    abilityRequest.startSetting = std::make_shared<AbilityStartSetting>(abilityStartSetting);

    auto abilityInfo = abilityRequest.abilityInfo;
    if (abilityInfo.type == AppExecFwk::AbilityType::DATA) {
        HILOG_ERROR("Cannot start data ability, use 'AcquireDataAbility()' instead.");
        return ERR_INVALID_VALUE;
    }

    if (!AbilityUtil::IsSystemDialogAbility(abilityInfo.bundleName, abilityInfo.name)) {
        result = PreLoadAppDataAbilities(abilityInfo.bundleName);
        if (result != ERR_OK) {
            HILOG_ERROR("StartAbility: App data ability preloading failed, '%{public}s', %{public}d",
                abilityInfo.bundleName.c_str(),
                result);
            return result;
        }
    }

    if (abilityInfo.type != AppExecFwk::AbilityType::PAGE) {
        HILOG_ERROR("Only support for page type ability.");
        return ERR_INVALID_VALUE;
    }
    if (IsSystemUiApp(abilityRequest.abilityInfo)) {
        return systemAppManager_->StartAbility(abilityRequest);
    }
    return currentStackManager_->StartAbility(abilityRequest);
}

int AbilityManagerService::TerminateAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant)
{
    HILOG_INFO("Terminate ability for result: %{public}d", (resultWant != nullptr));
    if (!VerificationToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    int result = AbilityUtil::JudgeAbilityVisibleControl(abilityRecord->GetAbilityInfo());
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return result;
    }

    if (IsSystemUiApp(abilityRecord->GetAbilityInfo())) {
        HILOG_ERROR("System ui not allow terminate.");
        return ERR_INVALID_VALUE;
    }

    auto type = abilityRecord->GetAbilityInfo().type;
    if (type == AppExecFwk::AbilityType::SERVICE) {
        return connectManager_->TerminateAbility(token);
    }

    if (type == AppExecFwk::AbilityType::DATA) {
        HILOG_ERROR("Cannot terminate data ability, use 'ReleaseDataAbility()' instead.");
        return ERR_INVALID_VALUE;
    }

    if ((resultWant != nullptr) &&
        AbilityUtil::IsSystemDialogAbility(
        abilityRecord->GetAbilityInfo().bundleName, abilityRecord->GetAbilityInfo().name) &&
        resultWant->HasParameter(AbilityConfig::SYSTEM_DIALOG_KEY) &&
        resultWant->HasParameter(AbilityConfig::SYSTEM_DIALOG_CALLER_BUNDLENAME) &&
        resultWant->HasParameter(AbilityConfig::SYSTEM_DIALOG_REQUEST_PERMISSIONS)) {
        RequestPermission(resultWant);
    }

    return currentStackManager_->TerminateAbility(token, resultCode, resultWant);
}

void AbilityManagerService::RequestPermission(const Want *resultWant)
{
    HILOG_INFO("Request permission.");
    CHECK_POINTER(iBundleManager_);
    CHECK_POINTER_IS_NULLPTR(resultWant);

    auto callerBundleName = resultWant->GetStringParam(AbilityConfig::SYSTEM_DIALOG_CALLER_BUNDLENAME);
    auto permissions = resultWant->GetStringArrayParam(AbilityConfig::SYSTEM_DIALOG_REQUEST_PERMISSIONS);

    for (auto &it : permissions) {
        auto ret = iBundleManager_->RequestPermissionFromUser(callerBundleName, it, GetUserId());
        HILOG_INFO("Request permission from user result :%{public}d, permission:%{public}s.", ret, it.c_str());
    }
}

int AbilityManagerService::TerminateAbilityByCaller(const sptr<IRemoteObject> &callerToken, int requestCode)
{
    HILOG_INFO("Terminate ability by caller.");
    if (!VerificationToken(callerToken)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (IsSystemUiApp(abilityRecord->GetAbilityInfo())) {
        HILOG_ERROR("System ui not allow terminate.");
        return ERR_INVALID_VALUE;
    }

    auto type = abilityRecord->GetAbilityInfo().type;
    switch (type) {
        case AppExecFwk::AbilityType::SERVICE: {
            auto result = connectManager_->TerminateAbility(abilityRecord, requestCode);
            if (result == NO_FOUND_ABILITY_BY_CALLER) {
                return currentStackManager_->TerminateAbility(abilityRecord, requestCode);
            }
            return result;
        }
        case AppExecFwk::AbilityType::PAGE: {
            auto result = currentStackManager_->TerminateAbility(abilityRecord, requestCode);
            if (result == NO_FOUND_ABILITY_BY_CALLER) {
                return connectManager_->TerminateAbility(abilityRecord, requestCode);
            }
            return result;
        }
        default:
            return ERR_INVALID_VALUE;
    }
}

int AbilityManagerService::GetRecentMissions(
    const int32_t numMax, const int32_t flags, std::vector<AbilityMissionInfo> &recentList)
{
    HILOG_INFO("numMax: %{public}d, flags: %{public}d", numMax, flags);
    if (numMax < 0 || flags < 0) {
        HILOG_ERROR("numMax or flags is invalid.");
        return ERR_INVALID_VALUE;
    }
    if (!CheckCallerIsSystemAppByIpc()) {
        HILOG_ERROR("caller is not systemApp");
        return CALLER_ISNOT_SYSTEMAPP;
    }

    return currentStackManager_->GetRecentMissions(numMax, flags, recentList);
}

int AbilityManagerService::GetMissionSnapshot(const int32_t missionId, MissionSnapshotInfo &snapshot)
{
    return 0;
}

int AbilityManagerService::SetMissionDescriptionInfo(
    const sptr<IRemoteObject> &token, const MissionDescriptionInfo &description)
{
    HILOG_INFO("%{public}s called", __func__);
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(currentStackManager_, INNER_ERR);

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    return currentStackManager_->SetMissionDescriptionInfo(abilityRecord, description);
}

int AbilityManagerService::GetMissionLockModeState()
{
    HILOG_INFO("%{public}s called", __func__);
    CHECK_POINTER_AND_RETURN(currentStackManager_, ERR_INVALID_VALUE);
    return currentStackManager_->GetMissionLockModeState();
}

int AbilityManagerService::UpdateConfiguration(const DummyConfiguration &config)
{
    HILOG_INFO("%{public}s called", __func__);
    CHECK_POINTER_AND_RETURN(currentStackManager_, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(systemAppManager_, ERR_INVALID_VALUE);

    systemAppManager_->UpdateConfiguration(config);

    return currentStackManager_->UpdateConfiguration(config);
}

int AbilityManagerService::MoveMissionToTop(int32_t missionId)
{
    HILOG_INFO("Move mission to top.");
    if (missionId < 0) {
        HILOG_ERROR("Mission id is invalid.");
        return ERR_INVALID_VALUE;
    }

    return currentStackManager_->MoveMissionToTop(missionId);
}

int AbilityManagerService::MoveMissionToEnd(const sptr<IRemoteObject> &token, const bool nonFirst)
{
    HILOG_INFO("Move mission to end.");
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);
    if (!VerificationToken(token)) {
        return ERR_INVALID_VALUE;
    }
    return currentStackManager_->MoveMissionToEnd(token, nonFirst);
}

int AbilityManagerService::RemoveMission(int id)
{
    HILOG_INFO("Remove mission.");
    if (id < 0) {
        HILOG_ERROR("Mission id is invalid.");
        return ERR_INVALID_VALUE;
    }
    if (!CheckCallerIsSystemAppByIpc()) {
        HILOG_ERROR("caller is not systemApp");
        return CALLER_ISNOT_SYSTEMAPP;
    }
    return currentStackManager_->RemoveMissionById(id);
}

int AbilityManagerService::RemoveStack(int id)
{
    HILOG_INFO("Remove stack.");
    if (id < 0) {
        HILOG_ERROR("Stack id is invalid.");
        return ERR_INVALID_VALUE;
    }
    return currentStackManager_->RemoveStack(id);
}

int AbilityManagerService::ConnectAbility(
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("Connect ability.");
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);
    AbilityRequest abilityRequest;
    int result = GenerateAbilityRequest(want, DEFAULT_INVAL_VALUE, abilityRequest, callerToken);
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request error.");
        return result;
    }
    auto abilityInfo = abilityRequest.abilityInfo;
    result = AbilityUtil::JudgeAbilityVisibleControl(abilityInfo);
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return result;
    }
    auto type = abilityInfo.type;
    if (type != AppExecFwk::AbilityType::SERVICE) {
        HILOG_ERROR("Connect Ability failed, target Ability is not Service.");
        return TARGET_ABILITY_NOT_SERVICE;
    }
    result = PreLoadAppDataAbilities(abilityInfo.bundleName);
    if (result != ERR_OK) {
        HILOG_ERROR("ConnectAbility: App data ability preloading failed, '%{public}s', %{public}d",
            abilityInfo.bundleName.c_str(),
            result);
        return result;
    }
    return connectManager_->ConnectAbilityLocked(abilityRequest, connect, callerToken);
}

int AbilityManagerService::DisconnectAbility(const sptr<IAbilityConnection> &connect)
{
    HILOG_DEBUG("Disconnect ability.");
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);
    return connectManager_->DisconnectAbilityLocked(connect);
}

void AbilityManagerService::RemoveAllServiceRecord()
{
    connectManager_->RemoveAll();
}

sptr<IWantSender> AbilityManagerService::GetWantSender(
    const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("Get want Sender.");
    CHECK_POINTER_AND_RETURN(pendingWantManager_, nullptr);

    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, nullptr);

    int32_t callerUid = IPCSkeleton::GetCallingUid();
    AppExecFwk::BundleInfo bundleInfo;
    bool bundleMgrResult =
        bms->GetBundleInfo(wantSenderInfo.bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo);
    if (!bundleMgrResult) {
        HILOG_ERROR("GetBundleInfo is fail.");
        return nullptr;
    }

    HILOG_INFO("AbilityManagerService::GetWantSender: bundleName = %{public}s", wantSenderInfo.bundleName.c_str());
    return pendingWantManager_->GetWantSender(
        callerUid, bundleInfo.uid, bms->CheckIsSystemAppByUid(callerUid), wantSenderInfo, callerToken);
}

int AbilityManagerService::SendWantSender(const sptr<IWantSender> &target, const SenderInfo &senderInfo)
{
    HILOG_INFO("Send want sender.");
    CHECK_POINTER_AND_RETURN(pendingWantManager_, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(target, ERR_INVALID_VALUE);
    return pendingWantManager_->SendWantSender(target, senderInfo);
}

void AbilityManagerService::CancelWantSender(const sptr<IWantSender> &sender)
{
    HILOG_INFO("Cancel want sender.");
    CHECK_POINTER(pendingWantManager_);
    CHECK_POINTER(sender);

    auto bms = GetBundleManager();
    CHECK_POINTER(bms);

    int32_t callerUid = IPCSkeleton::GetCallingUid();
    sptr<PendingWantRecord> record = iface_cast<PendingWantRecord>(sender->AsObject());

    AppExecFwk::BundleInfo bundleInfo;
    bool bundleMgrResult =
        bms->GetBundleInfo(record->GetKey()->GetBundleName(), AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo);
    if (!bundleMgrResult) {
        HILOG_ERROR("GetBundleInfo is fail.");
        return;
    }

    pendingWantManager_->CancelWantSender(callerUid, bundleInfo.uid, bms->CheckIsSystemAppByUid(callerUid), sender);
}

int AbilityManagerService::GetPendingWantUid(const sptr<IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if (pendingWantManager_ == nullptr) {
        HILOG_ERROR("%s, pendingWantManager_ is nullptr", __func__);
        return -1;
    }
    if (target == nullptr) {
        HILOG_ERROR("%s, target is nullptr", __func__);
        return -1;
    }
    return pendingWantManager_->GetPendingWantUid(target);
}

int AbilityManagerService::GetPendingWantUserId(const sptr<IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if (pendingWantManager_ == nullptr) {
        HILOG_ERROR("%s, pendingWantManager_ is nullptr", __func__);
        return -1;
    }
    if (target == nullptr) {
        HILOG_ERROR("%s, target is nullptr", __func__);
        return -1;
    }
    return pendingWantManager_->GetPendingWantUserId(target);
}

std::string AbilityManagerService::GetPendingWantBundleName(const sptr<IWantSender> &target)
{
    HILOG_INFO("Get pending want bundle name.");
    CHECK_POINTER_AND_RETURN(pendingWantManager_, "");
    CHECK_POINTER_AND_RETURN(target, "");
    return pendingWantManager_->GetPendingWantBundleName(target);
}

int AbilityManagerService::GetPendingWantCode(const sptr<IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if (pendingWantManager_ == nullptr) {
        HILOG_ERROR("%s, pendingWantManager_ is nullptr", __func__);
        return -1;
    }
    if (target == nullptr) {
        HILOG_ERROR("%s, target is nullptr", __func__);
        return -1;
    }
    return pendingWantManager_->GetPendingWantCode(target);
}

int AbilityManagerService::GetPendingWantType(const sptr<IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if (pendingWantManager_ == nullptr) {
        HILOG_ERROR("%s, pendingWantManager_ is nullptr", __func__);
        return -1;
    }
    if (target == nullptr) {
        HILOG_ERROR("%s, target is nullptr", __func__);
        return -1;
    }
    return pendingWantManager_->GetPendingWantType(target);
}

void AbilityManagerService::RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver)
{
    HILOG_INFO("Register cancel listener.");
    CHECK_POINTER(pendingWantManager_);
    CHECK_POINTER(sender);
    CHECK_POINTER(receiver);
    pendingWantManager_->RegisterCancelListener(sender, receiver);
}

void AbilityManagerService::UnregisterCancelListener(
    const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver)
{
    HILOG_INFO("Unregister cancel listener.");
    CHECK_POINTER(pendingWantManager_);
    CHECK_POINTER(sender);
    CHECK_POINTER(receiver);
    pendingWantManager_->UnregisterCancelListener(sender, receiver);
}

int AbilityManagerService::GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want)
{
    HILOG_INFO("Get pending request want.");
    CHECK_POINTER_AND_RETURN(pendingWantManager_, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(target, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(want, ERR_INVALID_VALUE);
    return pendingWantManager_->GetPendingRequestWant(target, want);
}

std::shared_ptr<AbilityRecord> AbilityManagerService::GetServiceRecordByElementName(const std::string &element)
{
    return connectManager_->GetServiceRecordByElementName(element);
}

std::list<std::shared_ptr<ConnectionRecord>> AbilityManagerService::GetConnectRecordListByCallback(
    sptr<IAbilityConnection> callback)
{
    return connectManager_->GetConnectRecordListByCallback(callback);
}

sptr<IAbilityScheduler> AbilityManagerService::AcquireDataAbility(
    const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s, called. uid %{public}d", __func__, IPCSkeleton::GetCallingUid());
    bool isSystem = (IPCSkeleton::GetCallingUid() == AbilityUtil::SYSTEM_UID);
    if (!isSystem) {
        HILOG_INFO("callerToken not system %{public}s", __func__);
        if (!VerificationToken(callerToken)) {
            HILOG_INFO("VerificationToken fail");
            return nullptr;
        }
    }

    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, nullptr);

    auto localUri(uri);
    if (localUri.GetScheme() != AbilityConfig::SCHEME_DATA_ABILITY) {
        HILOG_ERROR("Acquire data ability with invalid uri scheme.");
        return nullptr;
    }
    std::vector<std::string> pathSegments;
    localUri.GetPathSegments(pathSegments);
    if (pathSegments.empty()) {
        HILOG_ERROR("Acquire data ability with invalid uri path.");
        return nullptr;
    }

    AbilityRequest abilityRequest;
    std::string dataAbilityUri = localUri.ToString();
    bool queryResult = iBundleManager_->QueryAbilityInfoByUri(dataAbilityUri, abilityRequest.abilityInfo);
    if (!queryResult || abilityRequest.abilityInfo.name.empty() || abilityRequest.abilityInfo.bundleName.empty()) {
        HILOG_ERROR("Invalid ability info for data ability acquiring.");
        return nullptr;
    }
    int result = AbilityUtil::JudgeAbilityVisibleControl(abilityRequest.abilityInfo);
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return nullptr;
    }
    abilityRequest.appInfo = abilityRequest.abilityInfo.applicationInfo;
    if (abilityRequest.appInfo.name.empty() || abilityRequest.appInfo.bundleName.empty()) {
        HILOG_ERROR("Invalid app info for data ability acquiring.");
        return nullptr;
    }
    if (abilityRequest.abilityInfo.type != AppExecFwk::AbilityType::DATA) {
        HILOG_ERROR("BMS query result is not a data ability.");
        return nullptr;
    }
    HILOG_DEBUG("Query data ability info: %{public}s|%{public}s|%{public}s",
        abilityRequest.appInfo.name.c_str(),
        abilityRequest.appInfo.bundleName.c_str(),
        abilityRequest.abilityInfo.name.c_str());

    return dataAbilityManager_->Acquire(abilityRequest, tryBind, callerToken, isSystem);
}

int AbilityManagerService::ReleaseDataAbility(
    sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s, called.", __func__);
    bool isSystem = (IPCSkeleton::GetCallingUid() == AbilityUtil::SYSTEM_UID);
    if (!isSystem) {
        HILOG_INFO("callerToken not system %{public}s", __func__);
        if (!VerificationToken(callerToken)) {
            HILOG_INFO("VerificationToken fail");
            return ERR_INVALID_STATE;
        }
    }

    return dataAbilityManager_->Release(dataAbilityScheduler, callerToken, isSystem);
}

int AbilityManagerService::AttachAbilityThread(
    const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Attach ability thread.");
    CHECK_POINTER_AND_RETURN(scheduler, ERR_INVALID_VALUE);

    if (!VerificationToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    auto abilityInfo = abilityRecord->GetAbilityInfo();
    auto type = abilityInfo.type;
    if (type == AppExecFwk::AbilityType::SERVICE) {
        return connectManager_->AttachAbilityThreadLocked(scheduler, token);
    }
    if (type == AppExecFwk::AbilityType::DATA) {
        return dataAbilityManager_->AttachAbilityThread(scheduler, token);
    }
    if (IsSystemUiApp(abilityInfo)) {
        return systemAppManager_->AttachAbilityThread(scheduler, token);
    }
    return currentStackManager_->AttachAbilityThread(scheduler, token);
}

void AbilityManagerService::DumpFuncInit()
{
    dumpFuncMap_[KEY_DUMP_ALL] = &AbilityManagerService::DumpInner;
    dumpFuncMap_[KEY_DUMP_STACK_LIST] = &AbilityManagerService::DumpStackListInner;
    dumpFuncMap_[KEY_DUMP_STACK] = &AbilityManagerService::DumpStackInner;
    dumpFuncMap_[KEY_DUMP_MISSION] = &AbilityManagerService::DumpMissionInner;
    dumpFuncMap_[KEY_DUMP_TOP_ABILITY] = &AbilityManagerService::DumpTopAbilityInner;
    dumpFuncMap_[KEY_DUMP_WAIT_QUEUE] = &AbilityManagerService::DumpWaittingAbilityQueueInner;
    dumpFuncMap_[KEY_DUMP_SERVICE] = &AbilityManagerService::DumpStateInner;
    dumpFuncMap_[KEY_DUMP_DATA] = &AbilityManagerService::DataDumpStateInner;
    dumpFuncMap_[KEY_DUMP_SYSTEM_UI] = &AbilityManagerService::SystemDumpStateInner;
    dumpFuncMap_[KEY_DUMP_FOCUS_ABILITY] = &AbilityManagerService::DumpFocusMapInner;
}

void AbilityManagerService::DumpInner(const std::string &args, std::vector<std::string> &info)
{
    currentStackManager_->Dump(info);
}

void AbilityManagerService::DumpStackListInner(const std::string &args, std::vector<std::string> &info)
{
    currentStackManager_->DumpStackList(info);
}

void AbilityManagerService::DumpFocusMapInner(const std::string &args, std::vector<std::string> &info)
{
    currentStackManager_->DumpFocusMap(info);
}

void AbilityManagerService::DumpStackInner(const std::string &args, std::vector<std::string> &info)
{
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        info.push_back("error: invalid argument, please see 'ability dump -h'.");
        return;
    }
    int stackId = DEFAULT_INVAL_VALUE;
    (void)StrToInt(argList[1], stackId);
    currentStackManager_->DumpStack(stackId, info);
}

void AbilityManagerService::DumpMissionInner(const std::string &args, std::vector<std::string> &info)
{
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        info.push_back("error: invalid argument, please see 'ability dump -h'.");
        return;
    }
    int missionId = DEFAULT_INVAL_VALUE;
    (void)StrToInt(argList[1], missionId);
    currentStackManager_->DumpMission(missionId, info);
}

void AbilityManagerService::DumpTopAbilityInner(const std::string &args, std::vector<std::string> &info)
{
    currentStackManager_->DumpTopAbility(info);
}

void AbilityManagerService::DumpWaittingAbilityQueueInner(const std::string &args, std::vector<std::string> &info)
{
    std::string result;
    DumpWaittingAbilityQueue(result);
    info.push_back(result);
}

void AbilityManagerService::DumpStateInner(const std::string &args, std::vector<std::string> &info)
{
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    if (argList.size() == MIN_DUMP_ARGUMENT_NUM) {
        connectManager_->DumpState(info, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        connectManager_->DumpState(info);
    } else {
        info.emplace_back("error: invalid argument, please see 'ability dump -h'.");
    }
}

void AbilityManagerService::DataDumpStateInner(const std::string &args, std::vector<std::string> &info)
{
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    if (argList.size() == MIN_DUMP_ARGUMENT_NUM) {
        dataAbilityManager_->DumpState(info, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        dataAbilityManager_->DumpState(info);
    } else {
        info.emplace_back("error: invalid argument, please see 'ability dump -h'.");
    }
}

void AbilityManagerService::SystemDumpStateInner(const std::string &args, std::vector<std::string> &info)
{
    systemAppManager_->DumpState(info);
}

void AbilityManagerService::DumpState(const std::string &args, std::vector<std::string> &info)
{
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    auto it = dumpMap.find(argList[0]);
    if (it == dumpMap.end()) {
        return;
    }
    DumpKey key = it->second;
    auto itFunc = dumpFuncMap_.find(key);
    if (itFunc != dumpFuncMap_.end()) {
        auto dumpFunc = itFunc->second;
        if (dumpFunc != nullptr) {
            (this->*dumpFunc)(args, info);
            return;
        }
    }
    info.push_back("error: invalid argument, please see 'ability dump -h'.");
}

int AbilityManagerService::AbilityTransitionDone(const sptr<IRemoteObject> &token, int state)
{
    HILOG_INFO("Ability transition done, state:%{public}d", state);
    if (!VerificationToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN_LOG(abilityRecord, ERR_INVALID_VALUE, "Ability record is nullptr.");

    auto abilityInfo = abilityRecord->GetAbilityInfo();
    HILOG_DEBUG("state:%{public}d  name:%{public}s", state, abilityInfo.name.c_str());
    auto type = abilityInfo.type;
    if (type == AppExecFwk::AbilityType::SERVICE) {
        return connectManager_->AbilityTransitionDone(token, state);
    }
    if (type == AppExecFwk::AbilityType::DATA) {
        return dataAbilityManager_->AbilityTransitionDone(token, state);
    }
    if (IsSystemUiApp(abilityInfo)) {
        return systemAppManager_->AbilityTransitionDone(token, state);
    }

    return currentStackManager_->AbilityTransitionDone(token, state);
}

int AbilityManagerService::ScheduleConnectAbilityDone(
    const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject)
{
    HILOG_INFO("Schedule connect ability done.");
    if (!VerificationToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::SERVICE) {
        HILOG_ERROR("Connect ability failed, target ability is not service.");
        return TARGET_ABILITY_NOT_SERVICE;
    }

    return connectManager_->ScheduleConnectAbilityDoneLocked(token, remoteObject);
}

int AbilityManagerService::ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Schedule disconnect ability done.");
    if (!VerificationToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::SERVICE) {
        HILOG_ERROR("Connect ability failed, target ability is not service.");
        return TARGET_ABILITY_NOT_SERVICE;
    }

    return connectManager_->ScheduleDisconnectAbilityDoneLocked(token);
}

int AbilityManagerService::ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Schedule command ability done.");
    if (!VerificationToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::SERVICE) {
        HILOG_ERROR("Connect ability failed, target ability is not service.");
        return TARGET_ABILITY_NOT_SERVICE;
    }

    return connectManager_->ScheduleCommandAbilityDoneLocked(token);
}

void AbilityManagerService::AddWindowInfo(const sptr<IRemoteObject> &token, int32_t windowToken)
{
    HILOG_DEBUG("Add window id.");
    if (!VerificationToken(token)) {
        return;
    }
    currentStackManager_->AddWindowInfo(token, windowToken);
}

void AbilityManagerService::OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state)
{
    HILOG_INFO("On ability request done.");
    if (!VerificationToken(token)) {
        return;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER(abilityRecord);

    auto type = abilityRecord->GetAbilityInfo().type;
    switch (type) {
        case AppExecFwk::AbilityType::SERVICE:
            connectManager_->OnAbilityRequestDone(token, state);
            break;
        case AppExecFwk::AbilityType::DATA:
            dataAbilityManager_->OnAbilityRequestDone(token, state);
            break;
        default: {
            if (IsSystemUiApp(abilityRecord->GetAbilityInfo())) {
                systemAppManager_->OnAbilityRequestDone(token, state);
                break;
            }
            currentStackManager_->OnAbilityRequestDone(token, state);
            break;
        }
    }
}

void AbilityManagerService::OnAppStateChanged(const AppInfo &info)
{
    HILOG_INFO("On app state changed.");
    currentStackManager_->OnAppStateChanged(info);
    connectManager_->OnAppStateChanged(info);
    systemAppManager_->OnAppStateChanged(info);
    dataAbilityManager_->OnAppStateChanged(info);
}

std::shared_ptr<AbilityEventHandler> AbilityManagerService::GetEventHandler()
{
    return handler_;
}

void AbilityManagerService::SetStackManager(int userId)
{
    auto iterator = stackManagers_.find(userId);
    if (iterator != stackManagers_.end()) {
        currentStackManager_ = iterator->second;
    } else {
        currentStackManager_ = std::make_shared<AbilityStackManager>(userId);
        currentStackManager_->Init();
        stackManagers_.emplace(userId, currentStackManager_);
    }
}

std::shared_ptr<AbilityStackManager> AbilityManagerService::GetStackManager()
{
    return currentStackManager_;
}

void AbilityManagerService::DumpWaittingAbilityQueue(std::string &result)
{
    currentStackManager_->DumpWaittingAbilityQueue(result);
    return;
}

// multi user scene
int AbilityManagerService::GetUserId()
{
    return DEFAULT_USER_ID;
}

void AbilityManagerService::StartingLauncherAbility()
{
    HILOG_DEBUG("%{public}s", __func__);
    if (!iBundleManager_) {
        HILOG_INFO("bms service is null");
    }

    /* query if launcher ability has installed */
    AppExecFwk::AbilityInfo abilityInfo;
    /* First stage, hardcoding for the first launcher App */
    Want want;
    want.SetElementName(AbilityConfig::LAUNCHER_BUNDLE_NAME, AbilityConfig::LAUNCHER_ABILITY_NAME);
    while (!(iBundleManager_->QueryAbilityInfo(want, abilityInfo))) {
        HILOG_INFO("Waiting query launcher ability info completed.");
        usleep(REPOLL_TIME_MICRO_SECONDS);
    }

    HILOG_INFO("waiting boot animation for 5 seconds.");
    usleep(REPOLL_TIME_MICRO_SECONDS * WAITING_BOOT_ANIMATION_TIMER);
    HILOG_INFO("Start Home Launcher Ability.");
    /* start launch ability */
    (void)StartAbility(want, DEFAULT_INVAL_VALUE);
    return;
}

void AbilityManagerService::StartSystemUi(const std::string abilityName)
{
    HILOG_INFO("Starting system ui app.");
    Want want;
    want.SetElementName(AbilityConfig::SYSTEM_UI_BUNDLE_NAME, abilityName);
    HILOG_INFO("Ability name: %{public}s.", abilityName.c_str());
    (void)StartAbility(want, DEFAULT_INVAL_VALUE);
    return;
}

int AbilityManagerService::GenerateAbilityRequest(
    const Want &want, int requestCode, AbilityRequest &request, const sptr<IRemoteObject> &callerToken)
{
    request.want = want;
    request.requestCode = requestCode;
    request.callerToken = callerToken;
    request.startSetting = nullptr;

    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, GET_ABILITY_SERVICE_FAILED);

    bms->QueryAbilityInfo(want, request.abilityInfo);
    if (request.abilityInfo.name.empty() || request.abilityInfo.bundleName.empty()) {
        HILOG_ERROR("Get ability info failed.");
        return RESOLVE_ABILITY_ERR;
    }
    HILOG_DEBUG("Query ability name: %{public}s,", request.abilityInfo.name.c_str());

    request.appInfo = request.abilityInfo.applicationInfo;
    if (request.appInfo.name.empty() || request.appInfo.bundleName.empty()) {
        HILOG_ERROR("Get app info failed.");
        return RESOLVE_APP_ERR;
    }
    HILOG_DEBUG("Query app name: %{public}s,", request.appInfo.name.c_str());
    return ERR_OK;
}

int AbilityManagerService::GetAllStackInfo(StackInfo &stackInfo)
{
    HILOG_DEBUG("Get all stack info.");
    CHECK_POINTER_AND_RETURN(currentStackManager_, ERR_NO_INIT);
    currentStackManager_->GetAllStackInfo(stackInfo);
    return ERR_OK;
}

int AbilityManagerService::TerminateAbilityResult(const sptr<IRemoteObject> &token, int startId)
{
    HILOG_INFO("Terminate ability result, startId: %{public}d", startId);
    if (!VerificationToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    int result = AbilityUtil::JudgeAbilityVisibleControl(abilityRecord->GetAbilityInfo());
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return result;
    }

    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::SERVICE) {
        HILOG_ERROR("target ability is not service.");
        return TARGET_ABILITY_NOT_SERVICE;
    }

    return connectManager_->TerminateAbilityResult(token, startId);
}

int AbilityManagerService::StopServiceAbility(const Want &want)
{
    HILOG_DEBUG("Stop service ability.");
    AbilityRequest abilityRequest;
    int result = GenerateAbilityRequest(want, DEFAULT_INVAL_VALUE, abilityRequest, nullptr);
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request error.");
        return result;
    }
    auto abilityInfo = abilityRequest.abilityInfo;
    auto type = abilityInfo.type;
    if (type != AppExecFwk::AbilityType::SERVICE) {
        HILOG_ERROR("Target ability is not service type.");
        return TARGET_ABILITY_NOT_SERVICE;
    }
    return connectManager_->StopServiceAbility(abilityRequest);
}

void AbilityManagerService::OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord)
{
    CHECK_POINTER(abilityRecord);

    if (systemAppManager_ && abilityRecord->IsKernalSystemAbility()) {
        systemAppManager_->OnAbilityDied(abilityRecord);
        return;
    }

    if (currentStackManager_) {
        currentStackManager_->OnAbilityDied(abilityRecord);
    }

    if (connectManager_) {
        connectManager_->OnAbilityDied(abilityRecord);
    }

    if (dataAbilityManager_) {
        dataAbilityManager_->OnAbilityDied(abilityRecord);
    }
}

int AbilityManagerService::KillProcess(const std::string &bundleName)
{
    HILOG_DEBUG("Kill process, bundleName: %{public}s", bundleName.c_str());
    int ret = DelayedSingleton<AppScheduler>::GetInstance()->KillApplication(bundleName);
    if (ret != ERR_OK) {
        return KILL_PROCESS_FAILED;
    }
    return ERR_OK;
}

int AbilityManagerService::UninstallApp(const std::string &bundleName)
{
    HILOG_DEBUG("Uninstall app, bundleName: %{public}s", bundleName.c_str());
    CHECK_POINTER_AND_RETURN(currentStackManager_, ERR_NO_INIT);
    currentStackManager_->UninstallApp(bundleName);
    int ret = DelayedSingleton<AppScheduler>::GetInstance()->KillApplication(bundleName);
    if (ret != ERR_OK) {
        return UNINSTALL_APP_FAILED;
    }
    return ERR_OK;
}

sptr<AppExecFwk::IBundleMgr> AbilityManagerService::GetBundleManager()
{
    if (iBundleManager_ == nullptr) {
        auto bundleObj =
            OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (bundleObj == nullptr) {
            HILOG_ERROR("Failed to get bundle manager service.");
            return nullptr;
        }
        iBundleManager_ = iface_cast<AppExecFwk::IBundleMgr>(bundleObj);
    }

    return iBundleManager_;
}

int AbilityManagerService::PreLoadAppDataAbilities(const std::string &bundleName)
{
    if (bundleName.empty()) {
        HILOG_ERROR("Invalid bundle name when app data abilities preloading.");
        return ERR_INVALID_VALUE;
    }

    if (dataAbilityManager_ == nullptr) {
        HILOG_ERROR("Invalid data ability manager when app data abilities preloading.");
        return ERR_INVALID_STATE;
    }

    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, GET_ABILITY_SERVICE_FAILED);

    AppExecFwk::BundleInfo bundleInfo;
    bool ret = bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo);
    if (!ret) {
        HILOG_ERROR("Failed to get bundle info when app data abilities preloading.");
        return RESOLVE_APP_ERR;
    }

    HILOG_INFO("App data abilities preloading for bundle '%{public}s'...", bundleName.data());

    auto begin = system_clock::now();
    AbilityRequest dataAbilityRequest;
    dataAbilityRequest.appInfo = bundleInfo.applicationInfo;
    for (auto it = bundleInfo.abilityInfos.begin(); it != bundleInfo.abilityInfos.end(); ++it) {
        if (it->type != AppExecFwk::AbilityType::DATA) {
            continue;
        }
        if ((system_clock::now() - begin) >= DATA_ABILITY_START_TIMEOUT) {
            HILOG_ERROR("App data ability preloading for '%{public}s' timeout.", bundleName.c_str());
            return ERR_TIMED_OUT;
        }
        dataAbilityRequest.abilityInfo = *it;
        HILOG_INFO("App data ability preloading: '%{public}s.%{public}s'...", it->bundleName.c_str(), it->name.c_str());

        auto dataAbility = dataAbilityManager_->Acquire(dataAbilityRequest, false, nullptr, false);
        if (dataAbility == nullptr) {
            HILOG_ERROR(
                "Failed to preload data ability '%{public}s.%{public}s'.", it->bundleName.c_str(), it->name.c_str());
            return ERR_NULL_OBJECT;
        }
    }

    HILOG_INFO("App data abilities preloading done.");

    return ERR_OK;
}

bool AbilityManagerService::IsSystemUiApp(const AppExecFwk::AbilityInfo &info) const
{
    if (info.bundleName != AbilityConfig::SYSTEM_UI_BUNDLE_NAME) {
        return false;
    }
    return (info.name == AbilityConfig::SYSTEM_UI_NAVIGATION_BAR || info.name == AbilityConfig::SYSTEM_UI_STATUS_BAR);
}

void AbilityManagerService::HandleLoadTimeOut(int64_t eventId)
{
    HILOG_DEBUG("Handle load timeout.");
    if (systemAppManager_) {
        systemAppManager_->OnTimeOut(AbilityManagerService::LOAD_TIMEOUT_MSG, eventId);
    }
    if (currentStackManager_) {
        currentStackManager_->OnTimeOut(AbilityManagerService::LOAD_TIMEOUT_MSG, eventId);
    }
}

void AbilityManagerService::HandleActiveTimeOut(int64_t eventId)
{
    HILOG_DEBUG("Handle active timeout.");
    if (systemAppManager_) {
        systemAppManager_->OnTimeOut(AbilityManagerService::ACTIVE_TIMEOUT_MSG, eventId);
    }
    if (currentStackManager_) {
        currentStackManager_->OnTimeOut(AbilityManagerService::ACTIVE_TIMEOUT_MSG, eventId);
    }
}

void AbilityManagerService::HandleInactiveTimeOut(int64_t eventId)
{
    HILOG_DEBUG("Handle inactive timeout.");
    if (currentStackManager_) {
        currentStackManager_->OnTimeOut(AbilityManagerService::INACTIVE_TIMEOUT_MSG, eventId);
    }
}

bool AbilityManagerService::VerificationToken(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Verification token.");
    CHECK_POINTER_RETURN_BOOL(currentStackManager_);
    CHECK_POINTER_RETURN_BOOL(dataAbilityManager_);
    CHECK_POINTER_RETURN_BOOL(connectManager_);

    if (currentStackManager_->GetAbilityRecordByToken(token)) {
        return true;
    }

    if (currentStackManager_->GetAbilityFromTerminateList(token)) {
        return true;
    }

    if (dataAbilityManager_->GetAbilityRecordByToken(token)) {
        return true;
    }

    if (connectManager_->GetServiceRecordByToken(token)) {
        return true;
    }

    if (systemAppManager_->GetAbilityRecordByToken(token)) {
        return true;
    }

    HILOG_ERROR("Failed to verify token.");
    return false;
}

int AbilityManagerService::MoveMissionToFloatingStack(const MissionOption &missionOption)
{
    HILOG_INFO("Move mission to floating stack.");
    return currentStackManager_->MoveMissionToFloatingStack(missionOption);
}

int AbilityManagerService::MoveMissionToSplitScreenStack(const MissionOption &missionOption)
{
    HILOG_INFO("Move mission to split screen stack.");
    return currentStackManager_->MoveMissionToSplitScreenStack(missionOption);
}

int AbilityManagerService::ChangeFocusAbility(
    const sptr<IRemoteObject> &lostFocusToken, const sptr<IRemoteObject> &getFocusToken)
{
    HILOG_INFO("Change focus ability.");
    CHECK_POINTER_AND_RETURN(lostFocusToken, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(getFocusToken, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(currentStackManager_, ERR_INVALID_VALUE);

    if (!VerificationToken(lostFocusToken)) {
        return ERR_INVALID_VALUE;
    }

    if (!VerificationToken(getFocusToken)) {
        return ERR_INVALID_VALUE;
    }

    return currentStackManager_->ChangeFocusAbility(lostFocusToken, getFocusToken);
}

int AbilityManagerService::MinimizeMultiWindow(int missionId)
{
    HILOG_INFO("Minimize multi window.");
    return currentStackManager_->MinimizeMultiWindow(missionId);
}

int AbilityManagerService::MaximizeMultiWindow(int missionId)
{
    HILOG_INFO("Maximize multi window.");
    return currentStackManager_->MaximizeMultiWindow(missionId);
}

int AbilityManagerService::GetFloatingMissions(std::vector<AbilityMissionInfo> &list)
{
    HILOG_INFO("Get floating missions.");
    return currentStackManager_->GetFloatingMissions(list);
}

int AbilityManagerService::CloseMultiWindow(int missionId)
{
    HILOG_INFO("Close multi window.");
    CHECK_POINTER_AND_RETURN(currentStackManager_, ERR_INVALID_VALUE);
    return currentStackManager_->CloseMultiWindow(missionId);
}

int AbilityManagerService::SetMissionStackSetting(const StackSetting &stackSetting)
{
    HILOG_INFO("Set mission stack setting.");
    currentStackManager_->SetMissionStackSetting(stackSetting);
    return ERR_OK;
}

bool AbilityManagerService::IsFirstInMission(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Is first in mission.");
    CHECK_POINTER_RETURN_BOOL(token);
    CHECK_POINTER_RETURN_BOOL(currentStackManager_);

    if (!VerificationToken(token)) {
        return false;
    }

    return currentStackManager_->IsFirstInMission(token);
}

int AbilityManagerService::CompelVerifyPermission(const std::string &permission, int pid, int uid, std::string &message)
{
    HILOG_INFO("Compel verify permission.");
    return DelayedSingleton<AppScheduler>::GetInstance()->CompelVerifyPermission(permission, pid, uid, message);
}

int AbilityManagerService::PowerOff()
{
    HILOG_INFO("Power off.");
    CHECK_POINTER_AND_RETURN(currentStackManager_, ERR_NO_INIT);
    return currentStackManager_->PowerOff();
}

int AbilityManagerService::PowerOn()
{
    HILOG_INFO("Power on.");
    CHECK_POINTER_AND_RETURN(currentStackManager_, ERR_NO_INIT);
    return currentStackManager_->PowerOn();
}

int AbilityManagerService::LockMission(int missionId)
{
    HILOG_INFO("request lock mission id :%{public}d", missionId);
    CHECK_POINTER_AND_RETURN(currentStackManager_, ERR_NO_INIT);
    CHECK_POINTER_AND_RETURN(iBundleManager_, ERR_NO_INIT);

    int callerUid = IPCSkeleton::GetCallingUid();
    int callerPid = IPCSkeleton::GetCallingPid();
    bool isSystemApp = iBundleManager_->CheckIsSystemAppByUid(callerUid);
    HILOG_DEBUG("locker uid :%{public}d, pid :%{public}d. isSystemApp: %{public}d", callerUid, callerPid, isSystemApp);
    return currentStackManager_->StartLockMission(callerUid, missionId, isSystemApp, true);
}

int AbilityManagerService::UnlockMission(int missionId)
{
    HILOG_INFO("request unlock mission id :%{public}d", missionId);
    CHECK_POINTER_AND_RETURN(currentStackManager_, ERR_NO_INIT);
    CHECK_POINTER_AND_RETURN(iBundleManager_, ERR_NO_INIT);

    int callerUid = IPCSkeleton::GetCallingUid();
    int callerPid = IPCSkeleton::GetCallingPid();
    bool isSystemApp = iBundleManager_->CheckIsSystemAppByUid(callerUid);
    HILOG_DEBUG("locker uid :%{public}d, pid :%{public}d. isSystemApp: %{public}d", callerUid, callerPid, isSystemApp);
    return currentStackManager_->StartLockMission(callerUid, missionId, isSystemApp, false);
}

int AbilityManagerService::GetUidByBundleName(std::string bundleName)
{
    CHECK_POINTER_AND_RETURN(iBundleManager_, -1);
    return iBundleManager_->GetUidByBundleName(bundleName, GetUserId());
}

void AbilityManagerService::RestartAbility(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("%{public}s called", __func__);
    CHECK_POINTER(currentStackManager_);
    CHECK_POINTER(systemAppManager_);
    if (!VerificationToken(token)) {
        return;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER(abilityRecord);

    if (IsSystemUiApp(abilityRecord->GetAbilityInfo())) {
        systemAppManager_->RestartAbility(abilityRecord);
        return;
    }

    currentStackManager_->RestartAbility(abilityRecord);
}

void AbilityManagerService::NotifyBmsAbilityLifeStatus(
    const std::string &bundleName, const std::string &abilityName, const int64_t launchTime)
{
    auto bundleManager = GetBundleManager();
    CHECK_POINTER(bundleManager);
    bundleManager->NotifyActivityLifeStatus(bundleName, abilityName, launchTime);
}

void AbilityManagerService::StartSystemApplication()
{
    HILOG_DEBUG("%{public}s", __func__);

    ConnectBmsService();

    if (!amsConfigResolver_ || amsConfigResolver_->NonConfigFile()) {
        HILOG_INFO("start all");
        StartingLauncherAbility();
        StartingSystemUiAbility(SatrtUiMode::STARTUIBOTH);
        return;
    }

    if (amsConfigResolver_->GetStartLauncherState()) {
        HILOG_INFO("start launcher");
        StartingLauncherAbility();
    }

    if (amsConfigResolver_->GetStatusBarState()) {
        HILOG_INFO("start status bar");
        StartingSystemUiAbility(SatrtUiMode::STATUSBAR);
    }

    if (amsConfigResolver_->GetNavigationBarState()) {
        HILOG_INFO("start navigation bar");
        StartingSystemUiAbility(SatrtUiMode::NAVIGATIONBAR);
    }
}

void AbilityManagerService::ConnectBmsService()
{
    HILOG_DEBUG("%{public}s", __func__);
    HILOG_INFO("Waiting AppMgr Service run completed.");
    while (!appScheduler_->Init(shared_from_this())) {
        HILOG_ERROR("failed to init appScheduler_");
        usleep(REPOLL_TIME_MICRO_SECONDS);
    }

    HILOG_INFO("Waiting BundleMgr Service run completed.");
    /* wait until connected to bundle manager service */
    while (iBundleManager_ == nullptr) {
        sptr<IRemoteObject> bundle_obj =
            OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (bundle_obj == nullptr) {
            HILOG_ERROR("failed to get bundle manager service");
            usleep(REPOLL_TIME_MICRO_SECONDS);
            continue;
        }
        iBundleManager_ = iface_cast<AppExecFwk::IBundleMgr>(bundle_obj);
    }

    HILOG_INFO("Connect bms success!");
}

void AbilityManagerService::StartingSystemUiAbility(const SatrtUiMode &mode)
{
    HILOG_DEBUG("%{public}s", __func__);
    if (!iBundleManager_) {
        HILOG_INFO("bms service is null");
    }

    AppExecFwk::AbilityInfo statusBarInfo;
    AppExecFwk::AbilityInfo navigationBarInfo;
    Want statusBarWant;
    Want navigationBarWant;
    statusBarWant.SetElementName(AbilityConfig::SYSTEM_UI_BUNDLE_NAME, AbilityConfig::SYSTEM_UI_STATUS_BAR);
    navigationBarWant.SetElementName(AbilityConfig::SYSTEM_UI_BUNDLE_NAME, AbilityConfig::SYSTEM_UI_NAVIGATION_BAR);
    uint32_t waitCnt = 0;
    // Wait 10 minutes for the installation to complete.
    while ((!(iBundleManager_->QueryAbilityInfo(statusBarWant, statusBarInfo)) ||
            !(iBundleManager_->QueryAbilityInfo(navigationBarWant, navigationBarInfo))) &&
            waitCnt < MAX_WAIT_SYSTEM_UI_NUM) {
        HILOG_INFO("Waiting query system ui info completed.");
        usleep(REPOLL_TIME_MICRO_SECONDS);
        waitCnt++;
    }

    HILOG_INFO("start ui mode : %{public}d", mode);
    switch (mode) {
        case SatrtUiMode::STATUSBAR:
            StartSystemUi(AbilityConfig::SYSTEM_UI_STATUS_BAR);
            break;
        case SatrtUiMode::NAVIGATIONBAR:
            StartSystemUi(AbilityConfig::SYSTEM_UI_NAVIGATION_BAR);
            break;
        case SatrtUiMode::STARTUIBOTH:
            StartSystemUi(AbilityConfig::SYSTEM_UI_STATUS_BAR);
            StartSystemUi(AbilityConfig::SYSTEM_UI_NAVIGATION_BAR);
            break;
        default:
            HILOG_INFO("Input mode error ...");
            break;
    }
}

bool AbilityManagerService::CheckCallerIsSystemAppByIpc()
{
    HILOG_DEBUG("%{public}s begin", __func__);
    auto bms = GetBundleManager();
    CHECK_POINTER_RETURN_BOOL(bms);
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    HILOG_ERROR("callerUid %{public}d", callerUid);
    return bms->CheckIsSystemAppByUid(callerUid);
}
}  // namespace AAFwk
}  // namespace OHOS
