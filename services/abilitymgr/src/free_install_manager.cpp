/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "free_install_manager.h"

#include "ability_info.h"
#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "ability_util.h"
#include "atomic_service_status_callback.h"
#include "distributed_client.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
const std::u16string DMS_FREE_INSTALL_CALLBACK_TOKEN = u"ohos.DistributedSchedule.IDmsFreeInstallCallback";
constexpr uint32_t IDMS_CALLBACK_ON_FREE_INSTALL_DONE = 0;
FreeInstallManager::FreeInstallManager(const std::weak_ptr<AbilityManagerService> &server)
    : server_(server)
{
}

bool FreeInstallManager::CheckIsFreeInstall(const Want &want)
{
    HILOG_INFO("%{public}s", __func__);
    auto flags = want.GetFlags();
    if ((flags & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        HILOG_INFO("StartAbility with free install flags");
        return true;
    }
    return false;
}

bool FreeInstallManager::IsTopAbility(const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s", __func__);
    auto server = server_.lock();
    CHECK_POINTER_AND_RETURN_LOG(server, false, "Get server failed!");
    AppExecFwk::ElementName elementName = server->GetTopAbility();
    if (elementName.GetBundleName().empty() || elementName.GetAbilityName().empty()) {
        HILOG_ERROR("GetBundleName or GetAbilityName empty!");
        return false;
    }

    auto caller = Token::GetAbilityRecordByToken(callerToken);
    if (caller == nullptr) {
        HILOG_ERROR("Caller is null!");
        return false;
    }

    auto type = caller->GetAbilityInfo().type;
    if (type == AppExecFwk::AbilityType::SERVICE || type == AppExecFwk::AbilityType::EXTENSION) {
        HILOG_INFO("The ability is service or extension ability.");
        return true;
    }

    AppExecFwk::ElementName callerElementName = caller->GetWant().GetElement();
    std::string callerBundleName;
    std::string callerAbilityName;
    callerBundleName = callerElementName.GetBundleName();
    callerAbilityName = callerElementName.GetAbilityName();
    if (elementName.GetBundleName().compare(callerBundleName) == 0 &&
        elementName.GetAbilityName().compare(callerAbilityName) == 0) {
        HILOG_INFO("The ability is top ability.");
        return true;
    }

    return false;
}

bool FreeInstallManager::CheckTargetBundleList(const Want &want, int32_t userId,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s", __func__);
    auto caller = Token::GetAbilityRecordByToken(callerToken);
    if (caller == nullptr) {
        HILOG_ERROR("Caller is null!");
        return false;
    }

    AppExecFwk::ElementName callerElementName = caller->GetWant().GetElement();
    AppExecFwk::ApplicationInfo appInfo = {};
    std::string callerBundleName;
    auto bms = AbilityUtil::GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, false);
    callerBundleName = callerElementName.GetBundleName();
    if (callerBundleName.compare(want.GetBundle()) == 0) {
        HILOG_INFO("Free install in the same application.");
        return true;
    }

    HILOG_INFO("Free install with another application.");
    constexpr auto appInfoflag = AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO;
    if (!bms->GetApplicationInfo(callerBundleName, appInfoflag, userId, appInfo)) {
        HILOG_ERROR("Cannot find caller ApplicationInfo!");
        return false;
    }

    if (appInfo.targetBundleList.empty()) {
        HILOG_ERROR("targetBundleList size is empty!");
        return false;
    }

    auto it = std::find(appInfo.targetBundleList.begin(), appInfo.targetBundleList.end(), want.GetBundle());
    if (it != appInfo.targetBundleList.end()) {
        return true;
    }

    return false;
}

int FreeInstallManager::FreeInstall(const Want &want, int32_t userId, int requestCode,
    const sptr<IRemoteObject> &callerToken, bool ifOperateRemote)
{
    bool isFromRemote = want.GetBoolParam(FROM_REMOTE_KEY, false);
    if (!isFromRemote && !IsTopAbility(callerToken)) {
        return HandleFreeInstallErrorCode(NOT_TOP_ABILITY);
    }
    if (!isFromRemote && !CheckTargetBundleList(want, userId, callerToken)) {
        return HandleFreeInstallErrorCode(TARGET_BUNDLE_NOT_EXIST);
    }
    auto promise = std::make_shared<std::promise<int32_t>>();
    FreeInstallInfo info = {
        .want = want,
        .userId = userId,
        .requestCode = requestCode,
        .callerToken = callerToken,
        .promise = promise
    };
    freeInstallList_.push_back(info);
    sptr<AtomicServiceStatusCallback> callback = new AtomicServiceStatusCallback(weak_from_this());
    if (ifOperateRemote) {
        int32_t callerUid = IPCSkeleton::GetCallingUid();
        uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
        DistributedClient dmsClient;
        auto result = dmsClient.StartRemoteFreeInstall(want, callerUid, requestCode, accessToken, callback);
        if (result != ERR_NONE) {
            return HandleFreeInstallErrorCode(result);
        }
        auto remoteFuture = info.promise->get_future();
        std::future_status remoteStatus = remoteFuture.wait_for(std::chrono::milliseconds(
            DELAY_REMOTE_FREE_INSTALL_TIMEOUT));
        if (remoteStatus == std::future_status::timeout) {
            return HandleFreeInstallErrorCode(FREE_INSTALL_TIMEOUT);
        }
        return HandleFreeInstallErrorCode(remoteFuture.get());
    }
    auto bms = AbilityUtil::GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, GET_ABILITY_SERVICE_FAILED);
    AppExecFwk::AbilityInfo abilityInfo = {};
    constexpr auto flag = AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION;
    if (bms->QueryAbilityInfo(want, flag, userId, abilityInfo, callback)) {
        HILOG_INFO("The app has installed.");
    }
    auto future = info.promise->get_future();
    std::future_status status = future.wait_for(std::chrono::milliseconds(
        DELAY_LOCAL_FREE_INSTALL_TIMEOUT));
    if (status == std::future_status::timeout) {
        info.isInstalled = true;
        return HandleFreeInstallErrorCode(FREE_INSTALL_TIMEOUT);
    }
    return HandleFreeInstallErrorCode(future.get());
}

