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
#ifndef FOUNDATION_APPEXECFWK_OHOS_DEVICE_CONNECT_STATE_H
#define FOUNDATION_APPEXECFWK_OHOS_DEVICE_CONNECT_STATE_H
namespace OHOS {
namespace AppExecFwk {
enum class DeviceConnectState {
    // Indicates that the device fails to be connected.
    FAILURE = -1,

    // Indicates that the device is in the initial state or is disconnected.
    IDLE = 0,

    // Indicates that the device is being connected.
    CONNECTING = 1,

    // Indicates that the device is connected.
    CONNECTED = 2,

    // Indicates that the device is being disconnected.
    DIS_CONNECTING = 3
};
using DEVICE_CONNECT_STATE = enum DeviceConnectState;
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_DEVICE_CONNECT_STATE_H