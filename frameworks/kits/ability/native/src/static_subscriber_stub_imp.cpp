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

#include "static_subscriber_stub_imp.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
ErrCode StaticSubscriberStubImp::OnReceiveEvent(CommonEventData* data)
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto extension = extension_.lock();
    if (extension != nullptr) {
        std::shared_ptr<CommonEventData> commonEventData(data);
        extension->OnReceiveEvent(commonEventData);
        HILOG_INFO("%{public}s end successfully.", __func__);
        return 0;
    }
    HILOG_INFO("%{public}s end failed.", __func__);
    return -1;
}
} // namespace AppExecFwk
} // namespace OHOS
