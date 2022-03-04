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

#include "ability_manager_service.h"
#include "accesstoken_kit.h"

#include <fstream>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <csignal>
#include <cstdlib>

#include "ability_info.h"
#include "ability_manager_errors.h"
#include "ability_util.h"
#include "bytrace.h"
#include "bundle_mgr_client.h"
#include "distributed_client.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "itest_observer.h"
#include "locale_config.h"
#include "lock_screen_white_list.h"
#include "mission/mission_info_converter.h"
#include "mission_info_mgr.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "sa_mgr_client.h"
#include "softbus_bus_center.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "os_account_manager.h"
#include "png.h"
#include "ui_service_mgr_client.h"
#include "uri_permission_manager_client.h"
#include "xcollie/watchdog.h"
#include "parameter.h"

using OHOS::AppExecFwk::ElementName;
using OHOS::Security::AccessToken::AccessTokenKit;

namespace OHOS {
namespace AAFwk {
using namespace std::chrono;
const bool CONCURRENCY_MODE_FALSE = false;
const int32_t MAIN_USER_ID = 100;
const int32_t U0_USER_ID = 0;
constexpr int32_t INVALID_USER_ID = -1;
static const int EXPERIENCE_MEM_THRESHOLD = 20;
constexpr auto DATA_ABILITY_START_TIMEOUT = 5s;
constexpr int32_t NON_ANONYMIZE_LENGTH = 6;
constexpr uint32_t SCENE_FLAG_NORMAL = 0;
const int32_t MAX_NUMBER_OF_DISTRIBUTED_MISSIONS = 20;
const int32_t MAX_NUMBER_OF_CONNECT_BMS = 15;
const std::string EMPTY_DEVICE_ID = "";
const int32_t APP_MEMORY_SIZE = 512;
const int32_t GET_PARAMETER_INCORRECT = -9;
const int32_t GET_PARAMETER_OTHER = -1;
const int32_t SIZE_10 = -1;
const bool isRamConstrainedDevice = false;
const std::string APP_MEMORY_MAX_SIZE_PARAMETER = "const.product.dalvikheaplimit";
const std::string RAM_CONSTRAINED_DEVICE_SIGN = "const.product.islowram";
const std::string PKG_NAME = "ohos.distributedhardware.devicemanager";
const std::string ACTION_CHOOSE = "ohos.want.action.select";
const std::string PERMISSION_SET_ABILITY_CONTROLLER = "ohos.permission.SET_ABILITY_CONTROLLER";
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
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-focus", KEY_DUMP_FOCUS_ABILITY),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-f", KEY_DUMP_FOCUS_ABILITY),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--win-mode", KEY_DUMP_WINDOW_MODE),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-z", KEY_DUMP_WINDOW_MODE),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--mission-list", KEY_DUMP_MISSION_LIST),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-L", KEY_DUMP_MISSION_LIST),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--mission-infos", KEY_DUMP_MISSION_INFOS),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-S", KEY_DUMP_MISSION_INFOS),
};

const std::map<std::string, AbilityManagerService::DumpsysKey> AbilityManagerService::dumpsysMap = {
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("--all", KEY_DUMPSYS_ALL),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("-a", KEY_DUMPSYS_ALL),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("--mission-list", KEY_DUMPSYS_MISSION_LIST),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("-l", KEY_DUMPSYS_MISSION_LIST),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("--ability", KEY_DUMPSYS_ABILITY),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("-i", KEY_DUMPSYS_ABILITY),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("--extension", KEY_DUMPSYS_SERVICE),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("-e", KEY_DUMPSYS_SERVICE),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("--pending", KEY_DUMPSYS_PENDING),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("-p", KEY_DUMPSYS_PENDING),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("--process", KEY_DUMPSYS_PROCESS),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("-r", KEY_DUMPSYS_PROCESS),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("--data", KEY_DUMPSYS_DATA),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("-d", KEY_DUMPSYS_DATA),
};

const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<AbilityManagerService>::GetInstance().get());

AbilityManagerService::AbilityManagerService()
    : SystemAbility(ABILITY_MGR_SERVICE_ID, true),
      eventLoop_(nullptr),
      handler_(nullptr),
      state_(ServiceRunningState::STATE_NOT_START),
      iBundleManager_(nullptr)
{
    std::shared_ptr<AppScheduler> appScheduler(
        DelayedSingleton<AppScheduler>::GetInstance().get(), [](AppScheduler *x) { x->DecStrongRef(x); });
    appScheduler_ = appScheduler;
    DumpFuncInit();
    DumpSysFuncInit();
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

    // init user controller.
    userController_ = std::make_shared<UserController>();
    userController_->Init();
    int userId = MAIN_USER_ID;

    InitConnectManager(userId, true);
    InitDataAbilityManager(userId, true);
    InitPendWantManager(userId, true);
    systemDataAbilityManager_ = std::make_shared<DataAbilityManager>();

    amsConfigResolver_ = std::make_shared<AmsConfigurationParameter>();
    if (amsConfigResolver_) {
        amsConfigResolver_->Parse();
        HILOG_INFO("ams config parse");
    }
    useNewMission_ = amsConfigResolver_->IsUseNewMission();

    SetStackManager(userId, true);

    InitMissionListManager(userId, true);

    SwitchManagers(U0_USER_ID, false);
    int amsTimeOut = amsConfigResolver_->GetAMSTimeOutTime();
    if (HiviewDFX::Watchdog::GetInstance().AddThread("AMSWatchdog", handler_, amsTimeOut) != 0) {
        HILOG_ERROR("HiviewDFX::Watchdog::GetInstance AddThread Fail");
    }

    auto startSystemTask = [aams = shared_from_this()]() { aams->StartSystemApplication(); };
    handler_->PostTask(startSystemTask, "startLauncherAbility");
    auto creatWhiteListTask = [aams = shared_from_this()]() {
        if (access(AmsWhiteList::AMS_WHITE_LIST_DIR_PATH.c_str(), F_OK) != 0) {
            if (mkdir(AmsWhiteList::AMS_WHITE_LIST_DIR_PATH.c_str(), S_IRWXO | S_IRWXG | S_IRWXU)) {
                HILOG_ERROR("mkdir AmsWhiteList::AMS_WHITE_LIST_DIR_PATH Fail");
                return;
            }
        }
        if (aams->IsExistFile(AmsWhiteList::AMS_WHITE_LIST_FILE_PATH)) {
            HILOG_INFO("file exists");
            return;
        }
        HILOG_INFO("no such file,create...");
        std::ofstream outFile(AmsWhiteList::AMS_WHITE_LIST_FILE_PATH, std::ios::out);
        outFile.close();
    };
    handler_->PostTask(creatWhiteListTask, "creatWhiteList");
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

int AbilityManagerService::StartAbility(const Want &want, int32_t userId, int requestCode)
{
    HILOG_INFO("%{public}s coldStart:%{public}d", __func__, want.GetBoolParam("coldStart", false));
    return StartAbilityInner(want, nullptr, requestCode, -1, userId);
}

int AbilityManagerService::StartAbility(const Want &want, const sptr<IRemoteObject> &callerToken,
    int32_t userId, int requestCode)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto flags = want.GetFlags();
    if ((flags & Want::FLAG_ABILITY_CONTINUATION) == Want::FLAG_ABILITY_CONTINUATION) {
        HILOG_ERROR("StartAbility with continuation flags is not allowed!");
        return ERR_INVALID_VALUE;
    }
    HILOG_INFO("%{public}s", __func__);
    if (CheckIfOperateRemote(want)) {
        HILOG_INFO("AbilityManagerService::StartAbility. try to StartRemoteAbility");
        return StartRemoteAbility(want, requestCode);
    }
    return StartAbilityInner(want, callerToken, requestCode, -1, userId);
}

int AbilityManagerService::StartAbilityInner(const Want &want, const sptr<IRemoteObject> &callerToken,
    int requestCode, int callerUid, int32_t userId)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);

    if (VerifyAccountPermission(userId) == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    HILOG_DEBUG("%{public}s begin.", __func__);
    if (callerToken != nullptr && !VerificationAllToken(callerToken)) {
        HILOG_ERROR("%{public}s VerificationAllToken failed.", __func__);
        return ERR_INVALID_VALUE;
    }

    int32_t validUserId = GetValidUserId(userId);

    AbilityRequest abilityRequest;
    auto result = GenerateAbilityRequest(want, requestCode, abilityRequest, callerToken, validUserId);
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request local error.");
        return result;
    }

    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleUser ? U0_USER_ID : validUserId;
    HILOG_DEBUG("userId : %{public}d, singleUser is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleUser));

    if (!JudgeMultiUserConcurrency(abilityRequest.abilityInfo, validUserId)) {
        HILOG_ERROR("Multi-user non-concurrent mode is not satisfied.");
        return ERR_INVALID_VALUE;
    }

    result = CheckStaticCfgPermission(abilityInfo);
    if (result != AppExecFwk::Constants::PERMISSION_GRANTED) {
        return result;
    }
    GrantUriPermission(want, validUserId);
    result = AbilityUtil::JudgeAbilityVisibleControl(abilityInfo, callerUid);
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return result;
    }
    auto type = abilityInfo.type;
    HILOG_DEBUG("%{public}s Current ability type:%{public}d", __func__, type);
    if (type == AppExecFwk::AbilityType::DATA) {
        HILOG_ERROR("Cannot start data ability, use 'AcquireDataAbility()' instead.");
        return ERR_INVALID_VALUE;
    }
    if (!AbilityUtil::IsSystemDialogAbility(abilityInfo.bundleName, abilityInfo.name)) {
        HILOG_DEBUG("%{public}s PreLoadAppDataAbilities:%{public}s", __func__, abilityInfo.bundleName.c_str());
        result = PreLoadAppDataAbilities(abilityInfo.bundleName, validUserId);
        if (result != ERR_OK) {
            HILOG_ERROR("StartAbility: App data ability preloading failed, '%{public}s', %{public}d",
                abilityInfo.bundleName.c_str(), result);
            return result;
        }
    }
    UpdateCallerInfo(abilityRequest.want);
    if (type == AppExecFwk::AbilityType::SERVICE || type == AppExecFwk::AbilityType::EXTENSION) {
        auto connectManager = GetConnectManagerByUserId(validUserId);
        if (!connectManager) {
            HILOG_ERROR("connectManager is nullptr. userId=%{public}d", validUserId);
            return ERR_INVALID_VALUE;
        }
        HILOG_DEBUG("%{public}s Start SERVICE or EXTENSION", __func__);
        return connectManager->StartAbility(abilityRequest);
    }

    if (!IsAbilityControllerStart(want, abilityInfo.bundleName)) {
        HILOG_ERROR("IsAbilityControllerStart failed: %{public}s", abilityInfo.bundleName.c_str());
        return ERR_WOULD_BLOCK;
    }

    if (useNewMission_) {
        auto missionListManager = GetListManagerByUserId(validUserId);
        if (missionListManager == nullptr) {
            HILOG_ERROR("missionListManager is nullptr. userId=%{public}d", validUserId);
            return ERR_INVALID_VALUE;
        }
        HILOG_DEBUG("%{public}s StartAbility by MissionList", __func__);
        return missionListManager->StartAbility(abilityRequest);
    } else {
        auto stackManager = GetStackManagerByUserId(validUserId);
        if (!stackManager) {
            HILOG_ERROR("stackManager is nullptr. userId=%{public}d", validUserId);
            return ERR_INVALID_VALUE;
        }
        HILOG_DEBUG("%{public}s StartAbility by StackManager", __func__);
        return stackManager->StartAbility(abilityRequest);
    }
}

int AbilityManagerService::StartAbility(const Want &want, const AbilityStartSetting &abilityStartSetting,
    const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Start ability setting.");

    if (VerifyAccountPermission(userId) == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (callerToken != nullptr && !VerificationAllToken(callerToken)) {
        return ERR_INVALID_VALUE;
    }

    int32_t validUserId = GetValidUserId(userId);

    AbilityRequest abilityRequest;
    auto result = GenerateAbilityRequest(want, requestCode, abilityRequest, callerToken, validUserId);
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request local error.");
        return result;
    }
    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleUser ? U0_USER_ID : validUserId;
    HILOG_DEBUG("userId : %{public}d, singleUser is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleUser));

    if (!JudgeMultiUserConcurrency(abilityRequest.abilityInfo, validUserId)) {
        HILOG_ERROR("Multi-user non-concurrent mode is not satisfied.");
        return ERR_INVALID_VALUE;
    }

    result = CheckStaticCfgPermission(abilityInfo);
    if (result != AppExecFwk::Constants::PERMISSION_GRANTED) {
        return result;
    }
    result = AbilityUtil::JudgeAbilityVisibleControl(abilityInfo);
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return result;
    }

    abilityRequest.startSetting = std::make_shared<AbilityStartSetting>(abilityStartSetting);

    if (abilityInfo.type == AppExecFwk::AbilityType::DATA) {
        HILOG_ERROR("Cannot start data ability, use 'AcquireDataAbility()' instead.");
        return ERR_INVALID_VALUE;
    }

    if (!AbilityUtil::IsSystemDialogAbility(abilityInfo.bundleName, abilityInfo.name)) {
        result = PreLoadAppDataAbilities(abilityInfo.bundleName, validUserId);
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

    if (!IsAbilityControllerStart(want, abilityInfo.bundleName)) {
        return ERR_WOULD_BLOCK;
    }
    if (useNewMission_) {
        auto missionListManager = GetListManagerByUserId(validUserId);
        if (missionListManager == nullptr) {
            HILOG_ERROR("missionListManager is Null. userId=%{public}d", validUserId);
            return ERR_INVALID_VALUE;
        }
        return missionListManager->StartAbility(abilityRequest);
    } else {
        auto stackManager = GetStackManagerByUserId(validUserId);
        if (!stackManager) {
            HILOG_ERROR("stackManager is nullptr. userId=%{public}d", validUserId);
            return ERR_INVALID_VALUE;
        }
        return stackManager->StartAbility(abilityRequest);
    }
}

int AbilityManagerService::StartAbility(const Want &want, const StartOptions &startOptions,
    const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Start ability options.");

    if (VerifyAccountPermission(userId) == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (callerToken != nullptr && !VerificationAllToken(callerToken)) {
        return ERR_INVALID_VALUE;
    }

    int32_t validUserId = GetValidUserId(userId);

    AbilityRequest abilityRequest;
    auto result = GenerateAbilityRequest(want, requestCode, abilityRequest, callerToken, validUserId);
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request local error.");
        return result;
    }

    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleUser ? U0_USER_ID : validUserId;
    HILOG_DEBUG("userId : %{public}d, singleUser is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleUser));

    if (!JudgeMultiUserConcurrency(abilityRequest.abilityInfo, validUserId)) {
        HILOG_ERROR("Multi-user non-concurrent mode is not satisfied.");
        return ERR_INVALID_VALUE;
    }

    result = CheckStaticCfgPermission(abilityInfo);
    if (result != AppExecFwk::Constants::PERMISSION_GRANTED) {
        return result;
    }
    result = AbilityUtil::JudgeAbilityVisibleControl(abilityInfo);
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return result;
    }

    if (abilityInfo.type != AppExecFwk::AbilityType::PAGE) {
        HILOG_ERROR("Only support for page type ability.");
        return ERR_INVALID_VALUE;
    }

    if (!AbilityUtil::IsSystemDialogAbility(abilityInfo.bundleName, abilityInfo.name)) {
        result = PreLoadAppDataAbilities(abilityInfo.bundleName, validUserId);
        if (result != ERR_OK) {
            HILOG_ERROR("StartAbility: App data ability preloading failed, '%{public}s', %{public}d",
                abilityInfo.bundleName.c_str(),
                result);
            return result;
        }
    }

    if (!IsAbilityControllerStart(want, abilityInfo.bundleName)) {
        return ERR_WOULD_BLOCK;
    }
    GrantUriPermission(want, validUserId);
    abilityRequest.want.SetParam(Want::PARAM_RESV_DISPLAY_ID, startOptions.GetDisplayID());
    abilityRequest.want.SetParam(Want::PARAM_RESV_WINDOW_MODE, startOptions.GetWindowMode());
    if (useNewMission_) {
        auto missionListManager = GetListManagerByUserId(validUserId);
        if (missionListManager == nullptr) {
            HILOG_ERROR("missionListManager is Null. userId=%{public}d", validUserId);
            return ERR_INVALID_VALUE;
        }
        return missionListManager->StartAbility(abilityRequest);
    } else {
        auto stackManager = GetStackManagerByUserId(validUserId);
        if (!stackManager) {
            HILOG_ERROR("stackManager is nullptr. userId=%{public}d", validUserId);
            return ERR_INVALID_VALUE;
        }
        return stackManager->StartAbility(abilityRequest);
    }
}

void AbilityManagerService::GrantUriPermission(const Want &want, int32_t validUserId)
{
    HILOG_DEBUG("AbilityManagerService::GrantUriPermission is called.");
    auto bms = GetBundleManager();
    if (!bms) {
        HILOG_ERROR("Get bundle manager service failed.");
        return;
    }

    auto bundleName = want.GetBundle();
    AppExecFwk::BundleInfo bundleInfo;
    auto bundleFlag = AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO;
    if (!bms->GetBundleInfo(bundleName, bundleFlag, bundleInfo, validUserId)) {
        HILOG_ERROR("Not found ExtensionAbilityInfo according to the uri.");
        return;
    }

    if (want.GetFlags() & (Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION)) {
        HILOG_INFO("Want to grant r/w permission of the uri");
        auto targetTokenId = bundleInfo.applicationInfo.accessTokenId;
        GrantUriPermission(want, validUserId, targetTokenId);
    }
}

void AbilityManagerService::GrantUriPermission(const Want &want, int32_t validUserId, uint32_t targetTokenId)
{
    auto bms = GetBundleManager();
    if (!bms) {
        HILOG_ERROR("Get bundle manager service failed.");
        return;
    }
    auto uriStr = want.GetUri().ToString();
    auto uriVec = want.GetStringArrayParam(AbilityConfig::PARAMS_STREAM);
    uriVec.emplace_back(uriStr);
    auto upmClient = AAFwk::UriPermissionManagerClient::GetInstance();
    auto fromTokenId = IPCSkeleton::GetCallingTokenID();
    AppExecFwk::ExtensionAbilityInfo info;
    for (auto str : uriVec) {
        if (!bms->QueryExtensionAbilityInfoByUri(str, validUserId, info)) {
            HILOG_WARN("Not found ExtensionAbilityInfo according to the uri.");
            continue;
        }
        if (info.type != AppExecFwk::ExtensionAbilityType::FILESHARE) {
            HILOG_WARN("The upms only open to FILESHARE.");
            continue;
        }
        if (fromTokenId != info.applicationInfo.accessTokenId) {
            HILOG_WARN("Only the uri of this application can be authorized.");
            continue;
        }

        Uri uri(str);
        if (want.GetFlags() & Want::FLAG_AUTH_WRITE_URI_PERMISSION) {
            upmClient->GrantUriPermission(uri, Want::FLAG_AUTH_WRITE_URI_PERMISSION, fromTokenId, targetTokenId);
        } else {
            upmClient->GrantUriPermission(uri, Want::FLAG_AUTH_READ_URI_PERMISSION, fromTokenId, targetTokenId);
        }
    }
}

int AbilityManagerService::TerminateAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant)
{
    return TerminateAbilityWithFlag(token, resultCode, resultWant, true);
}

