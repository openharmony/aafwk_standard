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

#include "continuation_connector.h"

#include "app_log_wrapper.h"
#include "continuation_device_callback_proxy.h"
#include "remote_register_service_proxy.h"

namespace OHOS {
namespace AppExecFwk {
sptr<ContinuationConnector> ContinuationConnector::instance_ = nullptr;
std::mutex ContinuationConnector::mutex_;
const std::string ContinuationConnector::CONNECTOR_DEVICE_ID("");
const std::string ContinuationConnector::CONNECTOR_BUNDLE_NAME("com.ohos.controlcenter");
const std::string ContinuationConnector::CONNECTOR_ABILITY_NAME(
    "com.ohos.controlcenter.fatransfer.service.FeatureAbilityRegisterService");

ContinuationConnector::ContinuationConnector(const std::weak_ptr<Context> &context) : context_(context)
{}

/**
 * @brief get singleton of Class ContinuationConnector
 *
 * @param context: the running context for appcontext
 *
 * @return The singleton of ContinuationConnector
 */
sptr<ContinuationConnector> ContinuationConnector::GetInstance(const std::weak_ptr<Context> &context)
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = sptr<ContinuationConnector>(new (std::nothrow) ContinuationConnector(context));
        }
    }
    return instance_;
}

/**
 * @brief This method is called back to receive the connection result after an ability calls the
 * Ability#connectAbility(Intent, IAbilityConnection) method to connect it to a Service ability.
 *
 * @param element: Indicates information about the connected Service ability.
 * @param remote: Indicates the remote proxy object of the Service ability.
 * @param resultCode: Indicates the connection result code. The value 0 indicates a successful connection, and any other
 * value indicates a connection failure.
 */
void ContinuationConnector::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    APP_LOGI("%{public}s called begin", __func__);
    if (remoteObject == nullptr) {
        APP_LOGE("ContinuationConnector::OnAbilityConnectDone failed, remote is nullptr");
        return;
    }
    remoteRegisterService_ = iface_cast<RemoteRegisterServiceProxy>(remoteObject);
    if (remoteRegisterService_ == nullptr) {
        APP_LOGE("ContinuationConnector::OnAbilityConnectDone failed, remoteRegisterService_ is nullptr");
        return;
    }
    isConnected_.store(true);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto &iter : continuationRequestList_) {
            iter->Execute();
        }
        continuationRequestList_.clear();
    }
    APP_LOGI("%{public}s called end", __func__);
}

/**
 * @brief This method is called back to receive the disconnection result after the connected Service ability crashes or
 * is killed. If the Service ability exits unexpectedly, all its connections are disconnected, and each ability
 * previously connected to it will call onAbilityDisconnectDone.
 *
 * @param element: Indicates information about the disconnected Service ability.
 * @param resultCode: Indicates the disconnection result code. The value 0 indicates a successful disconnection, and any
 * other value indicates a disconnection failure.
 */
void ContinuationConnector::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    APP_LOGI("%{public}s called begin", __func__);
    remoteRegisterService_ = nullptr;
    isConnected_.store(false);
    APP_LOGI("%{public}s called end", __func__);
}

/**
 * @brief bind remote ability of RemoteRegisterService.
 *
 * @param request: request for continuation.
 */
void ContinuationConnector::BindRemoteRegisterAbility(const std::shared_ptr<AppExecFwk::ContinuationRequest> &request)
{
    APP_LOGI("%{public}s called begin", __func__);
    std::shared_ptr tmpcontext = context_.lock();
    if (tmpcontext == nullptr) {
        APP_LOGE("ContinuationConnector::BindRemoteRegisterAbility failed, context_.lock is nullptr");
        return;
    }
    if (request == nullptr) {
        APP_LOGE("ContinuationConnector::BindRemoteRegisterAbility failed, request is nullptr");
        return;
    }
    if (IsAbilityConnected()) {
        APP_LOGI("ContinuationConnector::BindRemoteRegisterAbility, remote register bounded");
        request->Execute();
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        continuationRequestList_.push_back(request);
    }
    BindRemoteRegisterAbility();
    APP_LOGI("%{public}s called end", __func__);
}

/**
 * @brief unbind remote ability of RemoteRegisterService.
 */
void ContinuationConnector::UnbindRemoteRegisterAbility()
{
    APP_LOGI("%{public}s called begin", __func__);
    std::shared_ptr tmpcontext = context_.lock();
    if (tmpcontext == nullptr) {
        APP_LOGE("ContinuationConnector::UnbindRemoteRegisterAbility failed, context_.lock is nullptr");
        return;
    }

    tmpcontext->DisconnectAbility(this);
    isConnected_.store(false);
    remoteRegisterService_ = nullptr;
    APP_LOGI("%{public}s called end", __func__);
}

