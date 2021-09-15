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
#ifndef FOUNDATION_APPEXECFWK_OHOS_CONTINUATION_DEVICE_CALLBACK_INTERFACE_H
#define FOUNDATION_APPEXECFWK_OHOS_CONTINUATION_DEVICE_CALLBACK_INTERFACE_H

#include <string>

namespace OHOS {
namespace AppExecFwk {
class IContinuationDeviceCallback {
public:
    IContinuationDeviceCallback() = default;
    virtual ~IContinuationDeviceCallback() = default;
    /**
     * Called when the user selects a device from the candidate device list.
     * You can implement your own processing logic in this callback to initiate the ability migration process.
     *
     * @param deviceId Indicates the ID of the selected device.
     * @param deviceType Indicates the type of the selected device, which can be
     * {@link ExtraParams#DEVICETYPE_SMART_PHONE}, {@link ExtraParams#DEVICETYPE_SMART_PAD},
     * {@link ExtraParams#DEVICETYPE_SMART_WATCH}, or {@link ExtraParams#DEVICETYPE_SMART_TV}.
     */
    virtual void OnDeviceConnectDone(const std::string &deviceId, const std::string &deviceType) = 0;

    /**
     * Called when the Device+ control center disconnects from a specified device.
     * You can implement your own processing logic in this callback, such as notifying the user of the disconnection.
     *
     * @param deviceId Indicates the ID of the disconnected device.
     */
    virtual void OnDeviceDisconnectDone(const std::string &deviceId) = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_CONTINUATION_DEVICE_CALLBACK_INTERFACE_H