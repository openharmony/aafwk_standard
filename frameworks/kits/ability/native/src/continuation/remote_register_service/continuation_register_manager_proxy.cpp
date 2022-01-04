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
#include "continuation_register_manager_proxy.h"

#include "app_log_wrapper.h"
#include "context.h"
#include "continuation_connector.h"
#include "continuation_device_callback_interface.h"
#include "continuation_request.h"
#include "request_callback.h"

namespace OHOS {
namespace AppExecFwk {
ContinuationRequestRegister::ContinuationRequestRegister(const std::string &bundleName, const ExtraParams &parameter,
    const std::shared_ptr<IContinuationDeviceCallback> &deviceCallback)
{
    parameter_ = parameter;
    deviceCallback_ = deviceCallback;
    bundleName_ = bundleName;
}

void ContinuationRequestRegister::Execute(void)
{
    if (continuatinConnector_ == nullptr) {
        APP_LOGE("%{public}s called, continuatinConnector is null", __func__);
        return;
    }

    int ret = continuatinConnector_->Register(context_, bundleName_, parameter_, deviceCallback_);
    if (requestCallback_ != nullptr) {
        requestCallback_->OnResult(ret);
        APP_LOGI("%{public}s called, ret=%{public}d", __func__, ret);
    } else {
        APP_LOGE("%{public}s called, requestCallback is null", __func__);
    }
}

ContinuationRequestUnRegister::ContinuationRequestUnRegister(int token)
{
    token_ = token;
}

void ContinuationRequestUnRegister::Execute(void)
{
    if (continuatinConnector_ == nullptr) {
        APP_LOGE("%{public}s called, continuatinConnector is null", __func__);
        return;
    }

    bool ret = continuatinConnector_->Unregister(token_);
    if (requestCallback_ != nullptr) {
        requestCallback_->OnResult(ret ? 0 : -1);
        APP_LOGI("%{public}s called, ret=%{public}d", __func__, (ret ? 0 : -1));
    } else {
        APP_LOGE("%{public}s called, requestCallback is null", __func__);
    }
}

ContinuationRequestUpdateConnectStatus::ContinuationRequestUpdateConnectStatus(
    int token, const std::string &deviceId, int status)
{
    token_ = token;
    deviceId_ = deviceId;
    status_ = status;
}

void ContinuationRequestUpdateConnectStatus::Execute(void)
{
    if (continuatinConnector_ == nullptr) {
        APP_LOGE("%{public}s called, continuatinConnector is null", __func__);
        return;
    }

    bool ret = continuatinConnector_->UpdateConnectStatus(token_, deviceId_, status_);
    if (requestCallback_ != nullptr) {
        requestCallback_->OnResult(ret ? 0 : -1);
        APP_LOGI("%{public}s called, ret=%{public}d", __func__, (ret ? 0 : -1));
    } else {
        APP_LOGE("%{public}s called, requestCallback is null", __func__);
    }
}

ContinuationRequestShowDeviceList::ContinuationRequestShowDeviceList(int token, const ExtraParams &parameter)
{
    token_ = token;
    parameter_ = parameter;
}

void ContinuationRequestShowDeviceList::Execute(void)
{
    if (continuatinConnector_ == nullptr) {
        APP_LOGE("%{public}s called, continuatinConnector is null", __func__);
        return;
    }

    bool ret = continuatinConnector_->ShowDeviceList(token_, parameter_);
    if (requestCallback_ != nullptr) {
        requestCallback_->OnResult(ret ? 0 : -1);
        APP_LOGI("%{public}s called, ret=%{public}d", __func__, (ret ? 0 : -1));
    } else {
        APP_LOGE("%{public}s called, requestCallback is null", __func__);
    }
}

ContinuationRegisterManagerProxy::ContinuationRegisterManagerProxy(const std::weak_ptr<Context> &context)
{
    context_ = context;
    std::shared_ptr<Context> ctx = context_.lock();
    if (ctx != nullptr) {
        applicationContext_ = std::weak_ptr<Context>(ctx->GetApplicationContext());
    }
}

ContinuationRegisterManagerProxy::~ContinuationRegisterManagerProxy()
{}

/**
 * Registers an ability to be migrated with the Device+ control center and obtains the registration token assigned
 * to the ability.
 *
 * @param bundleName Indicates the bundle name of the application whose ability is to be migrated.
 * @param parameter Indicates the {@link ExtraParams} object containing the extra parameters used to filter
 * the list of available devices. This parameter can be null.
 * @param deviceCallback Indicates the callback to be invoked when the connection state of the selected device
 * changes.
 * @param requestCallback Indicates the callback to be invoked when the Device+ service is connected.
 */
void ContinuationRegisterManagerProxy::Register(const std::string &bundleName, const ExtraParams &parameter,
    const std::shared_ptr<IContinuationDeviceCallback> &deviceCallback,
    const std::shared_ptr<RequestCallback> &requestCallback)
{
    APP_LOGI("%{public}s called", __func__);

    if (context_.lock() == nullptr || applicationContext_.lock() == nullptr) {
        APP_LOGE("%{public}s context or applicationContext is null", __func__);
        return;
    }

    ContinuationRequestRegister *pContinuationRequestRegister =
        new (std::nothrow) ContinuationRequestRegister(bundleName, parameter, deviceCallback);
    if (pContinuationRequestRegister != nullptr) {
        pContinuationRequestRegister->SetContext(context_);
        pContinuationRequestRegister->SetContinuationConnector(continuatinConnector_);
        pContinuationRequestRegister->SetRequestCallback(requestCallback);

        std::shared_ptr<ContinuationRequest> request(pContinuationRequestRegister);

        SendRequest(applicationContext_, request);
    } else {
        APP_LOGE("%{public}s Create ContinuationRequestRegister failed", __func__);
    }
}

/**
 * Unregisters a specified ability from the Device+ control center based on the token obtained during ability
 * registration.
 *
 * @param token Indicates the registration token of the ability.
 * @param requestCallback Indicates the callback to be invoked when the Device+ service is connected.
 * This parameter can be null.
 */
void ContinuationRegisterManagerProxy::Unregister(int token, const std::shared_ptr<RequestCallback> &requestCallback)
{
    APP_LOGI("%{public}s called", __func__);

    if (applicationContext_.lock() == nullptr) {
        APP_LOGI("%{public}s Context is null", __func__);
        return;
    }

    ContinuationRequestUnRegister *pContinuationRequestUnRegister =
        new (std::nothrow) ContinuationRequestUnRegister(token);
    if (pContinuationRequestUnRegister != nullptr) {
        pContinuationRequestUnRegister->SetContinuationConnector(continuatinConnector_);
        pContinuationRequestUnRegister->SetRequestCallback(requestCallback);

        std::shared_ptr<ContinuationRequest> request(pContinuationRequestUnRegister);

        SendRequest(applicationContext_, request);
    } else {
        APP_LOGE("%{public}s Create ContinuationRequestUnRegister failed", __func__);
    }
}

/**
 * Updates the connection state of the device where the specified ability is successfully migrated.
 *
 * @param token Indicates the registration token of the ability.
 * @param deviceId Indicates the ID of the device whose connection state is to be updated.
 * @param status Indicates the connection state to update, which can be {@link DeviceConnectState#FAILURE},
 * {@link DeviceConnectState#IDLE}, {@link DeviceConnectState#CONNECTING}, {@link DeviceConnectState#CONNECTED},
 * or {@link DeviceConnectState#DIS_CONNECTING}.
 * @param requestCallback Indicates the callback to be invoked when the Device+ service is connected.
 * This parameter can be null.
 */
void ContinuationRegisterManagerProxy::UpdateConnectStatus(
    int token, const std::string &deviceId, int status, const std::shared_ptr<RequestCallback> &requestCallback)
{
    APP_LOGI("%{public}s called", __func__);

    if (applicationContext_.lock() == nullptr) {
        APP_LOGI("%{public}s Context is null", __func__);
        return;
    }

    ContinuationRequestUpdateConnectStatus *pContinuationRequestUpdateConnectStatus =
        new (std::nothrow) ContinuationRequestUpdateConnectStatus(token, deviceId, status);

    if (pContinuationRequestUpdateConnectStatus != nullptr) {
        pContinuationRequestUpdateConnectStatus->SetContinuationConnector(continuatinConnector_);
        pContinuationRequestUpdateConnectStatus->SetRequestCallback(requestCallback);

        std::shared_ptr<ContinuationRequest> request(pContinuationRequestUpdateConnectStatus);

        SendRequest(applicationContext_, request);
    } else {
        APP_LOGE("%{public}s Create ContinuationRequestUpdateConnectStatus failed", __func__);
    }
}

/**
 * Shows the list of devices that can be selected for ability migration on the distributed network.
 *
 * @param token Indicates the registration token of the ability.
 * @param parameter Indicates the {@link ExtraParams} object containing the extra parameters used to filter
 * the list of available devices. This parameter can be null.
 * @param requestCallback Indicates the callback to be invoked when the Device+ service is connected.
 * This parameter can be null.
 */
void ContinuationRegisterManagerProxy::ShowDeviceList(
    int token, const ExtraParams &parameter, const std::shared_ptr<RequestCallback> &requestCallback)
{
    APP_LOGI("%{public}s called begin", __func__);

    if (applicationContext_.lock() == nullptr) {
        APP_LOGI("%{public}s Context is null", __func__);
        return;
    }

    ContinuationRequestShowDeviceList *pContinuationRequestShowDeviceList =
        new (std::nothrow) ContinuationRequestShowDeviceList(token, parameter);
    if (pContinuationRequestShowDeviceList != nullptr) {
        pContinuationRequestShowDeviceList->SetContinuationConnector(continuatinConnector_);
        pContinuationRequestShowDeviceList->SetRequestCallback(requestCallback);

        std::shared_ptr<ContinuationRequest> request(pContinuationRequestShowDeviceList);

        SendRequest(applicationContext_, request);
        APP_LOGI("%{public}s called end", __func__);
    } else {
        APP_LOGE("%{public}s Create ContinuationRequestShowDeviceList failed", __func__);
    }
}

/**
 * Disconnects from the Device+ control center.
 */
void ContinuationRegisterManagerProxy::Disconnect(void)
{
    APP_LOGI("%{public}s called begin", __func__);

    if (continuatinConnector_ != nullptr && continuatinConnector_->IsAbilityConnected()) {
        continuatinConnector_->UnbindRemoteRegisterAbility();
    }
    APP_LOGI("%{public}s called end", __func__);
}

void ContinuationRegisterManagerProxy::SendRequest(
    const std::weak_ptr<Context> &context, const std::shared_ptr<ContinuationRequest> &request)
{
    APP_LOGI("%{public}s called begin", __func__);
    if (request == nullptr) {
        APP_LOGE("%{public}s called, request is null", __func__);
        return;
    }

    if (continuatinConnector_ == nullptr) {
        continuatinConnector_ = ContinuationConnector::GetInstance(context);
    }

    if (!continuatinConnector_->IsAbilityConnected()) {
        continuatinConnector_->BindRemoteRegisterAbility(request);
    } else {
        request->Execute();
    }
    APP_LOGI("%{public}s called end", __func__);
}
}  // namespace AppExecFwk
}  // namespace OHOS
