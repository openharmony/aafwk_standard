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

#include "connection_manager.h"
#include "ability_connection.h"
#include "ability_context.h"
#include "ability_manager_client.h"
#include "dfx_dump_catcher.h"
#include "hichecker.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::HiviewDFX;
ConnectionManager& ConnectionManager::GetInstance()
{
    static ConnectionManager connectionManager;
    return connectionManager;
}

ErrCode ConnectionManager::ConnectAbility(const sptr<IRemoteObject> &connectCaller,
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback)
{
    if (connectCaller == nullptr || connectCallback == nullptr) {
        HILOG_ERROR("%{public}s, connectCaller or connectCallback is nullptr.", __func__);
        return ERR_INVALID_VALUE;
    }

    AppExecFwk::ElementName connectReceiver = want.GetElement();
    HILOG_DEBUG("%{public}s begin, connectCaller: %{public}p, connectReceiver: %{public}s.",
        __func__, connectCaller.GetRefPtr(),
        (connectReceiver.GetBundleName() + ":" + connectReceiver.GetAbilityName()).c_str());

    sptr<AbilityConnection> abilityConnection;
    auto item = std::find_if(abilityConnections_.begin(), abilityConnections_.end(),
        [&connectCaller, &connectReceiver](const std::map<ConnectionInfo,
            std::vector<sptr<AbilityConnectCallback>>>::value_type &obj) {
                return connectCaller == obj.first.connectCaller &&
                    connectReceiver.GetBundleName() == obj.first.connectReceiver.GetBundleName() &&
                    connectReceiver.GetAbilityName() == obj.first.connectReceiver.GetAbilityName();
        });
    if (item != abilityConnections_.end()) {
        std::vector<sptr<AbilityConnectCallback>> callbacks = item->second;
        callbacks.push_back(connectCallback);
        abilityConnections_[item->first] = callbacks;
        abilityConnection = item->first.abilityConnection;
        abilityConnection->SetConnectCallback(connectCallback);
        HILOG_INFO("%{public}s end, find abilityConnection:%{public}p exist, callbackSize:%{public}d.",
            __func__, abilityConnection.GetRefPtr(), (int32_t)callbacks.size());
        if (abilityConnection->GetResultCode() == ERR_OK) {
            connectCallback->OnAbilityConnectDone(connectReceiver, abilityConnection->GetRemoteObject(),
                abilityConnection->GetResultCode());
            return ERR_OK;
        } else {
            return HandleCallbackTimeOut(connectCaller, want, connectReceiver, abilityConnection, connectCallback);
        }
    } else {
        abilityConnection = new AbilityConnection(connectCallback);
        ErrCode ret =
            AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(want, abilityConnection, connectCaller);
        if (ret == ERR_OK) {
            ConnectionInfo connectionInfo(connectCaller, connectReceiver, abilityConnection);
            std::vector<sptr<AbilityConnectCallback>> callbacks;
            callbacks.push_back(connectCallback);
            abilityConnections_[connectionInfo] = callbacks;
        }
        HILOG_DEBUG("%{public}s end, not find connection, connection: %{public}p, abilityConnectionsSize:%{public}d.",
            __func__, abilityConnection.GetRefPtr(), (int32_t)abilityConnections_.size());
        return ret;
    }
}

