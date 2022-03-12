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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_START_SPECIFIED_ABILITY_RESPONSE_STUB_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_START_SPECIFIED_ABILITY_RESPONSE_STUB_H

#include "iremote_stub.h"
#include "nocopyable.h"
#include "string_ex.h"
#include "istart_specified_ability_response.h"

namespace OHOS {
namespace AppExecFwk {
class StartSpecifiedAbilityResponseStub : public IRemoteStub<IStartSpecifiedAbilityResponse> {
public:
    StartSpecifiedAbilityResponseStub();
    virtual ~StartSpecifiedAbilityResponseStub();

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    // virtual void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag) {}
    // virtual void OnTimeoutResponse(const AAFwk::Want &want) {}

private:
    int32_t HandleOnAcceptWantResponse(MessageParcel &data, MessageParcel &reply);
    int32_t HandleOnTimeoutResponse(MessageParcel &data, MessageParcel &reply);

    using responseFunc = std::function<int32_t(MessageParcel&, MessageParcel&)>;
    std::map<uint32_t, responseFunc> responseFuncMap_;

    DISALLOW_COPY_AND_MOVE(StartSpecifiedAbilityResponseStub);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_START_SPECIFIED_ABILITY_RESPONSE_STUB_H