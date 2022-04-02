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

#include "datashare_connection.h"

#include "ability_manager_client.h"
#include "bytrace.h"
#include "datashare_proxy.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
sptr<DataShareConnection> DataShareConnection::instance_ = nullptr;
std::mutex DataShareConnection::mutex_;

/**
 * @brief get singleton of Class DataShareConnection
 *
 * @return The singleton of DataShareConnection
 */
sptr<DataShareConnection> DataShareConnection::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = sptr<DataShareConnection>(new (std::nothrow) DataShareConnection());
        }
    }
    return instance_;
}

/**
 * @brief This method is called back to receive the connection result after an ability calls the
 * ConnectAbility method to connect it to an extension ability.
 *
 * @param element: Indicates information about the connected extension ability.
 * @param remote: Indicates the remote proxy object of the extension ability.
 * @param resultCode: Indicates the connection result code. The value 0 indicates a successful connection, and any
 * other value indicates a connection failure.
 */
void DataShareConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA,__PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s called begin", __func__);
    if (remoteObject == nullptr) {
        HILOG_ERROR("DataShareConnection::OnAbilityConnectDone failed, remote is nullptr");
        return;
    }
    dataShareProxy_ = iface_cast<DataShareProxy>(remoteObject);
    if (dataShareProxy_ == nullptr) {
        HILOG_ERROR("DataShareConnection::OnAbilityConnectDone failed, dataShareProxy_ is nullptr");
        return;
    }
    isConnected_.store(true);
    HILOG_INFO("%{public}s called end", __func__);
}

/**
 * @brief This method is called back to receive the disconnection result after the connected extension ability crashes
 * or is killed. If the extension ability exits unexpectedly, all its connections are disconnected, and each ability
 * previously connected to it will call onAbilityDisconnectDone.
 *
 * @param element: Indicates information about the disconnected extension ability.
 * @param resultCode: Indicates the disconnection result code. The value 0 indicates a successful disconnection, and
 * any other value indicates a disconnection failure.
 */
void DataShareConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA,__PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s called begin", __func__);
    dataShareProxy_ = nullptr;
    isConnected_.store(false);
    HILOG_INFO("%{public}s called end", __func__);
}

/**
 * @brief connect remote ability of DataShareExtAbility.
 */
void DataShareConnection::ConnectDataShareExtAbility(const AAFwk::Want &want, const sptr<IRemoteObject> &token)
{
    HILOG_INFO("%{public}s called begin", __func__);
    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(want, this, token);
    HILOG_INFO("%{public}s called end, ret=%{public}d", __func__, ret);
}

/**
 * @brief disconnect remote ability of DataShareExtAbility.
 */
void DataShareConnection::DisconnectDataShareExtAbility()
{
    HILOG_INFO("%{public}s called begin", __func__);
    dataShareProxy_ = nullptr;
    isConnected_.store(false);
    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(this);
    HILOG_INFO("%{public}s called end, ret=%{public}d", __func__, ret);
}

/**
 * @brief check whether connected to remote extension ability.
 *
 * @return bool true if connected, otherwise false.
 */
bool DataShareConnection::IsExtAbilityConnected()
{
    return isConnected_.load();
}

sptr<IDataShare> DataShareConnection::GetDataShareProxy()
{
    return dataShareProxy_;
}
}  // namespace AppExecFwk
}  // namespace OHOS