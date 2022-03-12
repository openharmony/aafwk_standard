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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_START_SPECIFIED_ABILITY_RESPONSE_PROXY_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_START_SPECIFIED_ABILITY_RESPONSE_PROXY_H

#include "iremote_proxy.h"
#include "istart_specified_ability_response.h"
namespace OHOS {
namespace AppExecFwk {
class StartSpecifiedAbilityResponseProxy : public IRemoteProxy<IStartSpecifiedAbilityResponse> {
public:
    explicit StartSpecifiedAbilityResponseProxy(const sptr<IRemoteObject> &impl);
    virtual ~StartSpecifiedAbilityResponseProxy() = default;

    virtual void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag) override;

    virtual void OnTimeoutResponse(const AAFwk::Want &want) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);
    static inline BrokerDelegator<StartSpecifiedAbilityResponseProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_START_SPECIFIED_ABILITY_RESPONSE_PROXY_H