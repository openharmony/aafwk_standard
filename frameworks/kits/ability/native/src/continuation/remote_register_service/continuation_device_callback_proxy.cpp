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
#include "continuation_device_callback_proxy.h"
#include "hilog_wrapper.h"
namespace OHOS {
namespace AppExecFwk {
/**
 * A constructor used to create a {@link ohos.aafwk.ability.continuation.ContinuationDeviceCallbackProxy} instance.
 */
ContinuationDeviceCallbackProxy::ContinuationDeviceCallbackProxy(std::shared_ptr<IContinuationDeviceCallback> &callback)
{
    callback_ = std::weak_ptr<IContinuationDeviceCallback>(callback);
}
/**
 * A destructor used to release a {@link ohos.aafwk.ability.continuation.ContinuationDeviceCallbackProxy} instance.
 */
ContinuationDeviceCallbackProxy::~ContinuationDeviceCallbackProxy()
{}
void ContinuationDeviceCallbackProxy::Connect(const std::string &deviceId, const std::string &deviceType)
{
    HILOG_INFO("%{public}s called begin", __func__);
    std::shared_ptr<IContinuationDeviceCallback> callback = nullptr;
    callback = callback_.lock();
    if (callback == nullptr) {
        HILOG_ERROR("%{public}s callback is null", __func__);
        return;
    }
    callback->OnDeviceConnectDone(deviceId, deviceType);
    HILOG_INFO("%{public}s called end", __func__);
}

void ContinuationDeviceCallbackProxy::Disconnect(const std::string &deviceId)
{
    HILOG_INFO("%{public}s called begin", __func__);
    std::shared_ptr<IContinuationDeviceCallback> callback = nullptr;
    callback = callback_.lock();
    if (callback == nullptr) {
        HILOG_ERROR("%{public}s callback is null", __func__);
        return;
    }
    callback->OnDeviceDisconnectDone(deviceId);
    HILOG_INFO("%{public}s called end", __func__);
}
}  // namespace AppExecFwk
}  // namespace OHOS