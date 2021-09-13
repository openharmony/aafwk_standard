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

#include "data_ability_caller_recipient.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
void DataAbilityCallerRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    HILOG_ERROR("recv DataAbilityCallerRecipient death notice");

    if (handler_) {
        handler_(remote);
    }
}

DataAbilityCallerRecipient::DataAbilityCallerRecipient(RemoteDiedHandler handler) : handler_(handler)
{
    HILOG_ERROR("%{public}s", __func__);
}

DataAbilityCallerRecipient::~DataAbilityCallerRecipient()
{
    HILOG_ERROR("%{public}s", __func__);
}
}  // namespace AAFwk
}  // namespace OHOS