ErrCode ConnectionManager::ConnectAbilityWithAccount(const sptr<IRemoteObject> &connectCaller,
    const AAFwk::Want &want, int accountId, const sptr<AbilityConnectCallback> &connectCallback)
{
    if (connectCaller == nullptr || connectCallback == nullptr) {
        HILOG_ERROR("%{public}s, connectCaller or connectCallback is nullptr.", __func__);
        return ERR_INVALID_VALUE;
    }

    AppExecFwk::ElementName connectReceiver = want.GetElement();
    HILOG_DEBUG("%{public}s begin, connectCaller: %{public}p, connectReceiver: %{public}s.",
        __func__, connectCaller.GetRefPtr(),
        (connectReceiver.GetBundleName() + ":" + connectReceiver.GetAbilityName()).c_str());

    sptr<AbilityConnection> abilityConnection;
    auto item = std::find_if(abilityConnections_.begin(), abilityConnections_.end(),
        [&connectCaller, &connectReceiver](const std::map<ConnectionInfo,
            std::vector<sptr<AbilityConnectCallback>>>::value_type &obj) {
                return connectCaller == obj.first.connectCaller &&
                    connectReceiver.GetBundleName() == obj.first.connectReceiver.GetBundleName() &&
                    connectReceiver.GetAbilityName() == obj.first.connectReceiver.GetAbilityName();
        });
    if (item != abilityConnections_.end()) {
        std::vector<sptr<AbilityConnectCallback>> callbacks = item->second;
        callbacks.push_back(connectCallback);
        abilityConnections_[item->first] = callbacks;
        abilityConnection = item->first.abilityConnection;
        abilityConnection->SetConnectCallback(connectCallback);
        HILOG_INFO("%{public}s end, find abilityConnection:%{public}p exist, callbackSize:%{public}d.",
            __func__, abilityConnection.GetRefPtr(), (int32_t)callbacks.size());
        if (abilityConnection->GetResultCode() == ERR_OK) {
            connectCallback->OnAbilityConnectDone(connectReceiver, abilityConnection->GetRemoteObject(),
                abilityConnection->GetResultCode());
            return ERR_OK;
        } else {
            return HandleCallbackTimeOut(connectCaller, want, connectReceiver, abilityConnection, connectCallback);
        }
    } else {
        abilityConnection = new AbilityConnection(connectCallback);
        ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(
            want, abilityConnection, connectCaller, accountId);
        if (ret == ERR_OK) {
            ConnectionInfo connectionInfo(connectCaller, connectReceiver, abilityConnection);
            std::vector<sptr<AbilityConnectCallback>> callbacks;
            callbacks.push_back(connectCallback);
            abilityConnections_[connectionInfo] = callbacks;
        }
        HILOG_DEBUG("%{public}s end, not find connection, connection: %{public}p, abilityConnectionsSize:%{public}d.",
            __func__, abilityConnection.GetRefPtr(), (int32_t)abilityConnections_.size());
        return ret;
    }
}

ErrCode ConnectionManager::DisconnectAbility(const sptr<IRemoteObject> &connectCaller,
    const AppExecFwk::ElementName &connectReceiver, const sptr<AbilityConnectCallback> &connectCallback)
{
    if (connectCaller == nullptr || connectCallback == nullptr) {
        HILOG_ERROR("%{public}s, connectCaller or connectCallback is nullptr.", __func__);
        return ERR_INVALID_VALUE;
    }

    HILOG_DEBUG("%{public}s begin, connectCaller: %{public}p, connectReceiver: %{public}s.",
        __func__, connectCaller.GetRefPtr(),
        (connectReceiver.GetBundleName() + ":" + connectReceiver.GetAbilityName()).c_str());

    auto item = std::find_if(abilityConnections_.begin(), abilityConnections_.end(),
        [&connectCaller, &connectReceiver](
            const std::map<ConnectionInfo, std::vector<sptr<AbilityConnectCallback>>>::value_type &obj) {
            return connectCaller == obj.first.connectCaller &&
                   connectReceiver.GetBundleName() == obj.first.connectReceiver.GetBundleName() &&
                   connectReceiver.GetAbilityName() == obj.first.connectReceiver.GetAbilityName();
        });
    if (item != abilityConnections_.end()) {
        std::vector<sptr<AbilityConnectCallback>> callbacks = item->second;
        HILOG_DEBUG("%{public}s begin remove callback, callbackSize:%{public}d.", __func__, (int32_t)callbacks.size());
        auto iter = callbacks.begin();
        while (iter != callbacks.end()) {
            if (*iter == connectCallback) {
                iter = callbacks.erase(iter);
            } else {
                iter++;
            }
        }
        abilityConnections_[item->first] = callbacks;
        sptr<AbilityConnection> abilityConnection;
        abilityConnection = item->first.abilityConnection;
        abilityConnection->SetConnectCallback(connectCallback);
        HILOG_INFO("%{public}s end, find abilityConnection:%{public}p exist, abilityConnectionsSize:%{public}d.",
            __func__, abilityConnection.GetRefPtr(), (int32_t)abilityConnections_.size());
        if (callbacks.size() == 0) {
            abilityConnections_.erase(item);
            HILOG_DEBUG("%{public}s disconnectAbility.", __func__);
            return AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(abilityConnection);
        } else {
            connectCallback->OnAbilityDisconnectDone(connectReceiver, ERR_OK);
            HILOG_DEBUG("%{public}s callbacks is not empty, do not need disconnectAbility.", __func__);
            return ERR_OK;
        }
    } else {
        HILOG_ERROR("%{public}s not find conn exist.", __func__);
        return ERR_INVALID_VALUE;
    }
}

