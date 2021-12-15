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
#ifndef FOUNDATION_APPEXECFWK_OHOS_CONTINUATION_REGISTER_MANAGER_PROXY_H
#define FOUNDATION_APPEXECFWK_OHOS_CONTINUATION_REGISTER_MANAGER_PROXY_H

#include <memory>
#include "continuation_register_manager_interface.h"
#include "continuation_request.h"
#include "continuation_connector.h"

namespace OHOS {
namespace AppExecFwk {
class ContinuationConnector;
class Context;
class IContinuationDeviceCallback;
class RequestCallback;

class ContinuationRequestBase : public ContinuationRequest {
public:
    ContinuationRequestBase() = default;
    virtual ~ContinuationRequestBase() = default;

    void SetContext(const std::weak_ptr<Context> &context)
    {
        context_ = context;
    }

    void SetContinuationConnector(const sptr<ContinuationConnector> &continuatinConnector)
    {
        continuatinConnector_ = continuatinConnector;
    }

    void SetRequestCallback(const std::shared_ptr<RequestCallback> &requestCallback)
    {
        requestCallback_ = requestCallback;
    }

protected:
    std::weak_ptr<Context> context_;
    sptr<ContinuationConnector> continuatinConnector_;
    std::shared_ptr<RequestCallback> requestCallback_ = nullptr;
};

class ContinuationRequestRegister : public ContinuationRequestBase {
public:
    ContinuationRequestRegister(const std::string &bundleName, const ExtraParams &parameter,
        const std::shared_ptr<IContinuationDeviceCallback> &deviceCallback);
    virtual ~ContinuationRequestRegister() = default;
    virtual void Execute() override;

private:
    ExtraParams parameter_;
    std::shared_ptr<IContinuationDeviceCallback> deviceCallback_ = nullptr;
    std::string bundleName_ = "";
};

class ContinuationRequestUnRegister : public ContinuationRequestBase {
public:
    ContinuationRequestUnRegister(int token);
    virtual ~ContinuationRequestUnRegister() = default;
    virtual void Execute() override;

private:
    int token_ = 0;
};

class ContinuationRequestUpdateConnectStatus : public ContinuationRequestBase {
public:
    ContinuationRequestUpdateConnectStatus(int token, const std::string &deviceId, int status);
    virtual ~ContinuationRequestUpdateConnectStatus() = default;
    virtual void Execute() override;

private:
    int token_ = 0;
    std::string deviceId_ = "";
    int status_;
};

class ContinuationRequestShowDeviceList : public ContinuationRequestBase {
public:
    ContinuationRequestShowDeviceList(int token, const ExtraParams &parameter);
    virtual ~ContinuationRequestShowDeviceList() = default;
    virtual void Execute() override;

private:
    int token_ = 0;
    ExtraParams parameter_;
};

class ContinuationRegisterManagerProxy : public IContinuationRegisterManager {
public:
    ContinuationRegisterManagerProxy(const std::weak_ptr<Context> &context);
    virtual ~ContinuationRegisterManagerProxy();

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
    virtual void Register(const std::string &bundleName, const ExtraParams &parameter,
        const std::shared_ptr<IContinuationDeviceCallback> &deviceCallback,
        const std::shared_ptr<RequestCallback> &requestCallback) override;

    /**
     * Unregisters a specified ability from the Device+ control center based on the token obtained during ability
     * registration.
     *
     * @param token Indicates the registration token of the ability.
     * @param requestCallback Indicates the callback to be invoked when the Device+ service is connected.
     * This parameter can be null.
     */
    virtual void Unregister(int token, const std::shared_ptr<RequestCallback> &requestCallback) override;

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
    virtual void UpdateConnectStatus(int token, const std::string &deviceId, int status,
        const std::shared_ptr<RequestCallback> &requestCallback) override;

    /**
     * Shows the list of devices that can be selected for ability migration on the distributed network.
     *
     * @param token Indicates the registration token of the ability.
     * @param parameter Indicates the {@link ExtraParams} object containing the extra parameters used to filter
     * the list of available devices. This parameter can be null.
     * @param requestCallback Indicates the callback to be invoked when the Device+ service is connected.
     * This parameter can be null.
     */
    virtual void ShowDeviceList(
        int token, const ExtraParams &parameter, const std::shared_ptr<RequestCallback> &requestCallback) override;

    /**
     * Disconnects from the Device+ control center.
     */
    virtual void Disconnect(void) override;

private:
    std::weak_ptr<Context> context_;
    std::weak_ptr<Context> applicationContext_;
    sptr<ContinuationConnector> continuatinConnector_;

    void SendRequest(const std::weak_ptr<Context> &context, const std::shared_ptr<ContinuationRequest> &request);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_CONTINUATION_REGISTER_MANAGER_PROXY_H