int FreeInstallManager::NotifyDmsCallback(const Want &want, int resultCode)
{
    if (dmsFreeInstallCbs_.empty()) {
        HILOG_ERROR("Has no dms callback.");
        return ERR_INVALID_VALUE;
    }

    MessageParcel reply;
    MessageOption option;

    for (auto it = dmsFreeInstallCbs_.begin(); it != dmsFreeInstallCbs_.end();) {
        std::string abilityName = (*it).want.GetElement().GetAbilityName();
        if (want.GetElement().GetAbilityName().compare(abilityName) == 0) {
            HILOG_INFO("Handle DMS.");
            MessageParcel data;
            if (!data.WriteInterfaceToken(DMS_FREE_INSTALL_CALLBACK_TOKEN)) {
                HILOG_ERROR("Write interface token failed.");
                return ERR_INVALID_VALUE;
            }

            if (!data.WriteInt32(resultCode)) {
                HILOG_ERROR("Write resultCode error.");
                return ERR_INVALID_VALUE;
            }

            if (!data.WriteParcelable(&((*it).want))) {
                HILOG_ERROR("want write failed.");
                return INNER_ERR;
            }

            if (!data.WriteInt32((*it).requestCode)) {
                HILOG_ERROR("Write resultCode error.");
                return ERR_INVALID_VALUE;
            }

            (*it).dmsCallback->SendRequest(IDMS_CALLBACK_ON_FREE_INSTALL_DONE, data, reply, option);
            it = dmsFreeInstallCbs_.erase(it);
        } else {
            it++;
        }
    }

    return reply.ReadInt32();
}

void FreeInstallManager::NotifyFreeInstallResult(const Want &want, int resultCode)
{
    if (freeInstallList_.empty()) {
        HILOG_INFO("Has no app callback.");
        return;
    }

    bool isFromRemote = want.GetBoolParam(FROM_REMOTE_KEY, false);
    HILOG_INFO("isFromRemote = %{public}d", isFromRemote);
    for (auto it = freeInstallList_.begin(); it != freeInstallList_.end();) {
        std::string abilityName = (*it).want.GetElement().GetAbilityName();
        if (want.GetElement().GetAbilityName().compare(abilityName) != 0) {
            it++;
            continue;
        }

        if ((*it).isInstalled) {
            it = freeInstallList_.erase(it);
            continue;
        }

        std::string freeInstallType = want.GetStringParam(FREE_INSTALL_TYPE);
        if (!isFromRemote && resultCode == ERR_OK && freeInstallType == "ConnectAbility") {
            resultCode = ERR_OK;
        } else if (!isFromRemote && resultCode == ERR_OK && freeInstallType == "StartAbility") {
            HILOG_INFO("Handle apps startability.");
            auto server = server_.lock();
            CHECK_POINTER(server);
            if ((*it).want.GetBoolParam(FREE_INSTALL_UPGRADED_KEY, false)) {
                HILOG_INFO("Handle apps upgraded.");
                resultCode = server->StartAbilityInner((*it).want, nullptr, (*it).requestCode, -1, -1);
            }
            resultCode = server->StartAbilityInner((*it).want, (*it).callerToken, (*it).requestCode, -1, -1);
        }

        if ((*it).promise != nullptr) {
            HILOG_INFO("Handle apps setvalue done.");
            (*it).promise->set_value(resultCode);
        }

        (*it).isInstalled = true;
        it++;
    }
}

