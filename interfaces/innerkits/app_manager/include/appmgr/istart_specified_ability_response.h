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

#ifndef FOUNDATION_ISTART_SPECIFIED_ABILITY_RESPONSE_H
#define FOUNDATION_ISTART_SPECIFIED_ABILITY_RESPONSE_H

#include "iremote_broker.h"
#include "iremote_object.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
class IStartSpecifiedAbilityResponse : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.startSpecifiedAbilityResponse");

    virtual void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag) = 0;

    virtual void OnTimeoutResponse(const AAFwk::Want &want) = 0;

    enum class Message {
        ON_ACCEPT_WANT_RESPONSE = 0,
        ON_TIMEOUT_RESPONSE,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_ISTART_SPECIFIED_ABILITY_RESPONSE_H