bool ConnectionManager::DisconnectCaller(const sptr<IRemoteObject> &connectCaller)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (connectCaller == nullptr) {
        HILOG_ERROR("%{public}s end, connectCaller is nullptr.", __func__);
        return false;
    }

    HILOG_DEBUG("%{public}s, connectCaller:%{public}p, abilityConnectionsSize:%{public}d.",
        __func__, connectCaller.GetRefPtr(), (int32_t)abilityConnections_.size());

    bool isDisconnect = false;
    auto iter = abilityConnections_.begin();
    while (iter != abilityConnections_.end()) {
        ConnectionInfo connectionInfo = iter->first;
        if (IsConnectCallerEqual(connectionInfo.connectCaller, connectCaller)) {
            HILOG_DEBUG("%{public}s DisconnectAbility connection:%{public}p.",
                __func__, connectionInfo.abilityConnection.GetRefPtr());
            ErrCode ret =
                AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(connectionInfo.abilityConnection);
            if (ret != ERR_OK) {
                HILOG_ERROR("%{public}s ams->DisconnectAbility error, ret=%{public}d", __func__, ret);
            }
            iter = abilityConnections_.erase(iter);
            isDisconnect = true;
        } else {
            ++iter;
        }
    }

    HILOG_DEBUG("%{public}s end, abilityConnectionsSize:%{public}d.", __func__, (int32_t)abilityConnections_.size());
    return isDisconnect;
}

bool ConnectionManager::DisconnectReceiver(const AppExecFwk::ElementName &connectReceiver)
{
    HILOG_DEBUG("%{public}s begin, abilityConnectionsSize:%{public}d, bundleName:%{public}s, abilityName:%{public}s.",
        __func__, (int32_t)abilityConnections_.size(), connectReceiver.GetBundleName().c_str(),
        connectReceiver.GetAbilityName().c_str());

    bool isDisconnect = false;
    auto iter = abilityConnections_.begin();
    while (iter != abilityConnections_.end()) {
        ConnectionInfo connectionInfo = iter->first;
        if (IsConnectReceiverEqual(connectionInfo.connectReceiver, connectReceiver)) {
            iter = abilityConnections_.erase(iter);
            isDisconnect = true;
        } else {
            ++iter;
        }
    }

    HILOG_DEBUG("%{public}s end, abilityConnectionsSize:%{public}d.", __func__, (int32_t)abilityConnections_.size());
    return isDisconnect;
}

void ConnectionManager::ReportConnectionLeakEvent(const int pid, const int tid)
{
    HILOG_DEBUG("%{public}s begin, pid:%{public}d, tid:%{public}d.", __func__, pid, tid);
    if (HiChecker::Contains(Rule::RULE_CHECK_ABILITY_CONNECTION_LEAK)) {
        DfxDumpCatcher dumplog;
        std::string stackTrace;
        bool ret = dumplog.DumpCatch(pid, tid, stackTrace);
        if (ret) {
            std::string cautionMsg = "TriggerRule:RULE_CHECK_ABILITY_CONNECTION_LEAK-pid=" +
                std::to_string(pid) + "-tid=" + std::to_string(tid) + ", has leaked connection" +
                ", Are you missing a call to DisconnectAbility()";
            HILOG_DEBUG("%{public}s cautionMsg:%{public}s.", __func__, cautionMsg.c_str());
            Caution caution(Rule::RULE_CHECK_ABILITY_CONNECTION_LEAK, cautionMsg, stackTrace);
            HiChecker::NotifyAbilityConnectionLeak(caution);
        } else {
            HILOG_ERROR("%{public}s dumpCatch stackTrace failed.", __func__);
        }
    }
}

bool ConnectionManager::IsConnectCallerEqual(const sptr<IRemoteObject> &connectCaller,
    const sptr<IRemoteObject> &connectCallerOther)
{
    return connectCaller == connectCallerOther;
}

bool ConnectionManager::IsConnectReceiverEqual(const AppExecFwk::ElementName &connectReceiver,
    const AppExecFwk::ElementName &connectReceiverOther)
{
    return connectReceiver.GetBundleName() == connectReceiverOther.GetBundleName() &&
           connectReceiver.GetAbilityName() == connectReceiverOther.GetAbilityName();
}

ErrCode ConnectionManager::HandleCallbackTimeOut(const sptr<IRemoteObject> &connectCaller, const AAFwk::Want &want,
    const AppExecFwk::ElementName &connectReceiver, sptr<AbilityConnection> abilityConnection,
    const sptr<AbilityConnectCallback> &connectCallback)
{
    if (abilityConnection->GetRemoteObject() == nullptr) {
        while (true) {
            if (abilityConnection->GetRemoteObject() != nullptr) {
                connectCallback->OnAbilityConnectDone(connectReceiver, abilityConnection->GetRemoteObject(),
                    abilityConnection->GetResultCode());
                return ERR_OK;
            }
        }
    } else {
        return AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(want, abilityConnection, connectCaller);
    }
}
}  // namespace AbilityRuntime
}  // namespace OHOS