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

#ifndef OHOS_OS_AAFWK_DATAABILITY_CALLER_RECIPIENT_H
#define OHOS_OS_AAFWK_DATAABILITY_CALLER_RECIPIENT_H

#include <functional>
#include <iremote_object.h>
#include <iremote_stub.h>

namespace OHOS {
namespace AAFwk {
class DataAbilityCallerRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;

    explicit DataAbilityCallerRecipient(RemoteDiedHandler handler);

    virtual ~DataAbilityCallerRecipient();

    virtual void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    RemoteDiedHandler handler_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_OS_AAFWK_DATAABILITY_CALLER_RECIPIENT_H