/**
 * @brief check whether connected to remote register service.
 *
 * @return bool true if connected, otherwise false.
 */
bool ContinuationConnector::IsAbilityConnected()
{
    return isConnected_.load();
}

/**
 * @brief unregister to control center continuation register service.
 *
 * @param token token from register return value.
 *
 * @return bool result of unregister.
 */
bool ContinuationConnector::Unregister(int token)
{
    APP_LOGI("%{public}s called begin", __func__);
    if (remoteRegisterService_ == nullptr) {
        APP_LOGE("ContinuationConnector::Unregister failed, remoteRegisterService_ is nullptr");
        return false;
    }

    APP_LOGI("%{public}s called end", __func__);
    return remoteRegisterService_->Unregister(token);
}

/**
 * @brief notify continuation status to control center continuation register service.
 *
 * @param token token from register.
 * @param deviceId device id.
 * @param status device status.
 *
 * @return bool result of updateConnectStatus.
 */
bool ContinuationConnector::UpdateConnectStatus(int token, const std::string &deviceId, int status)
{
    APP_LOGI("%{public}s called begin", __func__);
    if (remoteRegisterService_ == nullptr) {
        APP_LOGE("ContinuationConnector::UpdateConnectStatus failed, remoteRegisterService_ is nullptr");
        return false;
    }

    APP_LOGI("%{public}s called end", __func__);
    return remoteRegisterService_->UpdateConnectStatus(token, deviceId, status);
}

/**
 * @brief notify control center continuation register service to show device list.
 *
 * @param token token from register
 * @param parameter filter with supported device list.
 * @return bool result of showDeviceList.
 */
bool ContinuationConnector::ShowDeviceList(int token, const AppExecFwk::ExtraParams &parameter)
{
    APP_LOGI("%{public}s called begin", __func__);
    if (remoteRegisterService_ == nullptr) {
        APP_LOGE("ContinuationConnector::ShowDeviceList failed, remoteRegisterService_ is nullptr");
        return false;
    }
    APP_LOGI("%{public}s called end", __func__);
    return remoteRegisterService_->ShowDeviceList(token, parameter);
}

/**
 * @brief register to control center continuation register service.
 *
 * @param context ability context.
 * @param bundleName bundle name of ability.
 * @param parameter filter with supported device list.
 * @param callback callback for device connect and disconnect.
 *
 * @return int token.
 */
int ContinuationConnector::Register(std::weak_ptr<Context> &context, std::string bundleName,
    const AppExecFwk::ExtraParams &parameter, std::shared_ptr<IContinuationDeviceCallback> &callback)
{
    APP_LOGI("%{public}s called begin", __func__);
    std::shared_ptr pcontext = context.lock();
    if (pcontext == nullptr) {
        APP_LOGE("ContinuationConnector::Register failed, pcontext is nullptr");
        return -1;
    }
    if (remoteRegisterService_ == nullptr) {
        APP_LOGE("ContinuationConnector::Register failed, remoteRegisterService_ is nullptr");
        return -1;
    }
    sptr<IRemoteObject> token = pcontext->GetToken();
    if (token == nullptr) {
        APP_LOGE("ContinuationConnector::Register failed, token is nullptr");
        return -1;
    }

    sptr<ContinuationDeviceCallbackProxy> callBackSptr(new (std::nothrow) ContinuationDeviceCallbackProxy(callback));

    APP_LOGI("%{public}s called end", __func__);
    return remoteRegisterService_->Register(bundleName, token, parameter, callBackSptr);
}

/**
 * @brief bind remote ability of RemoteRegisterService.
 */
void ContinuationConnector::BindRemoteRegisterAbility()
{
    APP_LOGI("%{public}s called begin", __func__);
    std::shared_ptr tmpcontext = context_.lock();
    if (tmpcontext == nullptr) {
        APP_LOGE("ContinuationConnector::BindRemoteRegisterAbility failed, context_.lock is nullptr");
        return;
    }
    Want want;
    want.SetElementName(CONNECTOR_DEVICE_ID, CONNECTOR_BUNDLE_NAME, CONNECTOR_ABILITY_NAME);
    want.AddFlags(Want::FLAG_NOT_OHOS_COMPONENT);
    tmpcontext->ConnectAbility(want, this);
    APP_LOGI("%{public}s called end", __func__);
}
}  // namespace AppExecFwk
}  // namespace OHOS
