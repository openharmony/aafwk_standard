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
#include "continuation_register_manager.h"

#include "continuation_device_callback_interface.h"
#include "continuation_register_manager_proxy.h"
#include "extra_params.h"
#include "hilog_wrapper.h"
#include "request_callback.h"

namespace OHOS {
namespace AppExecFwk {

void ContinuationRegisterManager::Init(
    const std::shared_ptr<ContinuationRegisterManagerProxy> &continuationRegisterManagerProxy)
{
    continuationRegisterManagerProxy_ = continuationRegisterManagerProxy;
}

/**
 * register to controlcenter continuation register service.
 *
 * @param bundleName bundlename of ability.
 * @param parameter filter with supported device list.
 * @param callback callback for device connect and disconnect.
 * @param requestCallback callback for this request, -1 means failed, otherwise is register token.
 */
void ContinuationRegisterManager::Register(const std::string &bundleName, const ExtraParams &parameter,
    const std::shared_ptr<IContinuationDeviceCallback> &deviceCallback,
    const std::shared_ptr<RequestCallback> &requestCallback)
{
    HILOG_INFO("%{public}s called begin", __func__);
    if (continuationRegisterManagerProxy_ != nullptr) {
        continuationRegisterManagerProxy_->Register(bundleName, parameter, deviceCallback, requestCallback);
        HILOG_INFO("%{public}s called end", __func__);
    } else {
        HILOG_ERROR("%{public}s, ContinuationRegisterManagerProxy is null", __func__);
    }
}

/**
 * unregister to controlcenter continuation register service.
 *
 * @param token token from register return value.
 * @param requestCallback callback for this request, -1 means failed, otherwise succeeded.
 */
void ContinuationRegisterManager::Unregister(int token, const std::shared_ptr<RequestCallback> &requestCallback)
{
    HILOG_INFO("%{public}s called begin", __func__);
    if (continuationRegisterManagerProxy_ != nullptr) {
        continuationRegisterManagerProxy_->Unregister(token, requestCallback);
        HILOG_INFO("%{public}s called end", __func__);
    } else {
        HILOG_ERROR("%{public}s, ContinuationRegisterManagerProxy is null", __func__);
    }
}

/**
 * notify continuation status to controlcenter continuation register service.
 *
 * @param token token from register.
 * @param deviceId deviceid.
 * @param status device status.
 * @param requestCallback callback for this request, -1 means failed, otherwise successed.
 */
void ContinuationRegisterManager::UpdateConnectStatus(
    int token, const std::string &deviceId, int status, const std::shared_ptr<RequestCallback> &requestCallback)
{
    HILOG_INFO("%{public}s called begin", __func__);
    if (continuationRegisterManagerProxy_ != nullptr) {
        continuationRegisterManagerProxy_->UpdateConnectStatus(token, deviceId, status, requestCallback);
        HILOG_INFO("%{public}s called end", __func__);
    } else {
        HILOG_ERROR("%{public}s, ContinuationRegisterManagerProxy is null", __func__);
    }
}

/**
 * notify controlcenter continuation register service to show device list.
 *
 * @param token token from register
 * @param parameter filter with supported device list.
 * @param requestCallback callback for this request, -1 means failed, otherwise successed.
 */
void ContinuationRegisterManager::ShowDeviceList(
    int token, const ExtraParams &parameter, const std::shared_ptr<RequestCallback> &requestCallback)
{
    HILOG_INFO("%{public}s called begin", __func__);
    if (continuationRegisterManagerProxy_ != nullptr) {
        continuationRegisterManagerProxy_->ShowDeviceList(token, parameter, requestCallback);
        HILOG_INFO("%{public}s called end", __func__);
    } else {
        HILOG_ERROR("%{public}s, ContinuationRegisterManagerProxy is null", __func__);
    }
}

/**
 * disconnect to controlcenter continuation register service.
 */
void ContinuationRegisterManager::Disconnect(void)
{
    HILOG_INFO("%{public}s called begin", __func__);
    if (continuationRegisterManagerProxy_ != nullptr) {
        continuationRegisterManagerProxy_->Disconnect();
        HILOG_INFO("%{public}s called end", __func__);
    } else {
        HILOG_ERROR("%{public}s, ContinuationRegisterManagerProxy is null", __func__);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