int AbilityManagerService::CloseAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant)
{
    return TerminateAbilityWithFlag(token, resultCode, resultWant, false);
}

int AbilityManagerService::TerminateAbilityWithFlag(const sptr<IRemoteObject> &token, int resultCode,
    const Want *resultWant, bool flag)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Terminate ability for result: %{public}d", (resultWant != nullptr));
    if (!VerificationAllToken(token)) {
        HILOG_ERROR("%{public}s VerificationAllToken failed.", __func__);
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

    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto type = abilityRecord->GetAbilityInfo().type;
    if (type == AppExecFwk::AbilityType::SERVICE || type == AppExecFwk::AbilityType::EXTENSION) {
        auto connectManager = GetConnectManagerByUserId(userId);
        if (!connectManager) {
            HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        return connectManager->TerminateAbility(token);
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

    if (!IsAbilityControllerForeground(abilityRecord->GetAbilityInfo().bundleName)) {
        return ERR_WOULD_BLOCK;
    }

    if (useNewMission_) {
        auto missionListManager = GetListManagerByUserId(userId);
        if (missionListManager == nullptr) {
            HILOG_ERROR("missionListManager is Null. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        return missionListManager->TerminateAbility(abilityRecord, resultCode, resultWant, flag);
    } else {
        auto stackManager = GetStackManagerByUserId(userId);
        if (!stackManager) {
            HILOG_ERROR("stackManager is nullptr. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        return stackManager->TerminateAbility(token, resultCode, resultWant);
    }
}

int AbilityManagerService::StartRemoteAbility(const Want &want, int requestCode)
{
    HILOG_INFO("%{public}s", __func__);
    want.DumpInfo(0);
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    DistributedClient dmsClient;
    HILOG_INFO("AbilityManagerService::Try to StartRemoteAbility, callerUid = %{public}d", callerUid);
    HILOG_INFO("AbilityManagerService::Try to StartRemoteAbility, AccessTokenID = %{public}d", accessToken);
    int result = dmsClient.StartRemoteAbility(want, callerUid, requestCode, accessToken);
    if (result != ERR_NONE) {
        HILOG_ERROR("AbilityManagerService::StartRemoteAbility failed, result = %{public}d", result);
    }
    return result;
}

bool AbilityManagerService::CheckIsRemote(const std::string& deviceId)
{
    if (deviceId.empty()) {
        HILOG_INFO("CheckIsRemote: deviceId is empty.");
        return false;
    }
    std::string localDeviceId;
    if (!GetLocalDeviceId(localDeviceId)) {
        HILOG_ERROR("CheckIsRemote: get local deviceId failed");
        return false;
    }
    if (localDeviceId == deviceId) {
        HILOG_INFO("CheckIsRemote: deviceId is local.");
        return false;
    }
    HILOG_INFO("CheckIsRemote, deviceId = %{public}s", AnonymizeDeviceId(deviceId).c_str());
    return true;
}

bool AbilityManagerService::CheckIfOperateRemote(const Want &want)
{
    std::string deviceId = want.GetElement().GetDeviceID();
    if (deviceId.empty() || want.GetElement().GetBundleName().empty() ||
        want.GetElement().GetAbilityName().empty()) {
        HILOG_DEBUG("CheckIfOperateRemote: DeviceId or BundleName or GetAbilityName empty");
        return false;
    }
    return CheckIsRemote(deviceId);
}

bool AbilityManagerService::GetLocalDeviceId(std::string& localDeviceId)
{
    auto localNode = std::make_unique<NodeBasicInfo>();
    int32_t errCode = GetLocalNodeDeviceInfo(PKG_NAME.c_str(), localNode.get());
    if (errCode != ERR_OK) {
        HILOG_ERROR("AbilityManagerService::GetLocalNodeDeviceInfo errCode = %{public}d", errCode);
        return false;
    }
    if (localNode != nullptr) {
        localDeviceId = localNode->networkId;
        HILOG_INFO("get local deviceId, deviceId = %{public}s",
            AnonymizeDeviceId(localDeviceId).c_str());
        return true;
    }
    HILOG_ERROR("AbilityManagerService::GetLocalDeviceId localDeviceId null");
    return false;
}

std::string AbilityManagerService::AnonymizeDeviceId(const std::string& deviceId)
{
    if (deviceId.length() < NON_ANONYMIZE_LENGTH) {
        return EMPTY_DEVICE_ID;
    }
    std::string anonDeviceId = deviceId.substr(0, NON_ANONYMIZE_LENGTH);
    anonDeviceId.append("******");
    return anonDeviceId;
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
    if (!VerificationAllToken(callerToken)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (IsSystemUiApp(abilityRecord->GetAbilityInfo())) {
        HILOG_ERROR("System ui not allow terminate.");
        return ERR_INVALID_VALUE;
    }

    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto type = abilityRecord->GetAbilityInfo().type;
    auto stackManager = GetStackManagerByUserId(userId);
    auto connectManager = GetConnectManagerByUserId(userId);
    switch (type) {
        case AppExecFwk::AbilityType::SERVICE:
        case AppExecFwk::AbilityType::EXTENSION: {
            if (!connectManager) {
                HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
                return ERR_INVALID_VALUE;
            }
            auto result = connectManager->TerminateAbility(abilityRecord, requestCode);
            if (result == NO_FOUND_ABILITY_BY_CALLER) {
                if (!IsAbilityControllerForeground(abilityRecord->GetAbilityInfo().bundleName)) {
                    return ERR_WOULD_BLOCK;
                }

                if (!stackManager) {
                    HILOG_ERROR("stackManager is nullptr. userId=%{public}d", userId);
                    return ERR_INVALID_VALUE;
                }
                return stackManager->TerminateAbility(abilityRecord, requestCode);
            }
            return result;
        }
        case AppExecFwk::AbilityType::PAGE: {
            if (!IsAbilityControllerForeground(abilityRecord->GetAbilityInfo().bundleName)) {
                return ERR_WOULD_BLOCK;
            }
            if (!stackManager) {
                HILOG_ERROR("stackManager is nullptr. userId=%{public}d", userId);
                return ERR_INVALID_VALUE;
            }
            auto result = stackManager->TerminateAbility(abilityRecord, requestCode);
            if (result == NO_FOUND_ABILITY_BY_CALLER) {
                if (!connectManager) {
                    HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
                    return ERR_INVALID_VALUE;
                }
                return connectManager->TerminateAbility(abilityRecord, requestCode);
            }
            return result;
        }
        default:
            return ERR_INVALID_VALUE;
    }
}

int AbilityManagerService::MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser)
{
    HILOG_INFO("Minimize ability.");
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    int result = AbilityUtil::JudgeAbilityVisibleControl(abilityRecord->GetAbilityInfo());
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return result;
    }

    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::PAGE) {
        HILOG_ERROR("Cannot minimize except page ability.");
        return ERR_INVALID_VALUE;
    }

    if (!IsAbilityControllerForeground(abilityRecord->GetAbilityInfo().bundleName)) {
        return ERR_WOULD_BLOCK;
    }

    if (useNewMission_) {
        auto missionListManager = GetListManagerByUserId(userId);
        if (!missionListManager) {
            HILOG_ERROR("missionListManager is Null. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        return missionListManager->MinimizeAbility(token, fromUser);
    } else {
        auto stackManager = GetStackManagerByUserId(userId);
        if (!stackManager) {
            HILOG_ERROR("stackManager is nullptr. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        return stackManager->MinimizeAbility(token);
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

int AbilityManagerService::GetMissionSnapshot(const int32_t missionId, MissionPixelMap &missionPixelMap)
{
    if (missionId < 0) {
        HILOG_ERROR("GetMissionSnapshot failed.");
        return ERR_INVALID_VALUE;
    }
    return currentStackManager_->GetMissionSnapshot(missionId, missionPixelMap);
}

int AbilityManagerService::SetMissionDescriptionInfo(
    const sptr<IRemoteObject> &token, const MissionDescriptionInfo &description)
{
    HILOG_INFO("%{public}s called", __func__);
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(currentStackManager_, INNER_ERR);

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto stackManager = GetStackManagerByUserId(userId);
    if (!stackManager) {
        HILOG_ERROR("stackManager is nullptr. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return stackManager->SetMissionDescriptionInfo(abilityRecord, description);
}

int AbilityManagerService::GetMissionLockModeState()
{
    HILOG_INFO("%{public}s called", __func__);
    CHECK_POINTER_AND_RETURN(currentStackManager_, ERR_INVALID_VALUE);
    return currentStackManager_->GetMissionLockModeState();
}

int AbilityManagerService::UpdateConfiguration(const AppExecFwk::Configuration &config)
{
    HILOG_INFO("%{public}s called", __func__);
    return DelayedSingleton<AppScheduler>::GetInstance()->UpdateConfiguration(config);
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
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto stackManager = GetStackManagerByUserId(userId);
    if (!stackManager) {
        HILOG_ERROR("stackManager is nullptr. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return stackManager->MoveMissionToEnd(token, nonFirst);
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
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Connect ability.");
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);

    if (VerifyAccountPermission(userId) == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (CheckIfOperateRemote(want)) {
        HILOG_INFO("AbilityManagerService::ConnectAbility. try to ConnectRemoteAbility");
        return ConnectRemoteAbility(want, connect->AsObject());
    }

    int32_t validUserId = GetValidUserId(userId);
    if (callerToken != nullptr && callerToken->GetObjectDescriptor() != u"ohos.aafwk.AbilityToken") {
        HILOG_INFO("%{public}s invalid Token.", __func__);
        return ConnectLocalAbility(want, validUserId, connect, nullptr);
    }
    return ConnectLocalAbility(want, validUserId, connect, callerToken);
}

int AbilityManagerService::DisconnectAbility(const sptr<IAbilityConnection> &connect)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Disconnect ability.");
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);

    DisconnectLocalAbility(connect);
    DisconnectRemoteAbility(connect->AsObject());
    return ERR_OK;
}

int AbilityManagerService::ConnectLocalAbility(const Want &want, const int32_t userId,
    const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin ConnectAbilityLocal", __func__);
    AbilityRequest abilityRequest;
    ErrCode result = GenerateAbilityRequest(want, DEFAULT_INVAL_VALUE, abilityRequest, callerToken, userId);
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request error.");
        return result;
    }
    auto abilityInfo = abilityRequest.abilityInfo;
    int32_t validUserId = abilityInfo.applicationInfo.singleUser ? U0_USER_ID : userId;
    HILOG_DEBUG("validUserId : %{public}d, singleUser is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleUser));

    if (!JudgeMultiUserConcurrency(abilityRequest.abilityInfo, validUserId)) {
        HILOG_ERROR("Multi-user non-concurrent mode is not satisfied.");
        return ERR_INVALID_VALUE;
    }

    result = CheckStaticCfgPermission(abilityInfo);
    if (result != AppExecFwk::Constants::PERMISSION_GRANTED) {
        return result;
    }

    if (!VerifyUriPermisson(abilityRequest, want)) {
        HILOG_ERROR("The uri has not granted.");
        return ERR_INVALID_OPERATION;
    }

    result = AbilityUtil::JudgeAbilityVisibleControl(abilityInfo);
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return result;
    }
    auto type = abilityInfo.type;
    if (type != AppExecFwk::AbilityType::SERVICE && type != AppExecFwk::AbilityType::EXTENSION) {
        HILOG_ERROR("Connect Ability failed, target Ability is not Service.");
        return TARGET_ABILITY_NOT_SERVICE;
    }
    result = PreLoadAppDataAbilities(abilityInfo.bundleName, validUserId);
    if (result != ERR_OK) {
        HILOG_ERROR("ConnectAbility: App data ability preloading failed, '%{public}s', %{public}d",
            abilityInfo.bundleName.c_str(),
            result);
        return result;
    }

    auto connectManager = GetConnectManagerByUserId(validUserId);
    if (connectManager == nullptr) {
        HILOG_ERROR("connectManager is nullptr. userId=%{public}d", validUserId);
        return ERR_INVALID_VALUE;
    }
    return connectManager->ConnectAbilityLocked(abilityRequest, connect, callerToken);
}

int AbilityManagerService::ConnectRemoteAbility(const Want &want, const sptr<IRemoteObject> &connect)
{
    HILOG_INFO("%{public}s begin ConnectAbilityRemote", __func__);
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    DistributedClient dmsClient;
    HILOG_INFO("AbilityManagerService::Try to ConnectRemoteAbility, AccessTokenID = %{public}d", accessToken);
    return dmsClient.ConnectRemoteAbility(want, connect, callerUid, callerPid, accessToken);
}

int AbilityManagerService::DisconnectLocalAbility(const sptr<IAbilityConnection> &connect)
{
    HILOG_INFO("%{public}s begin DisconnectAbilityLocal", __func__);
    return connectManager_->DisconnectAbilityLocked(connect);
}

int AbilityManagerService::DisconnectRemoteAbility(const sptr<IRemoteObject> &connect)
{
    HILOG_INFO("%{public}s begin DisconnectAbilityRemote", __func__);
    DistributedClient dmsClient;
    return dmsClient.DisconnectRemoteAbility(connect);
}

int AbilityManagerService::ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
    int32_t missionId, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams)
{
    HILOG_INFO("ContinueMission srcDeviceId: %{public}s, dstDeviceId: %{public}s, missionId: %{public}d",
        srcDeviceId.c_str(), dstDeviceId.c_str(), missionId);
    if (VerifyMissionPermission() == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    DistributedClient dmsClient;
    return dmsClient.ContinueMission(srcDeviceId, dstDeviceId, missionId, callBack, wantParams);
}

int AbilityManagerService::ContinueAbility(const std::string &deviceId, int32_t missionId)
{
    HILOG_INFO("ContinueAbility deviceId : %{public}s, missionId = %{public}d.", deviceId.c_str(), missionId);

    sptr<IRemoteObject> abilityToken = GetAbilityTokenByMissionId(missionId);
    CHECK_POINTER_AND_RETURN(abilityToken, ERR_INVALID_VALUE);

    auto abilityRecord = Token::GetAbilityRecordByToken(abilityToken);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    abilityRecord->ContinueAbility(deviceId);
    return ERR_OK;
}

int AbilityManagerService::StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status)
{
    HILOG_INFO("Start Continuation.");
    if (!CheckIfOperateRemote(want)) {
        HILOG_ERROR("deviceId or bundle name or abilityName empty");
        return ERR_INVALID_VALUE;
    }
    CHECK_POINTER_AND_RETURN(abilityToken, ERR_INVALID_VALUE);

    int32_t appUid = IPCSkeleton::GetCallingUid();
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    HILOG_INFO("AbilityManagerService::Try to StartContinuation, AccessTokenID = %{public}d", accessToken);
    int32_t missionId = GetMissionIdByAbilityToken(abilityToken);
    if (missionId == -1) {
        HILOG_ERROR("AbilityManagerService::StartContinuation failed to get missionId.");
        return ERR_INVALID_VALUE;
    }
    DistributedClient dmsClient;
    auto result =  dmsClient.StartContinuation(want, missionId, appUid, status, accessToken);
    if (result != ERR_OK) {
        HILOG_ERROR("StartContinuation failed, result = %{public}d, notify caller", result);
        NotifyContinuationResult(missionId, result);
    }
    return result;
}

void AbilityManagerService::NotifyCompleteContinuation(const std::string &deviceId,
    int32_t sessionId, bool isSuccess)
{
    HILOG_INFO("NotifyCompleteContinuation.");
    DistributedClient dmsClient;
    dmsClient.NotifyCompleteContinuation(Str8ToStr16(deviceId), sessionId, isSuccess);
}

int AbilityManagerService::NotifyContinuationResult(int32_t missionId, int32_t result)
{
    HILOG_INFO("Notify Continuation Result : %{public}d.", result);

    auto abilityToken = GetAbilityTokenByMissionId(missionId);
    CHECK_POINTER_AND_RETURN(abilityToken, ERR_INVALID_VALUE);

    auto abilityRecord = Token::GetAbilityRecordByToken(abilityToken);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    abilityRecord->NotifyContinuationResult(result);
    return ERR_OK;
}

int AbilityManagerService::StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag)
{
    if (VerifyMissionPermission() == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    DistributedClient dmsClient;
    return dmsClient.StartSyncRemoteMissions(devId, fixConflict, tag);
}

int AbilityManagerService::StopSyncRemoteMissions(const std::string& devId)
{
    if (VerifyMissionPermission() == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    DistributedClient dmsClient;
    return dmsClient.StopSyncRemoteMissions(devId);
}

int AbilityManagerService::RegisterMissionListener(const std::string &deviceId,
    const sptr<IRemoteMissionListener> &listener)
{
    std::string localDeviceId;
    if (!GetLocalDeviceId(localDeviceId) || localDeviceId == deviceId) {
        HILOG_ERROR("RegisterMissionListener: Check DeviceId failed");
        return REGISTER_REMOTE_MISSION_LISTENER_FAIL;
    }
    CHECK_POINTER_AND_RETURN(listener, ERR_INVALID_VALUE);
    if (VerifyMissionPermission() == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    DistributedClient dmsClient;
    return dmsClient.RegisterMissionListener(Str8ToStr16(deviceId), listener->AsObject());
}

int AbilityManagerService::UnRegisterMissionListener(const std::string &deviceId,
    const sptr<IRemoteMissionListener> &listener)
{
    std::string localDeviceId;
    if (!GetLocalDeviceId(localDeviceId) || localDeviceId == deviceId) {
        HILOG_ERROR("RegisterMissionListener: Check DeviceId failed");
        return REGISTER_REMOTE_MISSION_LISTENER_FAIL;
    }
    CHECK_POINTER_AND_RETURN(listener, ERR_INVALID_VALUE);
    if (VerifyMissionPermission() == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    DistributedClient dmsClient;
    return dmsClient.UnRegisterMissionListener(Str8ToStr16(deviceId), listener->AsObject());
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
    if (!wantSenderInfo.bundleName.empty()) {
        bool bundleMgrResult = false;
        if (wantSenderInfo.userId < 0) {
            bundleMgrResult = bms->GetBundleInfo(wantSenderInfo.bundleName,
                AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo);
        } else {
            bundleMgrResult = bms->GetBundleInfo(wantSenderInfo.bundleName,
                AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, wantSenderInfo.userId);
        }
        if (!bundleMgrResult) {
            HILOG_ERROR("GetBundleInfo is fail.");
            return nullptr;
        }
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

int AbilityManagerService::SetShowOnLockScreen(bool isAllow)
{
    HILOG_INFO("SetShowOnLockScreen");
    CHECK_POINTER_AND_RETURN(currentStackManager_, ERR_NO_INIT);
    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, GET_ABILITY_SERVICE_FAILED);
    int callerUid = IPCSkeleton::GetCallingUid();
    std::string bundleName;
    bool result = bms->GetBundleNameForUid(callerUid, bundleName);
    if (!result) {
        HILOG_ERROR("GetBundleNameForUid fail");
        return GET_BUNDLENAME_BY_UID_FAIL;
    }
    return currentStackManager_->SetShowOnLockScreen(bundleName, isAllow);
}

int AbilityManagerService::LockMissionForCleanup(int32_t missionId)
{
    HILOG_INFO("request unlock mission for clean up all, id :%{public}d", missionId);
    CHECK_POINTER_AND_RETURN(currentStackManager_, ERR_NO_INIT);

    if (VerifyMissionPermission() == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    return currentMissionListManager_->SetMissionLockedState(missionId, true);
}

int AbilityManagerService::UnlockMissionForCleanup(int32_t missionId)
{
    HILOG_INFO("request unlock mission for clean up all, id :%{public}d", missionId);
    CHECK_POINTER_AND_RETURN(currentStackManager_, ERR_NO_INIT);

    if (VerifyMissionPermission() == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    return currentMissionListManager_->SetMissionLockedState(missionId, false);
}

int AbilityManagerService::RegisterMissionListener(const sptr<IMissionListener> &listener)
{
    HILOG_INFO("request RegisterMissionListener ");
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);

    if (VerifyMissionPermission() == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    return currentMissionListManager_->RegisterMissionListener(listener);
}

int AbilityManagerService::UnRegisterMissionListener(const sptr<IMissionListener> &listener)
{
    HILOG_INFO("request RegisterMissionListener ");
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);

    if (VerifyMissionPermission() == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    return currentMissionListManager_->UnRegisterMissionListener(listener);
}

int AbilityManagerService::GetMissionInfos(const std::string& deviceId, int32_t numMax,
    std::vector<MissionInfo> &missionInfos)
{
    HILOG_INFO("request GetMissionInfos.");
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);

    if (VerifyMissionPermission() == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (CheckIsRemote(deviceId)) {
        return GetRemoteMissionInfos(deviceId, numMax, missionInfos);
    }

    return currentMissionListManager_->GetMissionInfos(numMax, missionInfos);
}

int AbilityManagerService::GetRemoteMissionInfos(const std::string& deviceId, int32_t numMax,
    std::vector<MissionInfo> &missionInfos)
{
    HILOG_INFO("GetRemoteMissionInfos begin");
    DistributedClient dmsClient;
    int result = dmsClient.GetMissionInfos(deviceId, numMax, missionInfos);
    if (result != ERR_OK) {
        HILOG_ERROR("GetRemoteMissionInfos failed, result = %{public}d", result);
        return result;
    }
    return ERR_OK;
}

int AbilityManagerService::GetMissionInfo(const std::string& deviceId, int32_t missionId,
    MissionInfo &missionInfo)
{
    HILOG_INFO("request GetMissionInfo, missionId:%{public}d", missionId);
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);

    if (VerifyMissionPermission() == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (CheckIsRemote(deviceId)) {
        return GetRemoteMissionInfo(deviceId, missionId, missionInfo);
    }

    return currentMissionListManager_->GetMissionInfo(missionId, missionInfo);
}

int AbilityManagerService::GetRemoteMissionInfo(const std::string& deviceId, int32_t missionId,
    MissionInfo &missionInfo)
{
    HILOG_INFO("GetMissionInfoFromDms begin");
    std::vector<MissionInfo> missionVector;
    int result = GetRemoteMissionInfos(deviceId, MAX_NUMBER_OF_DISTRIBUTED_MISSIONS, missionVector);
    if (result != ERR_OK) {
        return result;
    }
    for (auto iter = missionVector.begin(); iter != missionVector.end(); iter++) {
        if (iter->id == missionId) {
            missionInfo = *iter;
            return ERR_OK;
        }
    }
    HILOG_WARN("missionId not found");
    return ERR_INVALID_VALUE;
}

int AbilityManagerService::CleanMission(int32_t missionId)
{
    HILOG_INFO("request CleanMission, missionId:%{public}d", missionId);
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);

    if (VerifyMissionPermission() == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    return currentMissionListManager_->ClearMission(missionId);
}

int AbilityManagerService::CleanAllMissions()
{
    HILOG_INFO("request CleanAllMissions ");
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);

    if (VerifyMissionPermission() == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    return currentMissionListManager_->ClearAllMissions();
}

int AbilityManagerService::MoveMissionToFront(int32_t missionId)
{
    HILOG_INFO("request MoveMissionToFront, missionId:%{public}d", missionId);
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);

    if (VerifyMissionPermission() == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    return currentMissionListManager_->MoveMissionToFront(missionId);
}

int AbilityManagerService::MoveMissionToFront(int32_t missionId, const StartOptions &startOptions)
{
    HILOG_INFO("request MoveMissionToFront, missionId:%{public}d", missionId);
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);

    if (VerifyMissionPermission() == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    auto options = std::make_shared<StartOptions>(startOptions);
    return currentMissionListManager_->MoveMissionToFront(missionId, options);
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
    bool isSystem = (IPCSkeleton::GetCallingUid() <= AppExecFwk::Constants::BASE_SYS_UID);
    if (!isSystem) {
        HILOG_INFO("callerToken not system %{public}s", __func__);
        if (!VerificationAllToken(callerToken)) {
            HILOG_INFO("VerificationAllToken fail");
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

    auto userId = GetValidUserId(INVALID_USER_ID);
    AbilityRequest abilityRequest;
    std::string dataAbilityUri = localUri.ToString();
    HILOG_INFO("%{public}s, called. userId %{public}d", __func__, userId);
    bool queryResult = iBundleManager_->QueryAbilityInfoByUri(dataAbilityUri, userId, abilityRequest.abilityInfo);
    if (!queryResult || abilityRequest.abilityInfo.name.empty() || abilityRequest.abilityInfo.bundleName.empty()) {
        HILOG_ERROR("Invalid ability info for data ability acquiring.");
        return nullptr;
    }

    if (!CheckDataAbilityRequest(abilityRequest)) {
        HILOG_ERROR("Invalid ability request info for data ability acquiring.");
        return nullptr;
    }

    HILOG_DEBUG("Query data ability info: %{public}s|%{public}s|%{public}s",
        abilityRequest.appInfo.name.c_str(),
        abilityRequest.appInfo.bundleName.c_str(),
        abilityRequest.abilityInfo.name.c_str());

    if (CheckStaticCfgPermission(abilityRequest.abilityInfo) != AppExecFwk::Constants::PERMISSION_GRANTED) {
        return nullptr;
    }

    if (abilityRequest.abilityInfo.applicationInfo.singleUser) {
        userId = U0_USER_ID;
    }

    std::shared_ptr<DataAbilityManager> dataAbilityManager = GetDataAbilityManagerByUserId(userId);
    CHECK_POINTER_AND_RETURN(dataAbilityManager, nullptr);
    return dataAbilityManager->Acquire(abilityRequest, tryBind, callerToken, isSystem);
}

bool AbilityManagerService::CheckDataAbilityRequest(AbilityRequest &abilityRequest)
{
    int result = AbilityUtil::JudgeAbilityVisibleControl(abilityRequest.abilityInfo);
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return false;
    }
    abilityRequest.appInfo = abilityRequest.abilityInfo.applicationInfo;
    if (abilityRequest.appInfo.name.empty() || abilityRequest.appInfo.bundleName.empty()) {
        HILOG_ERROR("Invalid app info for data ability acquiring.");
        return false;
    }
    if (abilityRequest.abilityInfo.type != AppExecFwk::AbilityType::DATA) {
        HILOG_ERROR("BMS query result is not a data ability.");
        return false;
    }
    abilityRequest.uid = abilityRequest.appInfo.uid;
    return true;
}

int AbilityManagerService::ReleaseDataAbility(
    sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s, called.", __func__);
    bool isSystem = (IPCSkeleton::GetCallingUid() <= AppExecFwk::Constants::BASE_SYS_UID);
    if (!isSystem) {
        HILOG_INFO("callerToken not system %{public}s", __func__);
        if (!VerificationAllToken(callerToken)) {
            HILOG_ERROR("VerificationAllToken fail");
            return ERR_INVALID_STATE;
        }
    }

    std::shared_ptr<DataAbilityManager> dataAbilityManager = GetDataAbilityManager(dataAbilityScheduler);
    if (!dataAbilityManager) {
        HILOG_ERROR("dataAbilityScheduler is not exists");
        return ERR_INVALID_VALUE;
    }

    return dataAbilityManager->Release(dataAbilityScheduler, callerToken, isSystem);
}

int AbilityManagerService::AttachAbilityThread(
    const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Attach ability thread.");
    CHECK_POINTER_AND_RETURN(scheduler, ERR_INVALID_VALUE);
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto abilityInfo = abilityRecord->GetAbilityInfo();
    auto type = abilityInfo.type;
    // force timeout ability for test
    if (IsNeedTimeoutForTest(abilityInfo.name, AbilityRecord::ConvertAbilityState(AbilityState::INITIAL))) {
        HILOG_WARN("force timeout ability for test, state:INITIAL, ability: %{public}s",
            abilityInfo.name.c_str());
        return ERR_OK;
    }
    int returnCode = -1;
    if (type == AppExecFwk::AbilityType::SERVICE || type == AppExecFwk::AbilityType::EXTENSION) {
        auto connectManager = GetConnectManagerByUserId(userId);
        if (!connectManager) {
            HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        returnCode = connectManager->AttachAbilityThreadLocked(scheduler, token);
    } else if (type == AppExecFwk::AbilityType::DATA) {
        auto dataAbilityManager = GetDataAbilityManagerByUserId(userId);
        if (!dataAbilityManager) {
            HILOG_ERROR("dataAbilityManager is Null. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        returnCode = dataAbilityManager->AttachAbilityThread(scheduler, token);
    } else {
        if (useNewMission_) {
            auto missionListManager = GetListManagerByUserId(userId);
            if (!missionListManager) {
                HILOG_ERROR("missionListManager is Null. userId=%{public}d", userId);
                return ERR_INVALID_VALUE;
            }
            returnCode = missionListManager->AttachAbilityThread(scheduler, token);
        } else {
            auto stackManager = GetStackManagerByUserId(userId);
            if (!stackManager) {
                HILOG_ERROR("stackManager is nullptr. userId=%{public}d", userId);
                return ERR_INVALID_VALUE;
            }
            returnCode = stackManager->AttachAbilityThread(scheduler, token);
        }
    }
    return returnCode;
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
    dumpFuncMap_[KEY_DUMP_FOCUS_ABILITY] = &AbilityManagerService::DumpFocusMapInner;
    dumpFuncMap_[KEY_DUMP_WINDOW_MODE] = &AbilityManagerService::DumpWindowModeInner;
    dumpFuncMap_[KEY_DUMP_MISSION_LIST] = &AbilityManagerService::DumpMissionListInner;
    dumpFuncMap_[KEY_DUMP_MISSION_INFOS] = &AbilityManagerService::DumpMissionInfosInner;
}

void AbilityManagerService::DumpSysFuncInit()
{
    dumpsysFuncMap_[KEY_DUMPSYS_ALL] = &AbilityManagerService::DumpSysInner;
    dumpsysFuncMap_[KEY_DUMPSYS_MISSION_LIST] = &AbilityManagerService::DumpSysMissionListInner;
    dumpsysFuncMap_[KEY_DUMPSYS_ABILITY] = &AbilityManagerService::DumpSysAbilityInner;
    dumpsysFuncMap_[KEY_DUMPSYS_SERVICE] = &AbilityManagerService::DumpSysStateInner;
    dumpsysFuncMap_[KEY_DUMPSYS_PENDING] = &AbilityManagerService::DumpSysPendingInner;
    dumpsysFuncMap_[KEY_DUMPSYS_PROCESS] = &AbilityManagerService::DumpSysProcess;
    dumpsysFuncMap_[KEY_DUMPSYS_DATA] = &AbilityManagerService::DataDumpSysStateInner;
}

void AbilityManagerService::DumpSysInner(
    const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int userId)
{
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    DumpSysMissionListInner(args, info, isClient, isUserID, userId);
    DumpSysStateInner(args, info, isClient, isUserID, userId);
    DumpSysPendingInner(args, info, isClient, isUserID, userId);
    DumpSysProcess(args, info, isClient, isUserID, userId);
}

void AbilityManagerService::DumpSysMissionListInner(
    const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId)
{
    std::shared_ptr<MissionListManager> targetManager;
    if (isUserID) {
        auto it = missionListManagers_.find(userId);
        if (it == missionListManagers_.end()) {
            info.push_back("error: No user found'.");
            return;
        }
        targetManager = it->second;
    } else {
        targetManager = currentMissionListManager_;
    }

    CHECK_POINTER(targetManager);

    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }

    if (argList.size() == MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpMissionList(info, isClient, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpMissionList(info, isClient);
    } else {
        info.emplace_back("error: invalid argument, please see 'ability dumpsys -h'.");
    }
}
void AbilityManagerService::DumpSysAbilityInner(
    const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId)
{
    std::shared_ptr<MissionListManager> targetManager;
    if (isUserID) {
        auto it = missionListManagers_.find(userId);
        if (it == missionListManagers_.end()) {
            info.push_back("error: No user found'.");
            return;
        }
        targetManager = it->second;
    } else {
        targetManager = currentMissionListManager_;
    }

    CHECK_POINTER(targetManager);

    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    if (argList.size() >= MIN_DUMP_ARGUMENT_NUM) {
        HILOG_INFO("argList = %{public}s", argList[1].c_str());
        std::vector<std::string> params(argList.begin() + MIN_DUMP_ARGUMENT_NUM, argList.end());
        targetManager->DumpMissionListByRecordId(info, isClient, std::stoi(argList[1]), params);
    } else {
        info.emplace_back("error: invalid argument, please see 'ability dumpsys -h'.");
    }
}

void AbilityManagerService::DumpSysStateInner(
    const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int userId)
{
    std::shared_ptr<AbilityConnectManager> targetManager;

    if (isUserID) {
        auto it = connectManagers_.find(userId);
        if (it == connectManagers_.end()) {
            info.push_back("error: No user found'.");
            return;
        }
        targetManager = it->second;
    } else {
        targetManager = connectManager_;
    }

    CHECK_POINTER(targetManager);

    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }

    if (argList.size() == MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpState(info, isClient, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpState(info, isClient);
    } else {
        info.emplace_back("error: invalid argument, please see 'ability dumpsys -h'.");
    }
}

void AbilityManagerService::DumpSysPendingInner(
    const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int userId)
{
    std::shared_ptr<PendingWantManager> targetManager;
    if (isUserID) {
        auto it = pendingWantManagers_.find(userId);
        if (it == pendingWantManagers_.end()) {
            info.push_back("error: No user found'.");
            return;
        }
        targetManager = it->second;
    } else {
        targetManager = pendingWantManager_;
    }

    CHECK_POINTER(targetManager);

    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }

    if (argList.size() == MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpByRecordId(info, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        targetManager->Dump(info);
    } else {
        info.emplace_back("error: invalid argument, please see 'ability dumpsys -h'.");
    }
}

void AbilityManagerService::DumpSysProcess(
    const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int userId)
{
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    std::vector<AppExecFwk::RunningProcessInfo> ProcessInfos;
    int ret = 0;
    if (isUserID) {
        ret = GetProcessRunningInfosByUserId(ProcessInfos, userId);
    } else {
        ret = GetProcessRunningInfos(ProcessInfos);
    }

    if (ret != ERR_OK || ProcessInfos.size() == 0) {
        return;
    }

    std::string dumpInfo = "  AppRunningRecords:";
    info.push_back(dumpInfo);
    auto processInfoID = 0;
    auto hasProcessName = (argList.size() == MIN_DUMP_ARGUMENT_NUM ? true : false);
    for (const auto& ProcessInfo : ProcessInfos) {
        if (hasProcessName && argList[1] != ProcessInfo.processName_) {
            continue;
        }

        dumpInfo = "    AppRunningRecord ID #" + std::to_string(processInfoID);
        processInfoID++;
        info.push_back(dumpInfo);
        dumpInfo = "      process name [" + ProcessInfo.processName_ + "]";
        info.push_back(dumpInfo);
        dumpInfo = "      pid #" + std::to_string(ProcessInfo.pid_) +
            "  uid #" + std::to_string(ProcessInfo.uid_);
        info.push_back(dumpInfo);
        auto appState = static_cast<AppState>(ProcessInfo.state_);
        if (appScheduler_) {
            dumpInfo = "      state #" + appScheduler_->ConvertAppState(appState);
        }
        info.push_back(dumpInfo);
    }
}

void AbilityManagerService::DataDumpSysStateInner(
    const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int userId)
{
    std::shared_ptr<DataAbilityManager> targetManager;
    if (isUserID) {
        auto it = dataAbilityManagers_.find(userId);
        if (it == dataAbilityManagers_.end()) {
            info.push_back("error: No user found'.");
            return;
        }
        targetManager = it->second;
    } else {
        targetManager = dataAbilityManager_;
    }

    CHECK_POINTER(targetManager);

    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    if (argList.size() == MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpSysState(info, isClient, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpSysState(info, isClient);
    } else {
        info.emplace_back("error: invalid argument, please see 'ability dump -h'.");
    }
}

void AbilityManagerService::DumpInner(const std::string &args, std::vector<std::string> &info)
{
    if (useNewMission_) {
        if (currentMissionListManager_) {
            currentMissionListManager_->Dump(info);
        }
    } else {
        if (currentStackManager_) {
            currentStackManager_->Dump(info);
        }
    }
}

void AbilityManagerService::DumpStackListInner(const std::string &args, std::vector<std::string> &info)
{
    currentStackManager_->DumpStackList(info);
}

void AbilityManagerService::DumpFocusMapInner(const std::string &args, std::vector<std::string> &info)
{
    currentStackManager_->DumpFocusMap(info);
}

void AbilityManagerService::DumpWindowModeInner(const std::string &args, std::vector<std::string> &info)
{
    currentStackManager_->DumpWindowMode(info);
}

void AbilityManagerService::DumpMissionListInner(const std::string &args, std::vector<std::string> &info)
{
    if (currentMissionListManager_) {
        currentMissionListManager_->DumpMissionList(info, false, "");
    }
}

void AbilityManagerService::DumpMissionInfosInner(const std::string &args, std::vector<std::string> &info)
{
    if (currentMissionListManager_) {
        currentMissionListManager_->DumpMissionInfos(info);
    }
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
    if (useNewMission_) {
        currentMissionListManager_->DumpMission(missionId, info);
    } else {
        currentStackManager_->DumpMission(missionId, info);
    }
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
        connectManager_->DumpState(info, false, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        connectManager_->DumpState(info, false);
    } else {
        info.emplace_back("error: invalid argument, please see 'ability dump -h'.");
    }
}

bool AbilityManagerService::IsExistFile(const std::string &path)
{
    HILOG_INFO("%{public}s", __func__);
    if (path.empty()) {
        return false;
    }
    struct stat buf = {};
    if (stat(path.c_str(), &buf) != 0) {
        return false;
    }
    HILOG_INFO("%{public}s  :file exists", __func__);
    return S_ISREG(buf.st_mode);
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

void AbilityManagerService::DumpSysState(
    const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int userId)
{
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    auto it = dumpsysMap.find(argList[0]);
    if (it == dumpsysMap.end()) {
        return;
    }
    DumpsysKey key = it->second;
    auto itFunc = dumpsysFuncMap_.find(key);
    if (itFunc != dumpsysFuncMap_.end()) {
        auto dumpsysFunc = itFunc->second;
        if (dumpsysFunc != nullptr) {
            (this->*dumpsysFunc)(args, info, isClient, isUserID, userId);
            return;
        }
    }
    info.push_back("error: invalid argument, please see 'ability dump -h'.");
}

int AbilityManagerService::AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Ability transition done, state:%{public}d", state);
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN_LOG(abilityRecord, ERR_INVALID_VALUE, "Ability record is nullptr.");
    auto abilityInfo = abilityRecord->GetAbilityInfo();
    HILOG_DEBUG("state:%{public}d  name:%{public}s", state, abilityInfo.name.c_str());
    auto type = abilityInfo.type;
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    // force timeout ability for test
    int targetState = AbilityRecord::ConvertLifeCycleToAbilityState(static_cast<AbilityLifeCycleState>(state));
    if (IsNeedTimeoutForTest(abilityInfo.name,
        AbilityRecord::ConvertAbilityState(static_cast<AbilityState>(targetState)))) {
        HILOG_WARN("force timeout ability for test, state:%{public}s, ability: %{public}s",
            AbilityRecord::ConvertAbilityState(static_cast<AbilityState>(targetState)).c_str(),
            abilityInfo.name.c_str());
        return ERR_OK;
    }
    if (type == AppExecFwk::AbilityType::SERVICE || type == AppExecFwk::AbilityType::EXTENSION) {
        auto connectManager = GetConnectManagerByUserId(userId);
        if (!connectManager) {
            HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        return connectManager->AbilityTransitionDone(token, state);
    }
    if (type == AppExecFwk::AbilityType::DATA) {
        auto dataAbilityManager = GetDataAbilityManagerByUserId(userId);
        if (!dataAbilityManager) {
            HILOG_ERROR("dataAbilityManager is Null. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        return dataAbilityManager->AbilityTransitionDone(token, state);
    }
    if (useNewMission_) {
        auto missionListManager = GetListManagerByUserId(userId);
        if (!missionListManager) {
            HILOG_ERROR("missionListManager is Null. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        return missionListManager->AbilityTransactionDone(token, state, saveData);
    } else {
        auto stackManager = GetStackManagerByUserId(userId);
        if (!stackManager) {
            HILOG_ERROR("stackManager is nullptr. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        return stackManager->AbilityTransitionDone(token, state, saveData);
    }
}

int AbilityManagerService::ScheduleConnectAbilityDone(
    const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Schedule connect ability done.");
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::SERVICE && type != AppExecFwk::AbilityType::EXTENSION) {
        HILOG_ERROR("Connect ability failed, target ability is not service.");
        return TARGET_ABILITY_NOT_SERVICE;
    }
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto connectManager = GetConnectManagerByUserId(userId);
    if (!connectManager) {
        HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return connectManager->ScheduleConnectAbilityDoneLocked(token, remoteObject);
}

int AbilityManagerService::ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Schedule disconnect ability done.");
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::SERVICE && type != AppExecFwk::AbilityType::EXTENSION) {
        HILOG_ERROR("Connect ability failed, target ability is not service.");
        return TARGET_ABILITY_NOT_SERVICE;
    }
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto connectManager = GetConnectManagerByUserId(userId);
    if (!connectManager) {
        HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return connectManager->ScheduleDisconnectAbilityDoneLocked(token);
}

int AbilityManagerService::ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Schedule command ability done.");
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::SERVICE && type != AppExecFwk::AbilityType::EXTENSION) {
        HILOG_ERROR("Connect ability failed, target ability is not service.");
        return TARGET_ABILITY_NOT_SERVICE;
    }
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto connectManager = GetConnectManagerByUserId(userId);
    if (!connectManager) {
        HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return connectManager->ScheduleCommandAbilityDoneLocked(token);
}

void AbilityManagerService::AddWindowInfo(const sptr<IRemoteObject> &token, int32_t windowToken)
{
    HILOG_DEBUG("Add window id.");
    if (!VerificationAllToken(token)) {
        return;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto stackManager = GetStackManagerByUserId(userId);
    if (!stackManager) {
        HILOG_ERROR("stackManager is nullptr. userId=%{public}d", userId);
        return ;
    }
    stackManager->AddWindowInfo(token, windowToken);
}

void AbilityManagerService::OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("On ability request done.");

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER(abilityRecord);
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;

    auto type = abilityRecord->GetAbilityInfo().type;
    switch (type) {
        case AppExecFwk::AbilityType::SERVICE:
        case AppExecFwk::AbilityType::EXTENSION: {
            auto connectManager = GetConnectManagerByUserId(userId);
            if (!connectManager) {
                HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
                return;
            }
            connectManager->OnAbilityRequestDone(token, state);
            break;
        }
        case AppExecFwk::AbilityType::DATA: {
            auto dataAbilityManager = GetDataAbilityManagerByUserId(userId);
            if (!dataAbilityManager) {
                HILOG_ERROR("dataAbilityManager is Null. userId=%{public}d", userId);
                return;
            }
            dataAbilityManager->OnAbilityRequestDone(token, state);
            break;
        }
        default: {
            if (useNewMission_) {
                auto missionListManager = GetListManagerByUserId(userId);
                if (!missionListManager) {
                    HILOG_ERROR("missionListManager is Null. userId=%{public}d", userId);
                    return;
                }
                missionListManager->OnAbilityRequestDone(token, state);
            } else {
                auto stackManager = GetStackManagerByUserId(userId);
                if (!stackManager) {
                    HILOG_ERROR("stackManager is nullptr. userId=%{public}d", userId);
                    return;
                }
                stackManager->OnAbilityRequestDone(token, state);
            }
            break;
        }
    }
}

void AbilityManagerService::OnAppStateChanged(const AppInfo &info)
{
    HILOG_INFO("On app state changed.");
    connectManager_->OnAppStateChanged(info);
    if (useNewMission_) {
        currentMissionListManager_->OnAppStateChanged(info);
    } else {
        currentStackManager_->OnAppStateChanged(info);
    }
    dataAbilityManager_->OnAppStateChanged(info);
}

std::shared_ptr<AbilityEventHandler> AbilityManagerService::GetEventHandler()
{
    return handler_;
}

void AbilityManagerService::SetStackManager(int userId, bool switchUser)
{
    auto iterator = stackManagers_.find(userId);
    if (iterator != stackManagers_.end()) {
        if (switchUser) {
            currentStackManager_ = iterator->second;
        }
    } else {
        auto manager = std::make_shared<AbilityStackManager>(userId);
        manager->Init();
        stackManagers_.emplace(userId, manager);
        if (switchUser) {
            currentStackManager_ = manager;
        }
    }
}

void AbilityManagerService::InitMissionListManager(int userId, bool switchUser)
{
    auto iterator = missionListManagers_.find(userId);
    if (iterator != missionListManagers_.end()) {
        if (switchUser) {
            DelayedSingleton<MissionInfoMgr>::GetInstance()->Init(userId);
            currentMissionListManager_ = iterator->second;
        }
    } else {
        auto manager = std::make_shared<MissionListManager>(userId);
        manager->Init();
        missionListManagers_.emplace(userId, manager);
        if (switchUser) {
            currentMissionListManager_ = manager;
        }
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
    if (userController_) {
        return userController_->GetCurrentUserId();
    }
    return U0_USER_ID;
}

void AbilityManagerService::StartingLauncherAbility()
{
    HILOG_DEBUG("%{public}s", __func__);
    if (!iBundleManager_) {
        HILOG_INFO("bms service is null");
        return;
    }

    /* query if launcher ability has installed */
    AppExecFwk::AbilityInfo abilityInfo;
    /* First stage, hardcoding for the first launcher App */
    auto userId = GetUserId();
    Want want;
    want.SetElementName(AbilityConfig::LAUNCHER_BUNDLE_NAME, AbilityConfig::LAUNCHER_ABILITY_NAME);
    HILOG_DEBUG("%{public}s, QueryAbilityInfo, userId is %{public}d", __func__, userId);
    while (!(iBundleManager_->QueryAbilityInfo(want, AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION,
        userId, abilityInfo))) {
        HILOG_INFO("Waiting query launcher ability info completed.");
        usleep(REPOLL_TIME_MICRO_SECONDS);
    }

    HILOG_INFO("Start Home Launcher Ability.");
    /* start launch ability */
    (void)StartAbility(want, userId, DEFAULT_INVAL_VALUE);
    return;
}

void AbilityManagerService::StartingPhoneServiceAbility()
{
    HILOG_DEBUG("%{public}s", __func__);
    if (!iBundleManager_) {
        HILOG_INFO("bms service is null");
        return;
    }

    AppExecFwk::AbilityInfo phoneServiceInfo;
    Want phoneServiceWant;
    phoneServiceWant.SetElementName(AbilityConfig::PHONE_SERVICE_BUNDLE_NAME,
        AbilityConfig::PHONE_SERVICE_ABILITY_NAME);

    auto userId = GetUserId();
    int attemptNums = 1;
    HILOG_DEBUG("%{public}s, QueryAbilityInfo, userId is %{public}d", __func__, userId);
    while (!(iBundleManager_->QueryAbilityInfo(phoneServiceWant,
        OHOS::AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT, userId, phoneServiceInfo)) &&
        attemptNums <= MAX_NUMBER_OF_CONNECT_BMS) {
        HILOG_INFO("Waiting query phone service ability info completed.");
        usleep(REPOLL_TIME_MICRO_SECONDS);
        attemptNums++;
    }

    (void)StartAbility(phoneServiceWant, userId, DEFAULT_INVAL_VALUE);
}

void AbilityManagerService::StartingContactsAbility()
{
    HILOG_DEBUG("%{public}s", __func__);
    if (!iBundleManager_) {
        HILOG_INFO("bms service is null");
        return;
    }

    AppExecFwk::AbilityInfo contactsInfo;
    Want contactsWant;
    contactsWant.SetElementName(AbilityConfig::CONTACTS_BUNDLE_NAME, AbilityConfig::CONTACTS_ABILITY_NAME);

    auto userId = GetUserId();
    int attemptNums = 1;
    HILOG_DEBUG("%{public}s, QueryAbilityInfo, userId is %{public}d", __func__, userId);
    while (!(iBundleManager_->QueryAbilityInfo(contactsWant,
        OHOS::AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT, userId, contactsInfo)) &&
        attemptNums <= MAX_NUMBER_OF_CONNECT_BMS) {
        HILOG_INFO("Waiting query contacts service completed.");
        usleep(REPOLL_TIME_MICRO_SECONDS);
        attemptNums++;
    }

    HILOG_INFO("attemptNums : %{public}d", attemptNums);
    if (attemptNums <= MAX_NUMBER_OF_CONNECT_BMS) {
        (void)StartAbility(contactsWant, userId, DEFAULT_INVAL_VALUE);
    }
}

void AbilityManagerService::StartingMmsAbility()
{
    HILOG_DEBUG("%{public}s", __func__);
    if (!iBundleManager_) {
        HILOG_INFO("bms service is null");
        return;
    }

    AppExecFwk::AbilityInfo mmsInfo;
    Want mmsWant;
    mmsWant.SetElementName(AbilityConfig::MMS_BUNDLE_NAME, AbilityConfig::MMS_ABILITY_NAME);

    auto userId = GetUserId();
    int attemptNums = 1;
    HILOG_DEBUG("%{public}s, QueryAbilityInfo, userId is %{public}d", __func__, userId);
    while (!(iBundleManager_->QueryAbilityInfo(mmsWant,
        OHOS::AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT, userId, mmsInfo)) &&
        attemptNums <= MAX_NUMBER_OF_CONNECT_BMS) {
        HILOG_INFO("Waiting query mms service completed.");
        usleep(REPOLL_TIME_MICRO_SECONDS);
        attemptNums++;
    }

    HILOG_INFO("attemptNums : %{public}d", attemptNums);
    if (attemptNums <= MAX_NUMBER_OF_CONNECT_BMS) {
        (void)StartAbility(mmsWant, userId, DEFAULT_INVAL_VALUE);
    }
}

int AbilityManagerService::GenerateAbilityRequest(
    const Want &want, int requestCode, AbilityRequest &request, const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    request.want = want;
    request.requestCode = requestCode;
    request.callerToken = callerToken;
    request.startSetting = nullptr;

    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, GET_ABILITY_SERVICE_FAILED);

    if (want.GetAction().compare(ACTION_CHOOSE) == 0) {
        return ShowPickerDialog(want, userId);
    }
    auto abilityInfoFlag = (AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_PERMISSION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_METADATA);
    HILOG_DEBUG("%{public}s, QueryAbilityInfo, userId is %{public}d", __func__, userId);
    bms->QueryAbilityInfo(
        want, abilityInfoFlag, userId, request.abilityInfo);
    if (request.abilityInfo.name.empty() || request.abilityInfo.bundleName.empty()) {
        // try to find extension
        std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
        bms->QueryExtensionAbilityInfos(want, abilityInfoFlag, userId, extensionInfos);
        if (extensionInfos.size() <= 0) {
            HILOG_ERROR("Get extension info failed.");
            return RESOLVE_ABILITY_ERR;
        }

        AppExecFwk::ExtensionAbilityInfo extensionInfo = extensionInfos.front();
        if (extensionInfo.bundleName.empty() || extensionInfo.name.empty()) {
            HILOG_ERROR("extensionInfo empty.");
            return RESOLVE_ABILITY_ERR;
        }
        HILOG_DEBUG("QueryExtensionAbilityInfo, extension ability info found, name=%{public}s",
            extensionInfo.name.c_str());
        // For compatibility translates to AbilityInfo
        InitAbilityInfoFromExtension(extensionInfo, request.abilityInfo);
    }
    HILOG_DEBUG("Query ability name: %{public}s, is stage mode: %{public}d",
        request.abilityInfo.name.c_str(), request.abilityInfo.isStageBasedModel);
    if (request.abilityInfo.type == AppExecFwk::AbilityType::SERVICE && request.abilityInfo.isStageBasedModel) {
        HILOG_INFO("stage mode, abilityInfo SERVICE type reset EXTENSION.");
        request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    }

    if (request.abilityInfo.applicationInfo.name.empty() || request.abilityInfo.applicationInfo.bundleName.empty()) {
        HILOG_ERROR("Get app info failed.");
        return RESOLVE_APP_ERR;
    }
    request.appInfo = request.abilityInfo.applicationInfo;
    request.compatibleVersion = request.appInfo.apiCompatibleVersion;
    request.uid = request.appInfo.uid;
    HILOG_DEBUG("End, app name: %{public}s, bundle name: %{public}s, uid: %{public}d",
        request.appInfo.name.c_str(), request.appInfo.bundleName.c_str(), request.uid);

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
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    int result = AbilityUtil::JudgeAbilityVisibleControl(abilityRecord->GetAbilityInfo());
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return result;
    }

    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::SERVICE && type != AppExecFwk::AbilityType::EXTENSION) {
        HILOG_ERROR("target ability is not service.");
        return TARGET_ABILITY_NOT_SERVICE;
    }

    auto connectManager = GetConnectManagerByUserId(userId);
    if (!connectManager) {
        HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return connectManager->TerminateAbilityResult(token, startId);
}

int AbilityManagerService::StopServiceAbility(const Want &want, int32_t userId)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Stop service ability.");

    int32_t validUserId = GetValidUserId(userId);

    AbilityRequest abilityRequest;
    auto result = GenerateAbilityRequest(want, DEFAULT_INVAL_VALUE, abilityRequest, nullptr, validUserId);
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request local error.");
        return result;
    }

    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleUser ? U0_USER_ID : validUserId;
    HILOG_DEBUG("validUserId : %{public}d, singleUser is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleUser));

    if (!JudgeMultiUserConcurrency(abilityRequest.abilityInfo, validUserId)) {
        HILOG_ERROR("Multi-user non-concurrent mode is not satisfied.");
        return ERR_INVALID_VALUE;
    }

    auto type = abilityInfo.type;
    if (type != AppExecFwk::AbilityType::SERVICE && type != AppExecFwk::AbilityType::EXTENSION) {
        HILOG_ERROR("Target ability is not service type.");
        return TARGET_ABILITY_NOT_SERVICE;
    }

    auto connectManager = GetConnectManagerByUserId(validUserId);
    if (connectManager == nullptr) {
        return ERR_INVALID_VALUE;
    }

    return connectManager->StopServiceAbility(abilityRequest);
}

void AbilityManagerService::OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord)
{
    CHECK_POINTER(abilityRecord);

    if (useNewMission_) {
        auto manager = GetListManagerByToken(abilityRecord->GetToken());
        if (manager) {
            manager->OnAbilityDied(abilityRecord, GetUserId());
            return;
        }
    } else {
        auto manager = GetStackManagerByToken(abilityRecord->GetToken());
        if (manager) {
            manager->OnAbilityDied(abilityRecord);
            return;
        }
    }

    auto manager = GetConnectManagerByToken(abilityRecord->GetToken());
    if (manager) {
        manager->OnAbilityDied(abilityRecord);
        return;
    }

    auto dataAbilityManager = GetDataAbilityManagerByToken(abilityRecord->GetToken());
    if (dataAbilityManager) {
        dataAbilityManager->OnAbilityDied(abilityRecord);
    }
}

void AbilityManagerService::OnCallConnectDied(std::shared_ptr<CallRecord> callRecord)
{
    CHECK_POINTER(callRecord);
    if (currentMissionListManager_) {
        currentMissionListManager_->OnCallConnectDied(callRecord);
    }
}

void AbilityManagerService::GetMaxRestartNum(int &max)
{
    if (amsConfigResolver_) {
        max = amsConfigResolver_->GetMaxRestartNum();
    }
}

bool AbilityManagerService::IsUseNewMission()
{
    return useNewMission_;
}

int AbilityManagerService::KillProcess(const std::string &bundleName)
{
    HILOG_DEBUG("Kill process, bundleName: %{public}s", bundleName.c_str());
    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, KILL_PROCESS_FAILED);
    int32_t userId = GetUserId();
    AppExecFwk::BundleInfo bundleInfo;
    if (!bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId)) {
        HILOG_ERROR("Failed to get bundle info when kill process.");
        return GET_BUNDLE_INFO_FAILED;
    }

    if (bundleInfo.isKeepAlive) {
        HILOG_ERROR("Can not kill keep alive process.");
        return KILL_PROCESS_KEEP_ALIVE;
    }

    int ret = DelayedSingleton<AppScheduler>::GetInstance()->KillApplication(bundleName);
    if (ret != ERR_OK) {
        return KILL_PROCESS_FAILED;
    }
    return ERR_OK;
}

int AbilityManagerService::ClearUpApplicationData(const std::string &bundleName)
{
    HILOG_DEBUG("ClearUpApplicationData, bundleName: %{public}s", bundleName.c_str());
    int ret = DelayedSingleton<AppScheduler>::GetInstance()->ClearUpApplicationData(bundleName);
    if (ret != ERR_OK) {
        return CLEAR_APPLICATION_DATA_FAIL;
    }
    return ERR_OK;
}

int AbilityManagerService::UninstallApp(const std::string &bundleName)
{
    HILOG_DEBUG("Uninstall app, bundleName: %{public}s", bundleName.c_str());
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getpid();
    if (callingPid != pid) {
        HILOG_ERROR("%{public}s: Not bundleMgr call.", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    CHECK_POINTER_AND_RETURN(currentStackManager_, ERR_NO_INIT);
    currentStackManager_->UninstallApp(bundleName);
    CHECK_POINTER_AND_RETURN(pendingWantManager_, ERR_NO_INIT);
    pendingWantManager_->ClearPendingWantRecord(bundleName);
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

int AbilityManagerService::PreLoadAppDataAbilities(const std::string &bundleName, const int32_t userId)
{
    if (bundleName.empty()) {
        HILOG_ERROR("Invalid bundle name when app data abilities preloading.");
        return ERR_INVALID_VALUE;
    }

    auto dataAbilityManager = GetDataAbilityManagerByUserId(userId);
    if (dataAbilityManager == nullptr) {
        HILOG_ERROR("Invalid data ability manager when app data abilities preloading.");
        return ERR_INVALID_STATE;
    }

    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, GET_ABILITY_SERVICE_FAILED);

    AppExecFwk::BundleInfo bundleInfo;
    bool ret = bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, userId);
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
        dataAbilityRequest.uid = bundleInfo.uid;
        HILOG_INFO("App data ability preloading: '%{public}s.%{public}s'...", it->bundleName.c_str(), it->name.c_str());

        auto dataAbility = dataAbilityManager->Acquire(dataAbilityRequest, false, nullptr, false);
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
    return (info.name == AbilityConfig::SYSTEM_UI_NAVIGATION_BAR ||
        info.name == AbilityConfig::SYSTEM_UI_STATUS_BAR ||
        info.name == AbilityConfig::SYSTEM_UI_ABILITY_NAME);
}

bool AbilityManagerService::IsSystemUI(const std::string &bundleName) const
{
    return bundleName == AbilityConfig::SYSTEM_UI_BUNDLE_NAME;
}

void AbilityManagerService::HandleLoadTimeOut(int64_t eventId)
{
    HILOG_DEBUG("Handle load timeout.");
    if (useNewMission_) {
        for (auto& item : missionListManagers_) {
            if (item.second) {
                item.second->OnTimeOut(AbilityManagerService::LOAD_TIMEOUT_MSG, eventId);
            }
        }
    } else {
        for (auto& item : stackManagers_) {
            if (item.second) {
                item.second->OnTimeOut(AbilityManagerService::LOAD_TIMEOUT_MSG, eventId);
            }
        }
    }
}

void AbilityManagerService::HandleActiveTimeOut(int64_t eventId)
{
    HILOG_DEBUG("Handle active timeout.");

    if (useNewMission_) {
        for (auto& item : missionListManagers_) {
            if (item.second) {
                item.second->OnTimeOut(AbilityManagerService::ACTIVE_TIMEOUT_MSG, eventId);
            }
        }
    } else {
        for (auto& item : stackManagers_) {
            if (item.second) {
                item.second->OnTimeOut(AbilityManagerService::ACTIVE_TIMEOUT_MSG, eventId);
            }
        }
    }
}

void AbilityManagerService::HandleInactiveTimeOut(int64_t eventId)
{
    HILOG_DEBUG("Handle inactive timeout.");
    if (useNewMission_) {
        for (auto& item : missionListManagers_) {
            if (item.second) {
                item.second->OnTimeOut(AbilityManagerService::INACTIVE_TIMEOUT_MSG, eventId);
            }
        }
    } else {
        for (auto& item : stackManagers_) {
            if (item.second) {
                item.second->OnTimeOut(AbilityManagerService::INACTIVE_TIMEOUT_MSG, eventId);
            }
        }
    }
}

void AbilityManagerService::HandleForegroundNewTimeOut(int64_t eventId)
{
    HILOG_DEBUG("Handle ForegroundNew timeout.");
    if (useNewMission_) {
        for (auto& item : missionListManagers_) {
            if (item.second) {
                item.second->OnTimeOut(AbilityManagerService::FOREGROUNDNEW_TIMEOUT_MSG, eventId);
            }
        }
    } else {
        for (auto& item : stackManagers_) {
            if (item.second) {
                item.second->OnTimeOut(AbilityManagerService::FOREGROUNDNEW_TIMEOUT_MSG, eventId);
            }
        }
    }
}

void AbilityManagerService::HandleBackgroundNewTimeOut(int64_t eventId)
{
    HILOG_DEBUG("Handle BackgroundNew timeout.");
    if (useNewMission_) {
        for (auto& item : missionListManagers_) {
            if (item.second) {
                item.second->OnTimeOut(AbilityManagerService::BACKGROUNDNEW_TIMEOUT_MSG, eventId);
            }
        }
    } else {
        for (auto& item : stackManagers_) {
            if (item.second) {
                item.second->OnTimeOut(AbilityManagerService::BACKGROUNDNEW_TIMEOUT_MSG, eventId);
            }
        }
    }
}

bool AbilityManagerService::VerificationToken(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Verification token.");
    CHECK_POINTER_RETURN_BOOL(dataAbilityManager_);
    CHECK_POINTER_RETURN_BOOL(connectManager_);
    CHECK_POINTER_RETURN_BOOL(currentStackManager_);
    CHECK_POINTER_RETURN_BOOL(currentMissionListManager_);

    if (useNewMission_) {
        if (currentMissionListManager_->GetAbilityRecordByToken(token)) {
            return true;
        }
        if (currentMissionListManager_->GetAbilityFromTerminateList(token)) {
            return true;
        }
    } else {
        if (currentStackManager_->GetAbilityRecordByToken(token)) {
            return true;
        }

        if (currentStackManager_->GetAbilityFromTerminateList(token)) {
            return true;
        }
    }

    if (dataAbilityManager_->GetAbilityRecordByToken(token)) {
        return true;
    }

    if (connectManager_->GetServiceRecordByToken(token)) {
        return true;
    }

    HILOG_ERROR("Failed to verify token.");
    return false;
}

bool AbilityManagerService::VerificationAllToken(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("VerificationAllToken.");

    if (useNewMission_) {
        for (auto item: missionListManagers_) {
            if (item.second && item.second->GetAbilityRecordByToken(token)) {
                return true;
            }

            if (item.second && item.second->GetAbilityFromTerminateList(token)) {
                return true;
            }
        }
    } else {
        for (auto item: stackManagers_) {
            if (item.second && item.second->GetAbilityRecordByToken(token)) {
                return true;
            }

            if (item.second && item.second->GetAbilityFromTerminateList(token)) {
                return true;
            }
        }
    }

    for (auto item: dataAbilityManagers_) {
        if (item.second && item.second->GetAbilityRecordByToken(token)) {
            return true;
        }
    }

    for (auto item: connectManagers_) {
        if (item.second && item.second->GetServiceRecordByToken(token)) {
            return true;
        }
    }

    HILOG_ERROR("Failed to verify all token.");
    return false;
}

const std::shared_ptr<DataAbilityManager> &AbilityManagerService::GetDataAbilityManager(
    const sptr<IAbilityScheduler> &scheduler)
{
    if (scheduler == nullptr) {
        HILOG_ERROR("the param ability scheduler is nullptr");
        return nullptr;
    }

    for (auto item: dataAbilityManagers_) {
        if (item.second && item.second->ContainsDataAbility(scheduler)) {
            return item.second;
        }
    }

    return nullptr;
}

std::shared_ptr<AbilityStackManager> AbilityManagerService::GetStackManagerByUserId(int32_t userId)
{
    auto it = stackManagers_.find(userId);
    if (it != stackManagers_.end()) {
        return it->second;
    }
    HILOG_ERROR("%{public}s, Failed to get Manager. UserId = %{public}d", __func__, userId);
    return nullptr;
}

std::shared_ptr<MissionListManager> AbilityManagerService::GetListManagerByUserId(int32_t userId)
{
    auto it = missionListManagers_.find(userId);
    if (it != missionListManagers_.end()) {
        return it->second;
    }
    HILOG_ERROR("%{public}s, Failed to get Manager. UserId = %{public}d", __func__, userId);
    return nullptr;
}

std::shared_ptr<AbilityConnectManager> AbilityManagerService::GetConnectManagerByUserId(int32_t userId)
{
    auto it = connectManagers_.find(userId);
    if (it != connectManagers_.end()) {
        return it->second;
    }
    HILOG_ERROR("%{public}s, Failed to get Manager. UserId = %{public}d", __func__, userId);
    return nullptr;
}

std::shared_ptr<DataAbilityManager> AbilityManagerService::GetDataAbilityManagerByUserId(int32_t userId)
{
    auto it = dataAbilityManagers_.find(userId);
    if (it != dataAbilityManagers_.end()) {
        return it->second;
    }
    HILOG_ERROR("%{public}s, Failed to get Manager. UserId = %{public}d", __func__, userId);
    return nullptr;
}

std::shared_ptr<AbilityStackManager> AbilityManagerService::GetStackManagerByToken(const sptr<IRemoteObject> &token)
{
    for (auto item: stackManagers_) {
        if (item.second && item.second->GetAbilityRecordByToken(token)) {
            return item.second;
        }

        if (item.second && item.second->GetAbilityFromTerminateList(token)) {
            return item.second;
        }
    }

    return nullptr;
}

std::shared_ptr<MissionListManager> AbilityManagerService::GetListManagerByToken(const sptr<IRemoteObject> &token)
{
    for (auto item: missionListManagers_) {
        if (item.second && item.second->GetAbilityRecordByToken(token)) {
            return item.second;
        }

        if (item.second && item.second->GetAbilityFromTerminateList(token)) {
            return item.second;
        }
    }

    return nullptr;
}

std::shared_ptr<AbilityConnectManager> AbilityManagerService::GetConnectManagerByToken(
    const sptr<IRemoteObject> &token)
{
    for (auto item: connectManagers_) {
        if (item.second && item.second->GetServiceRecordByToken(token)) {
            return item.second;
        }
    }

    return nullptr;
}

std::shared_ptr<DataAbilityManager> AbilityManagerService::GetDataAbilityManagerByToken(
    const sptr<IRemoteObject> &token)
{
    for (auto item: dataAbilityManagers_) {
        if (item.second && item.second->GetAbilityRecordByToken(token)) {
            return item.second;
        }
    }

    return nullptr;
}


int AbilityManagerService::MoveMissionToFloatingStack(const MissionOption &missionOption)
{
    HILOG_INFO("Move mission to floating stack.");
    return currentStackManager_->MoveMissionToFloatingStack(missionOption);
}

int AbilityManagerService::MoveMissionToSplitScreenStack(const MissionOption &primary, const MissionOption &secondary)
{
    HILOG_INFO("Move mission to split screen stack.");
    return currentStackManager_->MoveMissionToSplitScreenStack(primary, secondary);
}

int AbilityManagerService::ChangeFocusAbility(
    const sptr<IRemoteObject> &lostFocusToken, const sptr<IRemoteObject> &getFocusToken)
{
    HILOG_INFO("Change focus ability.");
    CHECK_POINTER_AND_RETURN(lostFocusToken, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(getFocusToken, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(currentStackManager_, ERR_INVALID_VALUE);

    if (!VerificationAllToken(lostFocusToken)) {
        return ERR_INVALID_VALUE;
    }

    if (!VerificationAllToken(getFocusToken)) {
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

    if (!VerificationAllToken(token)) {
        return false;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto stackManager = GetStackManagerByUserId(userId);
    if (!stackManager) {
        HILOG_ERROR("stackManager is nullptr. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return stackManager->IsFirstInMission(token);
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
    if (!VerificationAllToken(token)) {
        return;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    CHECK_POINTER(abilityRecord);

    auto stackManager = GetStackManagerByUserId(userId);
    if (!stackManager) {
        HILOG_ERROR("stackManager is nullptr. userId=%{public}d", userId);
        return;
    }
    stackManager->RestartAbility(abilityRecord);
}

void AbilityManagerService::NotifyBmsAbilityLifeStatus(
    const std::string &bundleName, const std::string &abilityName, const int64_t launchTime, const int uid)
{
    auto bundleManager = GetBundleManager();
    CHECK_POINTER(bundleManager);
    HILOG_INFO("NotifyBmsAbilityLifeStatus is called, uid :%{public}d", uid);
    bundleManager->NotifyAbilityLifeStatus(bundleName, abilityName, launchTime, uid);
}

void AbilityManagerService::StartSystemApplication()
{
    HILOG_DEBUG("%{public}s", __func__);

    ConnectBmsService();

    if (!amsConfigResolver_ || amsConfigResolver_->NonConfigFile()) {
        HILOG_INFO("start all");
        StartingSettingsDataAbility();
        StartingSystemUiAbility();
        return;
    }

    StartingSettingsDataAbility();
    StartingSystemUiAbility();
    StartupResidentProcess(U0_USER_ID);
}

void AbilityManagerService::StartingSystemUiAbility()
{
    HILOG_DEBUG("%{public}s", __func__);
    AppExecFwk::AbilityInfo systemUiInfo;
    if (!iBundleManager_) {
        HILOG_INFO("bms server is null");
        return;
    }
    Want systemUiWant;
    systemUiWant.SetElementName(AbilityConfig::SYSTEM_UI_BUNDLE_NAME, AbilityConfig::SYSTEM_UI_ABILITY_NAME);
    uint32_t waitCnt = 0;
    // Wait 10 minutes for the installation to complete.
    while (!iBundleManager_->QueryAbilityInfo(systemUiWant, systemUiInfo) && waitCnt < MAX_WAIT_SYSTEM_UI_NUM) {
        HILOG_INFO("Waiting query system ui info completed.");
        usleep(REPOLL_TIME_MICRO_SECONDS);
        waitCnt++;
    }
    (void)StartAbility(systemUiWant, U0_USER_ID, DEFAULT_INVAL_VALUE);
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

bool AbilityManagerService::CheckCallerIsSystemAppByIpc()
{
    HILOG_DEBUG("%{public}s begin", __func__);
    auto bms = GetBundleManager();
    CHECK_POINTER_RETURN_BOOL(bms);
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    HILOG_ERROR("callerUid %{public}d", callerUid);
    return bms->CheckIsSystemAppByUid(callerUid);
}

int AbilityManagerService::GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info)
{
    HILOG_INFO("Get pending request info.");
    CHECK_POINTER_AND_RETURN(pendingWantManager_, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(target, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(info, ERR_INVALID_VALUE);
    return pendingWantManager_->GetWantSenderInfo(target, info);
}

void AbilityManagerService::UpdateLockScreenState(bool isLockScreen)
{
    HILOG_DEBUG("%{public}s begin", __func__);
    CHECK_POINTER(currentStackManager_);
    currentStackManager_->UpdateLockScreenState(isLockScreen);
}

/**
 * Get system memory information.
 * @param SystemMemoryAttr, memory information.
 */
void AbilityManagerService::GetSystemMemoryAttr(AppExecFwk::SystemMemoryAttr &memoryInfo)
{
    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (appScheduler == nullptr) {
        HILOG_ERROR("%{public}s, appScheduler is nullptr", __func__);
        return;
    }

    int memoryThreshold = 0;
    if (amsConfigResolver_ == nullptr) {
        HILOG_ERROR("%{public}s, amsConfigResolver_ is nullptr", __func__);
        memoryThreshold = EXPERIENCE_MEM_THRESHOLD;
    } else {
        memoryThreshold = amsConfigResolver_->GetMemThreshold(AmsConfig::MemThreshold::HOME_APP);
    }

    nlohmann::json memJson = { "memoryThreshold", memoryThreshold };
    std::string memConfig = memJson.dump();

    appScheduler->GetSystemMemoryAttr(memoryInfo, memConfig);
}

int AbilityManagerService::GetAppMemorySize()
{
    HILOG_INFO("service GetAppMemorySize start");
    const char *key = "const.product.dalvikheaplimit";
    const char *def = "512m";
    char *valueGet = nullptr;
    unsigned int len = 128;
    int ret = GetParameter(key, def, valueGet, len);
    if ((ret != GET_PARAMETER_OTHER) && (ret != GET_PARAMETER_INCORRECT)) {
        int *size = 0;
        int len = strlen(valueGet);
        int index = 0;
        for (int i = 0; i < len; i++) {
            while (!(valueGet[i] > '0' && valueGet[i] < '9')) {
                i++;
            }
            while (valueGet[i] >= '0' && valueGet[i] < '9') {
                int t = valueGet[i] - '0';
                size[index] = size[index] * SIZE_10 + t;
                i++;
            }
            index++;
        }
        return *size;
    }
    return APP_MEMORY_SIZE;
}

bool AbilityManagerService::IsRamConstrainedDevice()
{
    HILOG_INFO("service IsRamConstrainedDevice start");
    const char *key = "const.product.islowram";
    const char *def = "0";
    char *valueGet = nullptr;
    unsigned int len = 128;
    int ret = GetParameter(key, def, valueGet, len);
    if ((ret != GET_PARAMETER_OTHER) && (ret != GET_PARAMETER_INCORRECT)) {
        int value = atoi(valueGet);
        if (value) {
            return true;
        } else {
            return isRamConstrainedDevice;
        }
    }
    return isRamConstrainedDevice;
}

int AbilityManagerService::GetMissionSaveTime() const
{
    if (!amsConfigResolver_) {
        return 0;
    }

    return amsConfigResolver_->GetMissionSaveTime();
}

int32_t AbilityManagerService::GetMissionIdByAbilityToken(const sptr<IRemoteObject> &token)
{
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        HILOG_ERROR("abilityRecord is Null.");
        return -1;
    }
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto missionListManager = GetListManagerByUserId(userId);
    if (!missionListManager) {
        HILOG_ERROR("missionListManager is Null. userId=%{public}d", userId);
        return -1;
    }
    return missionListManager->GetMissionIdByAbilityToken(token);
}

sptr<IRemoteObject> AbilityManagerService::GetAbilityTokenByMissionId(int32_t missionId)
{
    if (!currentMissionListManager_) {
        return nullptr;
    }
    return currentMissionListManager_->GetAbilityTokenByMissionId(missionId);
}

void AbilityManagerService::StartingSettingsDataAbility()
{
    HILOG_DEBUG("%{public}s", __func__);
    if (!iBundleManager_) {
        HILOG_INFO("bms service is null");
        return;
    }

    AppExecFwk::AbilityInfo abilityInfo;
    Want want;
    want.SetElementName(AbilityConfig::SETTINGS_DATA_BUNDLE_NAME, AbilityConfig::SETTINGS_DATA_ABILITY_NAME);
    uint32_t waitCnt = 0;
    while (!iBundleManager_->QueryAbilityInfo(want, OHOS::AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT,
        U0_USER_ID, abilityInfo) && waitCnt < MAX_WAIT_SETTINGS_DATA_NUM) {
        HILOG_INFO("Waiting query settings data info completed.");
        usleep(REPOLL_TIME_MICRO_SECONDS);
        waitCnt++;
    }

    // node: do not use abilityInfo.uri directly, need check uri first.
    auto GetValidUri = [&]() -> std::string {
        int32_t firstSeparator = abilityInfo.uri.find_first_of('/');
        int32_t lastSeparator = abilityInfo.uri.find_last_of('/');
        if (lastSeparator - firstSeparator != 1) {
            HILOG_ERROR("ability info uri error, uri: %{public}s", abilityInfo.uri.c_str());
            return "";
        }

        std::string uriStr = abilityInfo.uri;
        uriStr.insert(lastSeparator, "/");
        return uriStr;
    };

    std::string abilityUri = GetValidUri();
    if (abilityUri.empty()) {
        return;
    }

    HILOG_INFO("abilityInfo uri: %{public}s.", abilityUri.c_str());

    // start settings data ability
    Uri uri(abilityUri);
    (void)AcquireDataAbility(uri, true, nullptr);
}

int AbilityManagerService::StartRemoteAbilityByCall(const Want &want, const sptr<IRemoteObject> &connect)
{
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    uint32_t tokenCaller = IPCSkeleton::GetCallingTokenID();
    DistributedClient dmsClient;
    return dmsClient.StartRemoteAbilityByCall(want, connect, callerUid, callerPid, tokenCaller);
}

int AbilityManagerService::ReleaseRemoteAbility(const sptr<IRemoteObject> &connect,
    const AppExecFwk::ElementName &element)
{
    DistributedClient dmsClient;
    return dmsClient.ReleaseRemoteAbility(connect, element);
}

int AbilityManagerService::StartAbilityByCall(
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("call ability.");
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);

    if (CheckIfOperateRemote(want)) {
        HILOG_INFO("start remote ability by call");
        return StartRemoteAbilityByCall(want, connect->AsObject());
    }

    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.callerUid = IPCSkeleton::GetCallingUid();
    abilityRequest.callerToken = callerToken;
    abilityRequest.startSetting = nullptr;
    abilityRequest.want = want;
    abilityRequest.connect = connect;
    int result = GenerateAbilityRequest(want, -1, abilityRequest, callerToken, GetUserId());
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request error.");
        return result;
    }

    if (!abilityRequest.IsNewVersion()) {
        HILOG_ERROR("target ability compatible version is lower than 8.");
        return RESOLVE_CALL_ABILITY_VERSION_ERR;
    }

    result = CheckCallPermissions(abilityRequest);
    if (result != ERR_OK) {
        HILOG_ERROR("CheckCallPermissions fail, result: %{public}d", result);
        return RESOLVE_CALL_NO_PERMISSIONS;
    }

    return currentMissionListManager_->ResolveLocked(abilityRequest);
}

int AbilityManagerService::ReleaseAbility(
    const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element)
{
    HILOG_DEBUG("Release called ability.");

    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);

    std::string elementName = element.GetURI();
    HILOG_DEBUG("try to release called ability, name: %{public}s.", elementName.c_str());

    if (CheckIsRemote(element.GetDeviceID())) {
        HILOG_INFO("release remote ability");
        return ReleaseRemoteAbility(connect->AsObject(), element);
    }

    return currentMissionListManager_->ReleaseLocked(connect, element);
}

int AbilityManagerService::CheckCallPermissions(const AbilityRequest &abilityRequest)
{
    HILOG_DEBUG("%{public}s begin", __func__);
    auto abilityInfo = abilityRequest.abilityInfo;
    auto callerUid = abilityRequest.callerUid;
    auto targetUid = abilityInfo.applicationInfo.uid;
    if (AbilityUtil::ROOT_UID == callerUid) {
        HILOG_DEBUG("uid is root,ability cannot be called.");
        return RESOLVE_CALL_NO_PERMISSIONS;
    }
    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, GET_ABILITY_SERVICE_FAILED);
    auto isCallerSystemApp = bms->CheckIsSystemAppByUid(callerUid);
    auto isTargetSystemApp = bms->CheckIsSystemAppByUid(targetUid);
    HILOG_ERROR("isCallerSystemApp:%{public}d, isTargetSystemApp:%{public}d",
        isCallerSystemApp, isTargetSystemApp);
    if (callerUid != SYSTEM_UID && !isCallerSystemApp) {
        HILOG_DEBUG("caller is common app.");
        std::string bundleName;
        bool result = bms->GetBundleNameForUid(callerUid, bundleName);
        if (!result) {
            HILOG_ERROR("GetBundleNameForUid from bms fail.");
            return RESOLVE_CALL_NO_PERMISSIONS;
        }
        if (bundleName != abilityInfo.bundleName && callerUid != targetUid && !isTargetSystemApp) {
            HILOG_ERROR("the bundlename of caller is different from target one, caller: %{public}s "
                        "target: %{public}s",
                bundleName.c_str(),
                abilityInfo.bundleName.c_str());
            return RESOLVE_CALL_NO_PERMISSIONS;
        }
    } else {
        HILOG_DEBUG("caller is systemapp or system ability.");
    }
    HILOG_DEBUG("the caller has permission to resolve the callproxy of common ability.");
    // check whether the target ability is singleton mode and page type.
    if (abilityInfo.type == AppExecFwk::AbilityType::PAGE &&
        abilityInfo.launchMode == AppExecFwk::LaunchMode::SINGLETON) {
        HILOG_DEBUG("called ability is common ability and singleton.");
    } else {
        HILOG_ERROR("called ability is not common ability or singleton.");
        return RESOLVE_CALL_ABILITY_TYPE_ERR;
    }
    return ERR_OK;
}

int AbilityManagerService::SetMissionLabel(const sptr<IRemoteObject> &token, const std::string &label)
{
    HILOG_DEBUG("%{public}s", __func__);
    auto missionListManager = currentMissionListManager_;
    if (missionListManager) {
        missionListManager->SetMissionLabel(token, label);
    }
    return 0;
}

int AbilityManagerService::StartUser(int userId)
{
    HILOG_DEBUG("%{public}s, userId:%{public}d", __func__, userId);
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (userController_) {
        return userController_->StartUser(userId, true);
    }
    return 0;
}

int AbilityManagerService::StopUser(int userId, const sptr<IStopUserCallback> &callback)
{
    HILOG_DEBUG("%{public}s", __func__);
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    auto ret = -1;
    if (userController_) {
        ret = userController_->StopUser(userId);
        HILOG_DEBUG("ret = %{public}d", ret);
    }
    if (callback) {
        callback->OnStopUserDone(userId, ret);
    }
    return 0;
}

void AbilityManagerService::OnAcceptWantResponse(
    const AAFwk::Want &want, const std::string &flag)
{
    HILOG_DEBUG("On accept want response");
    if (!currentMissionListManager_) {
        return;
    }
    currentMissionListManager_->OnAcceptWantResponse(want, flag);
}

void AbilityManagerService::OnStartSpecifiedAbilityTimeoutResponse(const AAFwk::Want &want)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    if (!currentMissionListManager_) {
        return;
    }
    currentMissionListManager_->OnStartSpecifiedAbilityTimeoutResponse(want);
}

int AbilityManagerService::GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info)
{
    HILOG_DEBUG("Get running ability infos.");
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();

    currentMissionListManager_->GetAbilityRunningInfos(info, isPerm);
    connectManager_->GetAbilityRunningInfos(info, isPerm);
    dataAbilityManager_->GetAbilityRunningInfos(info, isPerm);

    return ERR_OK;
}

int AbilityManagerService::GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info)
{
    HILOG_DEBUG("Get extension infos, upperLimit : %{public}d", upperLimit);
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();

    connectManager_->GetExtensionRunningInfos(upperLimit, info, GetUserId(), isPerm);
    return ERR_OK;
}

int AbilityManagerService::GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info)
{
    return DelayedSingleton<AppScheduler>::GetInstance()->GetProcessRunningInfos(info);
}

int AbilityManagerService::GetProcessRunningInfosByUserId(
    std::vector<AppExecFwk::RunningProcessInfo> &info, int32_t userId)
{
    return DelayedSingleton<AppScheduler>::GetInstance()->GetProcessRunningInfosByUserId(info, userId);
}

void AbilityManagerService::ClearUserData(int32_t userId)
{
    HILOG_DEBUG("%{public}s", __func__);
    missionListManagers_.erase(userId);
    connectManagers_.erase(userId);
    dataAbilityManagers_.erase(userId);
    pendingWantManagers_.erase(userId);
}

int AbilityManagerService::RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return 0;
    }

    if (!currentMissionListManager_) {
        HILOG_ERROR("snapshot: currentMissionListManager_ is nullptr.");
        return INNER_ERR;
    }
    currentMissionListManager_->RegisterSnapshotHandler(handler);
    HILOG_INFO("snapshot: AbilityManagerService register snapshot handler success.");
    return ERR_OK;
}

int32_t AbilityManagerService::GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
    MissionSnapshot& missionSnapshot)
{
    if (VerifyMissionPermission() == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (CheckIsRemote(deviceId)) {
        HILOG_INFO("get remote mission snapshot.");
        return GetRemoteMissionSnapshotInfo(deviceId, missionId, missionSnapshot);
    }
    HILOG_INFO("get local mission snapshot.");
    if (!currentMissionListManager_) {
        HILOG_ERROR("snapshot: currentMissionListManager_ is nullptr.");
        return INNER_ERR;
    }
    auto token = GetAbilityTokenByMissionId(missionId);
    bool result = currentMissionListManager_->GetMissionSnapshot(missionId, token, missionSnapshot);
    if (!result) {
        return INNER_ERR;
    }
    return ERR_OK;
}

int32_t AbilityManagerService::GetRemoteMissionSnapshotInfo(const std::string& deviceId, int32_t missionId,
    MissionSnapshot& missionSnapshot)
{
    HILOG_INFO("GetRemoteMissionSnapshotInfo begin");
    std::unique_ptr<MissionSnapshot> missionSnapshotPtr = std::make_unique<MissionSnapshot>();
    DistributedClient dmsClient;
    int result = dmsClient.GetRemoteMissionSnapshotInfo(deviceId, missionId, missionSnapshotPtr);
    if (result != ERR_OK) {
        HILOG_ERROR("GetRemoteMissionSnapshotInfo failed, result = %{public}d", result);
        return result;
    }
    missionSnapshot = *missionSnapshotPtr;
    return ERR_OK;
}

void AbilityManagerService::StartFreezingScreen()
{
    HILOG_INFO("%{public}s", __func__);
}

void AbilityManagerService::StopFreezingScreen()
{
    HILOG_INFO("%{public}s", __func__);
}

void AbilityManagerService::UserStarted(int32_t userId)
{
    HILOG_INFO("%{public}s", __func__);
    InitConnectManager(userId, false);
    SetStackManager(userId, false);
    InitMissionListManager(userId, false);
    InitDataAbilityManager(userId, false);
    InitPendWantManager(userId, false);
}

void AbilityManagerService::SwitchToUser(int32_t oldUserId, int32_t userId)
{
    HILOG_INFO("%{public}s, oldUserId:%{public}d, newUserId:%{public}d", __func__, oldUserId, userId);
    SwitchManagers(userId);
    PauseOldUser(oldUserId);
    StartUserApps(userId);
}

void AbilityManagerService::SwitchManagers(int32_t userId, bool switchUser)
{
    HILOG_INFO("%{public}s, SwitchManagers:%{public}d-----begin", __func__, userId);
    InitConnectManager(userId, switchUser);
    SetStackManager(userId, switchUser);
    InitMissionListManager(userId, switchUser);
    InitDataAbilityManager(userId, switchUser);
    InitPendWantManager(userId, switchUser);
    HILOG_INFO("%{public}s, SwitchManagers:%{public}d-----end", __func__, userId);
}

void AbilityManagerService::PauseOldUser(int32_t userId)
{
    HILOG_INFO("%{public}s, PauseOldUser:%{public}d-----begin", __func__, userId);
    if (useNewMission_) {
        PauseOldMissionListManager(userId);
    } else {
        PauseOldStackManager(userId);
    }
    HILOG_INFO("%{public}s, PauseOldUser:%{public}d-----end", __func__, userId);
}

void AbilityManagerService::PauseOldMissionListManager(int32_t userId)
{
    HILOG_INFO("%{public}s, PauseOldMissionListManager:%{public}d-----begin", __func__, userId);
    auto it = missionListManagers_.find(userId);
    if (it == missionListManagers_.end()) {
        HILOG_INFO("%{public}s, PauseOldMissionListManager:%{public}d-----end1", __func__, userId);
        return;
    }
    auto manager = it->second;
    if (!manager) {
        HILOG_INFO("%{public}s, PauseOldMissionListManager:%{public}d-----end2", __func__, userId);
        return;
    }
    manager->PauseManager();
    HILOG_INFO("%{public}s, PauseOldMissionListManager:%{public}d-----end", __func__, userId);
}

void AbilityManagerService::PauseOldStackManager(int32_t userId)
{
    auto it = stackManagers_.find(userId);
    if (it == stackManagers_.end()) {
        return;
    }
    auto manager = it->second;
    if (!manager) {
        return;
    }
    manager->PauseManager();
}

void AbilityManagerService::StartUserApps(int32_t userId)
{
    HILOG_INFO("StartUserApps, userId:%{public}d, currentUserId:%{public}d", userId, GetUserId());
    if (useNewMission_) {
        if (currentMissionListManager_ && currentMissionListManager_->IsStarted()) {
            HILOG_INFO("missionListManager ResumeManager");
            currentMissionListManager_->ResumeManager();
            return;
        }
    } else {
        if (currentStackManager_ && currentStackManager_->IsStarted()) {
            HILOG_INFO("stack ResumeManager");
            currentStackManager_->ResumeManager();
            return;
        }
    }
    StartSystemAbilityByUser(userId);
}

void AbilityManagerService::StartSystemAbilityByUser(int32_t userId)
{
    HILOG_INFO("StartSystemAbilityByUser, userId:%{public}d, currentUserId:%{public}d", userId, GetUserId());
    ConnectBmsService();

    if (!amsConfigResolver_ || amsConfigResolver_->NonConfigFile()) {
        HILOG_INFO("start all");
        StartingLauncherAbility();
        StartingScreenLockAbility();
        return;
    }

    if (amsConfigResolver_->GetStartLauncherState()) {
        HILOG_INFO("start launcher");
        StartingLauncherAbility();
    }

    if (amsConfigResolver_->GetStartScreenLockState()) {
        StartingScreenLockAbility();
    }

    if (amsConfigResolver_->GetPhoneServiceState()) {
        HILOG_INFO("start phone service");
        StartingPhoneServiceAbility();
    }

    if (amsConfigResolver_->GetStartContactsState()) {
        HILOG_INFO("start contacts");
        StartingContactsAbility();
    }

    if (amsConfigResolver_->GetStartMmsState()) {
        HILOG_INFO("start mms");
        StartingMmsAbility();
    }
}

void AbilityManagerService::InitConnectManager(int32_t userId, bool switchUser)
{
    auto it = connectManagers_.find(userId);
    if (it == connectManagers_.end()) {
        auto manager = std::make_shared<AbilityConnectManager>();
        manager->SetEventHandler(handler_);
        connectManagers_.emplace(userId, manager);
        if (switchUser) {
            connectManager_ = manager;
        }
    } else {
        if (switchUser) {
            connectManager_ = it->second;
        }
    }
}

void AbilityManagerService::InitDataAbilityManager(int32_t userId, bool switchUser)
{
    auto it = dataAbilityManagers_.find(userId);
    if (it == dataAbilityManagers_.end()) {
        auto manager = std::make_shared<DataAbilityManager>();
        dataAbilityManagers_.emplace(userId, manager);
        if (switchUser) {
            dataAbilityManager_ = manager;
        }
    } else {
        if (switchUser) {
            dataAbilityManager_ = it->second;
        }
    }
}

void AbilityManagerService::InitPendWantManager(int32_t userId, bool switchUser)
{
    auto it = pendingWantManagers_.find(userId);
    if (it == pendingWantManagers_.end()) {
        auto manager = std::make_shared<PendingWantManager>();
        pendingWantManagers_.emplace(userId, manager);
        if (switchUser) {
            pendingWantManager_ = manager;
        }
    } else {
        if (switchUser) {
            pendingWantManager_ = it->second;
        }
    }
}

int32_t AbilityManagerService::GetValidUserId(const int32_t userId)
{
    HILOG_DEBUG("%{public}s  userId = %{public}d", __func__, userId);
    int32_t validUserId = userId;

    if (DEFAULT_INVAL_VALUE == userId) {
        validUserId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
        HILOG_DEBUG("%{public}s validUserId = %{public}d, CallingUid = %{public}d", __func__, validUserId,
            IPCSkeleton::GetCallingUid());
        if (validUserId == U0_USER_ID) {
            validUserId = GetUserId();
        }
    }
    return validUserId;
}

int AbilityManagerService::SetAbilityController(const sptr<IAbilityController> &abilityController,
    bool imAStabilityTest)
{
    HILOG_DEBUG("%{public}s, imAStabilityTest: %{public}d", __func__, imAStabilityTest);
    auto bms = GetBundleManager();
    if (bms == nullptr) {
        HILOG_ERROR("bms nullptr");
        return ERR_INVALID_VALUE;
    }
    std::string bundleName;
    int uid = IPCSkeleton::GetCallingUid();
    bms->GetBundleNameForUid(uid, bundleName);
    HILOG_INFO("%{public}s, bundleName: %{public}s, uid = %{public}d", __func__, bundleName.c_str(), uid);
    if (bms->CheckPermission(bundleName, PERMISSION_SET_ABILITY_CONTROLLER) == 0) {
        HILOG_ERROR("PERMISSION_SET_ABILITY_CONTROLLER check failed");
        return CHECK_PERMISSION_FAILED;
    }
    std::lock_guard<std::recursive_mutex> guard(globalLock_);
    abilityController_ = abilityController;
    controllerIsAStabilityTest_ = imAStabilityTest;
    HILOG_DEBUG("%{public}s, end", __func__);
    return ERR_OK;
}

int AbilityManagerService::SendANRProcessID(int pid)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    int anrTimeOut = amsConfigResolver_->GetANRTimeOutTime();
    auto timeoutTask = [pid]() {
        if (kill(pid, SIGKILL) != ERR_OK) {
            HILOG_ERROR("Kill app not response process failed");
        }
    };
    if (!SetANRMissionByProcessID(pid)) {
        HILOG_ERROR("Set app not response mission record failed");
    }
    handler_->PostTask(timeoutTask, "TIME_OUT_TASK", anrTimeOut);
    if (kill(pid, SIGUSR1) != ERR_OK) {
        HILOG_ERROR("Send sig to app not response process failed");
        return SEND_USR1_SIG_FAIL;
    }
    return ERR_OK;
}

bool AbilityManagerService::IsRunningInStabilityTest()
{
    std::lock_guard<std::recursive_mutex> guard(globalLock_);
    bool ret = abilityController_ != nullptr && controllerIsAStabilityTest_;
    HILOG_DEBUG("%{public}s, IsRunningInStabilityTest: %{public}d", __func__, ret);
    return ret;
}

bool AbilityManagerService::IsAbilityControllerStart(const Want &want, const std::string &bundleName)
{
    if (abilityController_ != nullptr && controllerIsAStabilityTest_) {
        HILOG_DEBUG("%{public}s, controllerIsAStabilityTest_: %{public}d", __func__, controllerIsAStabilityTest_);
        bool isStart = abilityController_->AllowAbilityStart(want, bundleName);
        if (!isStart) {
            HILOG_INFO("Not finishing start ability because controller starting: %{public}s", bundleName.c_str());
            return false;
        }
    }
    return true;
}

bool AbilityManagerService::IsAbilityControllerForeground(const std::string &bundleName)
{
    if (abilityController_ != nullptr && controllerIsAStabilityTest_) {
        HILOG_DEBUG("%{public}s, controllerIsAStabilityTest_: %{public}d", __func__, controllerIsAStabilityTest_);
        bool isResume = abilityController_->AllowAbilityBackground(bundleName);
        if (!isResume) {
            HILOG_INFO("Not finishing terminate ability because controller resuming: %{public}s", bundleName.c_str());
            return false;
        }
    }
    return true;
}

int32_t AbilityManagerService::InitAbilityInfoFromExtension(AppExecFwk::ExtensionAbilityInfo &extensionInfo,
    AppExecFwk::AbilityInfo &abilityInfo)
{
    abilityInfo.applicationName = extensionInfo.applicationInfo.name;
    abilityInfo.applicationInfo = extensionInfo.applicationInfo;
    abilityInfo.bundleName = extensionInfo.bundleName;
    abilityInfo.package = extensionInfo.moduleName;
    abilityInfo.moduleName = extensionInfo.moduleName;
    abilityInfo.name = extensionInfo.name;
    abilityInfo.srcEntrance = extensionInfo.srcEntrance;
    abilityInfo.srcPath = extensionInfo.srcEntrance;
    abilityInfo.iconPath = extensionInfo.icon;
    abilityInfo.iconId = extensionInfo.iconId;
    abilityInfo.label = extensionInfo.label;
    abilityInfo.labelId = extensionInfo.labelId;
    abilityInfo.description = extensionInfo.description;
    abilityInfo.descriptionId = extensionInfo.descriptionId;
    abilityInfo.permissions = extensionInfo.permissions;
    abilityInfo.readPermission = extensionInfo.readPermission;
    abilityInfo.writePermission = extensionInfo.writePermission;
    abilityInfo.extensionAbilityType = extensionInfo.type;
    abilityInfo.visible = extensionInfo.visible;
    abilityInfo.resourcePath = extensionInfo.resourcePath;
    abilityInfo.enabled = extensionInfo.enabled;
    abilityInfo.isModuleJson = true;
    abilityInfo.isStageBasedModel = true;
    abilityInfo.process = extensionInfo.process;
    abilityInfo.metadata = extensionInfo.metadata;
    abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    return 0;
}

int AbilityManagerService::StartUserTest(const Want &want, const sptr<IRemoteObject> &observer)
{
    HILOG_DEBUG("enter");
    if (observer == nullptr) {
        HILOG_ERROR("observer is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::string bundleName = want.GetStringParam("-p");
    if (bundleName.empty()) {
        HILOG_ERROR("Invalid bundle name");
        return ERR_INVALID_VALUE;
    }

    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, START_USER_TEST_FAIL);
    AppExecFwk::BundleInfo bundleInfo;
    if (!bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, U0_USER_ID)) {
        HILOG_ERROR("Failed to get bundle info by U0_USER_ID %{public}d.", U0_USER_ID);
        int32_t userId = GetUserId();
        if (!bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, GetUserId())) {
            HILOG_ERROR("Failed to get bundle info by userId %{public}d.", userId);
            return GET_BUNDLE_INFO_FAILED;
        }
    }

    int ret = KillProcess(bundleName);
    if (ret) {
        HILOG_ERROR("Failed to kill process.");
        return ret;
    }

    return DelayedSingleton<AppScheduler>::GetInstance()->StartUserTest(want, observer, bundleInfo);
}

int AbilityManagerService::FinishUserTest(const std::string &msg, const int &resultCode, const std::string &bundleName)
{
    HILOG_DEBUG("enter");
    if (bundleName.empty()) {
        HILOG_ERROR("Invalid bundle name.");
        return ERR_INVALID_VALUE;
    }
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    return DelayedSingleton<AppScheduler>::GetInstance()->FinishUserTest(msg, resultCode, bundleName, callingPid);
}

int AbilityManagerService::GetCurrentTopAbility(sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("enter");

    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, GET_ABILITY_SERVICE_FAILED);

    auto callerUid = IPCSkeleton::GetCallingUid();
    std::string bundleName;
    auto result = bms->GetBundleNameForUid(callerUid, bundleName);
    if (!result) {
        HILOG_ERROR("GetBundleNameForUid fail");
        return GET_BUNDLENAME_BY_UID_FAIL;
    }

    auto abilityRecord = currentMissionListManager_->GetCurrentTopAbility(bundleName);
    if (!abilityRecord) {
        HILOG_ERROR("Failed to get top ability");
        return ERR_INVALID_VALUE;
    }

    token = abilityRecord->GetToken();
    if (!token) {
        HILOG_ERROR("Failed to get token");
        return ERR_INVALID_VALUE;
    }

    HILOG_INFO("bundleName : %{public}s, abilityName : %{public}s",
        bundleName.data(), abilityRecord->GetAbilityInfo().name.data());
    return ERR_OK;
}

int AbilityManagerService::DelegatorDoAbilityForeground(const sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("enter");
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);

    auto missionId = GetMissionIdByAbilityToken(token);
    if (missionId < 0) {
        HILOG_ERROR("Invalid mission id.");
        return ERR_INVALID_VALUE;
    }

    return DelegatorMoveMissionToFront(missionId);
}

int AbilityManagerService::DelegatorDoAbilityBackground(const sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("enter");
    return MinimizeAbility(token, true);
}

int AbilityManagerService::DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag)
{
    HILOG_DEBUG("DoAbilityForeground, sceneFlag:%{public}d", flag);
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);
    if (!VerificationToken(token) && !VerificationAllToken(token)) {
        HILOG_ERROR("%{public}s token error.", __func__);
        return ERR_INVALID_VALUE;
    }

    std::lock_guard<std::recursive_mutex> guard(globalLock_);
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    int result = AbilityUtil::JudgeAbilityVisibleControl(abilityRecord->GetAbilityInfo());
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return result;
    }

    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::PAGE) {
        HILOG_ERROR("Cannot minimize except page ability.");
        return ERR_INVALID_VALUE;
    }

    if (!IsAbilityControllerForeground(abilityRecord->GetAbilityInfo().bundleName)) {
        HILOG_ERROR("IsAbilityControllerForeground false.");
        return ERR_WOULD_BLOCK;
    }

    abilityRecord->lifeCycleStateInfo_.sceneFlagBak = flag;
    abilityRecord->ProcessForegroundAbility(flag);
    return ERR_OK;
}

int AbilityManagerService::DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag)
{
    HILOG_DEBUG("DoAbilityBackground, sceneFlag:%{public}d", flag);
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    abilityRecord->lifeCycleStateInfo_.sceneFlag = flag;
    int ret = MinimizeAbility(token);
    abilityRecord->lifeCycleStateInfo_.sceneFlag = SCENE_FLAG_NORMAL;
    return ret;
}

int AbilityManagerService::DelegatorMoveMissionToFront(int32_t missionId)
{
    HILOG_INFO("enter missionId : %{public}d", missionId);
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);

    return currentMissionListManager_->MoveMissionToFront(missionId);
}

int32_t AbilityManagerService::ShowPickerDialog(const Want& want, int32_t userId)
{
    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, GET_ABILITY_SERVICE_FAILED);
    HILOG_INFO("share content: ShowPickerDialog, userId is %{public}d", userId);
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    bms->QueryAbilityInfos(want, AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, userId, abilityInfos);
    return Ace::UIServiceMgrClient::GetInstance()->ShowAppPickerDialog(want, abilityInfos, userId);
}

void AbilityManagerService::UpdateCallerInfo(Want& want)
{
    int32_t tokenId = IPCSkeleton::GetCallingTokenID();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    want.SetParam(Want::PARAM_RESV_CALLER_TOKEN, tokenId);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, callerUid);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, callerPid);
}

bool AbilityManagerService::JudgeMultiUserConcurrency(const AppExecFwk::AbilityInfo &info, const int32_t userId)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);

    if (userId == U0_USER_ID) {
        HILOG_DEBUG("%{public}s, userId is 0.", __func__);
        return true;
    }

    HILOG_DEBUG("userId : %{public}d, current userId : %{public}d", userId, GetUserId());

    // Only non-concurrent mode is supported
    bool concurrencyMode = CONCURRENCY_MODE_FALSE;
    if (!concurrencyMode) {
        return (userId == GetUserId());
    }

    return true;
}

void AbilityManagerService::StartingScreenLockAbility()
{
    HILOG_DEBUG("%{public}s", __func__);
    if (!iBundleManager_) {
        HILOG_INFO("bms service is null");
        return;
    }

    constexpr int maxAttemptNums = 5;
    auto userId = GetUserId();
    int attemptNums = 1;
    AppExecFwk::AbilityInfo screenLockInfo;
    Want screenLockWant;
    screenLockWant.SetElementName(AbilityConfig::SCREEN_LOCK_BUNDLE_NAME, AbilityConfig::SCREEN_LOCK_ABILITY_NAME);
    HILOG_DEBUG("%{public}s, QueryAbilityInfo, userId is %{public}d", __func__, userId);
    while (!(iBundleManager_->QueryAbilityInfo(screenLockWant,
        OHOS::AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT, userId, screenLockInfo)) &&
        attemptNums <= maxAttemptNums) {
        HILOG_INFO("Waiting query mms service completed.");
        usleep(REPOLL_TIME_MICRO_SECONDS);
        attemptNums++;
    }

    HILOG_INFO("attemptNums : %{public}d", attemptNums);
    if (attemptNums <= maxAttemptNums) {
        (void)StartAbility(screenLockWant, userId, DEFAULT_INVAL_VALUE);
    }
}

int AbilityManagerService::ForceTimeoutForTest(const std::string &abilityName, const std::string &state)
{
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != AbilityUtil::ROOT_UID) {
        HILOG_ERROR("calling uid has no permission to force timeout.");
        return INVALID_DATA;
    }
    if (abilityName.empty()) {
        HILOG_ERROR("abilityName is empty.");
        return INVALID_DATA;
    }
    if (abilityName == "clean") {
        timeoutMap_.clear();
        return ERR_OK;
    }
    if (state != AbilityRecord::ConvertAbilityState(AbilityState::INITIAL) &&
        state != AbilityRecord::ConvertAbilityState(AbilityState::FOREGROUND_NEW) &&
        state != AbilityRecord::ConvertAbilityState(AbilityState::BACKGROUND_NEW) &&
        state != AbilityRecord::ConvertAbilityState(AbilityState::TERMINATING)) {
        HILOG_ERROR("lifecycle state is invalid.");
        return INVALID_DATA;
    }
    timeoutMap_.insert(std::make_pair(state, abilityName));
    return ERR_OK;
}

int AbilityManagerService::CheckStaticCfgPermission(AppExecFwk::AbilityInfo &abilityInfo)
{
    auto tokenId = IPCSkeleton::GetCallingTokenID();

    if ((abilityInfo.type == AppExecFwk::AbilityType::EXTENSION &&
        abilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::DATASHARE) ||
        (abilityInfo.type == AppExecFwk::AbilityType::DATA)) {
        // just need check the read permission and write permission of extension ability or data ability
        if (!abilityInfo.readPermission.empty()) {
            int checkReadPermission = AccessTokenKit::VerifyAccessToken(tokenId, abilityInfo.readPermission);
            if (checkReadPermission == ERR_OK) {
                return AppExecFwk::Constants::PERMISSION_GRANTED;
            }
            HILOG_WARN("verify access token fail, read permission: %{public}s", abilityInfo.readPermission.c_str());
        }
        if (!abilityInfo.writePermission.empty()) {
            int checkWritePermission = AccessTokenKit::VerifyAccessToken(tokenId, abilityInfo.writePermission);
            if (checkWritePermission == ERR_OK) {
                return AppExecFwk::Constants::PERMISSION_GRANTED;
            }
            HILOG_WARN("verify access token fail, write permission: %{public}s", abilityInfo.writePermission.c_str());
        }

        if (!abilityInfo.readPermission.empty() || !abilityInfo.writePermission.empty()) {
            // 'readPermission' and 'writePermission' take precedence over 'permission'
            // when 'readPermission' or 'writePermission' is not empty, no need check 'permission'
            return AppExecFwk::Constants::PERMISSION_NOT_GRANTED;
        }
    }

    // verify permission if 'permission' is not empty
    if (abilityInfo.permissions.empty()) {
        return AppExecFwk::Constants::PERMISSION_GRANTED;
    }

    for (auto permission : abilityInfo.permissions) {
        if (AccessTokenKit::VerifyAccessToken(tokenId, permission)
            != AppExecFwk::Constants::PERMISSION_GRANTED) {
            HILOG_ERROR("verify access token fail, permission: %{public}s", permission.c_str());
            return AppExecFwk::Constants::PERMISSION_NOT_GRANTED;
        }
    }

    return AppExecFwk::Constants::PERMISSION_GRANTED;
}

bool AbilityManagerService::IsNeedTimeoutForTest(const std::string &abilityName, const std::string &state) const
{
    for (auto iter = timeoutMap_.begin(); iter != timeoutMap_.end(); iter++) {
        if (iter->first == state && iter->second == abilityName) {
            return true;
        }
    }
    return false;
}

bool AbilityManagerService::VerifyUriPermisson(const AbilityRequest &abilityRequest, const Want &want)
{
    if (abilityRequest.abilityInfo.extensionAbilityType != AppExecFwk::ExtensionAbilityType::FILESHARE) {
        HILOG_DEBUG("Only FILESHARE need to Verify uri permission.");
        return true;
    }
    auto uriStr = want.GetUri().ToString();
    auto uriVec = want.GetStringArrayParam(AbilityConfig::PARAMS_STREAM);
    uriVec.emplace_back(uriStr);
    auto targetTokenId = IPCSkeleton::GetCallingTokenID();
    auto uriPermMgrClient = AAFwk::UriPermissionManagerClient::GetInstance();
    for (auto str : uriVec) {
        Uri uri(str);
        if (uriPermMgrClient->VerifyUriPermission(uri, Want::FLAG_AUTH_WRITE_URI_PERMISSION, targetTokenId)) {
            return true;
        }
        if (uriPermMgrClient->VerifyUriPermission(uri, Want::FLAG_AUTH_READ_URI_PERMISSION, targetTokenId)) {
            return true;
        }
    }
    return false;
}

bool AbilityManagerService::SetANRMissionByProcessID(int pid)
{
    HILOG_INFO("start.");
    if (appScheduler_ == nullptr || currentMissionListManager_ == nullptr) {
        HILOG_ERROR("null point.");
        return false;
    }
    std::vector<sptr<IRemoteObject>> tokens;
    if (appScheduler_->GetAbilityRecordsByProcessID(pid, tokens) != ERR_OK) {
        HILOG_ERROR("Get ability record failed.");
        return false;
    }
    for (auto &item : tokens) {
        auto abilityRecord = currentMissionListManager_->GetAbilityRecordByToken(item);
        if (abilityRecord == nullptr) {
            HILOG_WARN("abilityRecord is nullptr.");
            continue;
        }
        auto mission = abilityRecord->GetMission();
        if (mission == nullptr) {
            HILOG_WARN("mission is nullptr.");
            continue;
        }
        mission->SetANRState();
    }
    return true;
}

void AbilityManagerService::StartupResidentProcess(int userId)
{
    // Location may change
    auto bms = GetBundleManager();
    if (bms == nullptr) {
        HILOG_ERROR("StartupResidentProcess bms is nullptr");
        return;
    }

    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    bool getBundleInfos = iBundleManager_->GetBundleInfos(OHOS::AppExecFwk::GET_BUNDLE_DEFAULT, bundleInfos, userId);
    if (!getBundleInfos) {
        HILOG_ERROR("get bundle infos failed");
        return;
    }

    HILOG_INFO("StartupResidentProcess GetBundleInfos size: %{public}u, userId: %{public}d",
        bundleInfos.size(), userId);

    StartMainElement(userId, bundleInfos);
    if (!bundleInfos.empty()) {
        DelayedSingleton<AppScheduler>::GetInstance()->StartupResidentProcess(bundleInfos);
    }
}

void AbilityManagerService::StartMainElement(int userId, std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    std::set<uint32_t> needEraseIndexSet;

    for (int i = 0; i< bundleInfos.size(); i++) {
        if (!bundleInfos[i].isKeepAlive) {
            needEraseIndexSet.insert(i);
            continue;
        }
        for (auto hapModuleInfo : bundleInfos[i].hapModuleInfos) {
            std::string mainElement;
            if (!hapModuleInfo.isModuleJson) {
                // old application model
                mainElement = hapModuleInfo.mainAbility;
                if (mainElement.empty()) {
                    continue;
                }
                needEraseIndexSet.insert(i);
                std::string uriStr;
                bool getDataAbilityInfo = GetDataAbilityUri(hapModuleInfo.abilityInfos, mainElement, uriStr);
                if (getDataAbilityInfo) {
                    // dataability, need use AcquireDataAbility
                    Uri uri(uriStr);
                    (void)AcquireDataAbility(uri, true, nullptr);
                    continue;
                }
            } else {
                // new application model
                mainElement = hapModuleInfo.mainElementName;
                if (mainElement.empty()) {
                    continue;
                }
                needEraseIndexSet.insert(i);
            }

            // startAbility
            AppExecFwk::AbilityInfo abilityInfo;
            Want want;
            want.SetElementName(hapModuleInfo.bundleName, mainElement);
            (void)StartAbility(want, userId, DEFAULT_INVAL_VALUE);
        }
    }

    // delete item which process has been started.
    for (auto iter = needEraseIndexSet.rbegin(); iter != needEraseIndexSet.rend(); iter++) {
        bundleInfos.erase(bundleInfos.begin() + *iter);
    }
}

bool AbilityManagerService::GetDataAbilityUri(const std::vector<AppExecFwk::AbilityInfo> &abilityInfos,
    const std::string &mainAbility, std::string &uri)
{
    if (abilityInfos.empty() || mainAbility.empty()) {
        HILOG_ERROR("abilityInfos or mainAbility is empty. mainAbility: %{public}s", mainAbility.c_str());
        return false;
    }

    for (auto abilityInfo : abilityInfos) {
        if (abilityInfo.type == AppExecFwk::AbilityType::DATA &&
            abilityInfo.name == mainAbility) {
            uri = abilityInfo.uri;
            return true;
        }
    }

    return false;
}

int AbilityManagerService::VerifyMissionPermission()
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (isSaCall) {
        return ERR_OK;
    }
    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        PermissionConstants::PERMISSION_MANAGE_MISSION);
    if (isCallingPerm) {
        HILOG_DEBUG("%{public}s: Permission verification succeeded.", __func__);
        return ERR_OK;
    }
    HILOG_ERROR("%{public}s: Permission verification failed", __func__);
    return CHECK_PERMISSION_FAILED;
}

void AbilityManagerService::GetAbilityRunningInfo(std::vector<AbilityRunningInfo> &info,
    std::shared_ptr<AbilityRecord> &abilityRecord)
{
    AbilityRunningInfo runningInfo;
    AppExecFwk::RunningProcessInfo processInfo;

    runningInfo.ability = abilityRecord->GetWant().GetElement();
    runningInfo.startTime = abilityRecord->GetStartTime();
    runningInfo.abilityState = static_cast<int>(abilityRecord->GetAbilityState());

    DelayedSingleton<AppScheduler>::GetInstance()->
        GetRunningProcessInfoByToken(abilityRecord->GetToken(), processInfo);
    runningInfo.pid = processInfo.pid_;
    runningInfo.uid = processInfo.uid_;
    runningInfo.processName = processInfo.processName_;
    info.emplace_back(runningInfo);
}

int AbilityManagerService::VerifyAccountPermission(int32_t userId)
{
    if ((userId < 0) || (userController_ && (userController_->GetCurrentUserId() == userId))) {
        return ERR_OK;
    }
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (isSaCall) {
        return ERR_OK;
    }
    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        PermissionConstants::PERMISSION_INTERACT_ACROSS_LOCAL_ACCOUNTS);
    if (isCallingPerm) {
        return ERR_OK;
    }
    HILOG_ERROR("%{public}s: Permission verification failed", __func__);
    return CHECK_PERMISSION_FAILED;
}
}  // namespace AAFwk
}  // namespace OHOS