int FreeInstallManager::FreeInstallAbilityFromRemote(const Want &want, const sptr<IRemoteObject> &callback,
    int32_t userId, int requestCode)
{
    HILOG_INFO("%{public}s", __func__);
    if (callback == nullptr) {
        HILOG_ERROR("FreeInstallAbilityFromRemote callback is nullptr.");
        return ERR_INVALID_VALUE;
    }

    FreeInstallInfo info = {
        .want = want,
        .userId = userId,
        .requestCode = requestCode,
        .dmsCallback = callback
    };
    dmsFreeInstallCbs_.push_back(info);

    auto freeInstallTask = [manager = shared_from_this(), info]() {
        auto result = manager->FreeInstall(info.want, info.userId, info.requestCode, nullptr, false);
        if (result != ERR_OK) {
            manager->NotifyDmsCallback(info.want, result);
        }
    };

    std::shared_ptr<AbilityEventHandler> handler =
        DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");

    handler->PostTask(freeInstallTask, "FreeInstallAbilityFromRemote");
    return ERR_OK;
}

int FreeInstallManager::HandleFreeInstallErrorCode(int resultCode)
{
    auto it = FIErrorStrs.find(static_cast<enum NativeFreeInstallError>(resultCode));
    if (it != FIErrorStrs.end()) {
        HILOG_ERROR("Error code : %{public}d, info: %{public}s", resultCode, it->second.c_str());
    }

    auto itToApp = FIErrorToAppMaps.find(static_cast<enum NativeFreeInstallError>(resultCode));
    if (itToApp == FIErrorToAppMaps.end()) {
        HILOG_ERROR("Undefind error code.");
        return resultCode;
    }
    return itToApp->second;
}

int FreeInstallManager::IsConnectFreeInstall(const Want &want, int32_t userId,
    const sptr<IRemoteObject> &callerToken, std::string& localDeviceId)
{
    if (CheckIsFreeInstall(want)) {
        auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
        AppExecFwk::AbilityType type = abilityRecord->GetAbilityInfo().type;
        if (type == AppExecFwk::AbilityType::PAGE) {
            if (!IsTopAbility(callerToken)) {
                return NOT_TOP_ABILITY;
            }
        }
        std::string wantBundleName = want.GetElement().GetBundleName();
        std::string wantAbilityName = want.GetElement().GetAbilityName();
        std::string wantDeviceId = want.GetElement().GetDeviceID();
        std::string wantModuleName = want.GetStringParam("moduleName");
        if (!(localDeviceId == wantDeviceId || wantDeviceId.empty())) {
            HILOG_ERROR("AbilityManagerService::IsConnectFreeInstall. wantDeviceId error");
            return ERR_INVALID_VALUE;
        }

        if (wantBundleName.empty() || wantAbilityName.empty()) {
            HILOG_ERROR("AbilityManagerService::IsConnectFreeInstall. wantBundleName or wantAbilityName is empty");
            return ERR_INVALID_VALUE;
        }
        auto bms = AbilityUtil::GetBundleManager();
        CHECK_POINTER_AND_RETURN(bms, GET_ABILITY_SERVICE_FAILED);
        int callerUid = IPCSkeleton::GetCallingUid();
        std::string LocalBundleName;
        bms->GetBundleNameForUid(callerUid, LocalBundleName);
        if (LocalBundleName != wantBundleName) {
            HILOG_ERROR("AbilityManagerService::IsConnectFreeInstall. wantBundleName is not local BundleName");
            return ERR_INVALID_VALUE;
        }
        AppExecFwk::AbilityInfo abilityInfo;
        if (!(bms->QueryAbilityInfo(want, AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, userId,
            abilityInfo))) {
            HILOG_INFO("AbilityManagerService::IsConnectFreeInstall. try to FreeInstall");
            int result = FreeInstall(want, userId, DEFAULT_INVAL_VALUE, callerToken, false);
            if (result) {
                HILOG_ERROR("AbilityManagerService::IsConnectFreeInstall. FreeInstall error");
                return result;
            }
            HILOG_INFO("AbilityManagerService::IsConnectFreeInstall. FreeInstall success");
        }
    }
    return ERR_OK;
}

void FreeInstallManager::OnInstallFinished(int resultCode, const Want &want, int32_t userId)
{
    HILOG_INFO("%{public}s resultCode = %{public}d", __func__, resultCode);
    NotifyDmsCallback(want, resultCode);
    NotifyFreeInstallResult(want, resultCode);

    if (resultCode == ERR_OK) {
        auto updateAtmoicServiceTask = [want, userId]() {
            auto bms = AbilityUtil::GetBundleManager();
            CHECK_POINTER(bms);
            bms->UpgradeAtomicService(want, userId);
        };

        std::shared_ptr<AbilityEventHandler> handler =
            DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
        CHECK_POINTER_LOG(handler, "Fail to get AbilityEventHandler.");
        handler->PostTask(updateAtmoicServiceTask, "UpdateAtmoicServiceTask");
    }
}

void FreeInstallManager::OnRemoteInstallFinished(int resultCode, const Want &want, int32_t userId)
{
    HILOG_INFO("%{public}s resultCode = %{public}d", __func__, resultCode);
    NotifyFreeInstallResult(want, resultCode);
}
}  // namespace AAFwk
}  // namespace OHOS
