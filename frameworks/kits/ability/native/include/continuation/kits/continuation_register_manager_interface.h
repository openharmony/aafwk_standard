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
#ifndef FOUNDATION_APPEXECFWK_OHOS_CONTINUATION_REGISTER_MANAGER_INTERFACE_H
#define FOUNDATION_APPEXECFWK_OHOS_CONTINUATION_REGISTER_MANAGER_INTERFACE_H
#include <string>
#include <memory>
#include "extra_params.h"

namespace OHOS {
namespace AppExecFwk {
class IContinuationDeviceCallback;
class RequestCallback;
class IContinuationRegisterManager {
public:
    IContinuationRegisterManager() = default;
    virtual ~IContinuationRegisterManager() = default;
    /**
     * @brief Registers an ability to be migrated with the Device+ control center and obtains the registration token
     * assigned to the ability.
     *
     * <p>You can use {@link IContinuationDeviceCallback} to listen for the device connection state changes after the
     * user selects a device for ability migration and implement your own processing logic. When the device is
     * connected, {@link IContinuationDeviceCallback#onDeviceConnectDone} is called to initiate the ability migration
     * process; when the device is disconnected, {@link IContinuationDeviceCallback#onDeviceDisconnectDone} is called
     * to perform related operations, for example, notify the user of the disconnection. In addition, you can obtain
     * the registration token via {@link RequestCallback#onResult}.
     * <p>To use this method, you must dynamically request the {@code ohos.permission.DISTRIBUTED_DATASYNC} permission
     * from the user. The {@code ohos.permission.DISTRIBUTED_DATASYNC} permission is of the {@code user_grant} level.
     *
     * @param bundleName Indicates the bundle name of the application whose ability is to be migrated.
     * @param parameter Indicates the {@link ExtraParams} object containing the extra parameters used to filter
     * the list of available devices. This parameter can be null.
     * @param deviceCallback Indicates the callback to be invoked when the connection state of the selected device
     * changes.
     * @param requestCallback Indicates the callback to be invoked when the Device+ service is connected.
     * @return none
     */
    virtual void Register(const std::string &bundleName, const ExtraParams &parameter,
        const std::shared_ptr<IContinuationDeviceCallback> &deviceCallback,
        const std::shared_ptr<RequestCallback> &requestCallback) = 0;

    /**
     * @brief Unregisters a specified ability from the Device+ control center based on the token obtained during ability
     * registration.
     *
     * @param token Indicates the registration token of the ability.
     * @param requestCallback Indicates the callback to be invoked when the Device+ service is connected.
     * This parameter can be null.
     * @return none
     */
    virtual void Unregister(int token, const std::shared_ptr<RequestCallback> &requestCallback) = 0;

    /**
     * @brief Updates the connection state of the device where the specified ability is successfully migrated.
     *
     * <p>After the migration is successful, you can call this method in the UI thread to update the connection state
     * of the migrated ability and the device connection state on the screen showing available devices.
     *
     * @param token Indicates the registration token of the ability.
     * @param deviceId Indicates the ID of the device whose connection state is to be updated.
     * @param status Indicates the connection state to update, which can be {@link DeviceConnectState#FAILURE},
     * {@link DeviceConnectState#IDLE}, {@link DeviceConnectState#CONNECTING}, {@link DeviceConnectState#CONNECTED},
     * or {@link DeviceConnectState#DIS_CONNECTING}.
     * @param requestCallback Indicates the callback to be invoked when the Device+ service is connected.
     * This parameter can be null.
     * @return none
     */
    virtual void UpdateConnectStatus(int token, const std::string &deviceId, int status,
        const std::shared_ptr<RequestCallback> &requestCallback) = 0;

    /**
     * @brief Shows the list of devices that can be selected for ability migration on the distributed network.
     *
     * @param token Indicates the registration token of the ability.
     * @param parameter Indicates the {@link ExtraParams} object containing the extra parameters used to filter
     * the list of available devices. This parameter can be null.
     * @param requestCallback Indicates the callback to be invoked when the Device+ service is connected.
     * This parameter can be null.
     * @return none
     */
    virtual void ShowDeviceList(
        int token, const ExtraParams &parameter, const std::shared_ptr<RequestCallback> &requestCallback) = 0;

    /**
     * @brief Disconnects from the Device+ control center.
     *
     * <p>This method can be called when you want to stop migrating an ability.
     *
     * @param none
     * @return none
     */
    virtual void Disconnect() = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_CONTINUATION_REGISTER_MANAGER_INTERFACE_H